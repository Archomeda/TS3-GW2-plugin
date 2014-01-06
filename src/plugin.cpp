/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include <Windows.h>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"
#include "rapidjson/document.h"
#include "plugin.h"
#include "gw2_info.h"
#include "mumblelink.h"
#include "stringutils.h"
using namespace std;

static struct TS3Functions ts3Functions;
GW2Info gw2Info;
vector<GW2RemoteInfo> gw2RemoteInfos;

static PluginItemType infoDataType = (PluginItemType)0;
static uint64 infoDataId = 0;

static CRITICAL_SECTION cs;
int threadStopRequested = 0;
HANDLE hThread = 0;

#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#if _DEBUG
#define debuglog(str, ...) printf(str, __VA_ARGS__);
#else
#define debuglog(str, ...)
#endif

#define PLUGIN_API_VERSION 19

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 1024
#define INFODATA_BUFSIZE 1024
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

DWORD WINAPI mumbleLinkCheckLoop (LPVOID lpParam);


/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
	return "Guild Wars 2 Plugin";
}

/* Plugin version */
const char* ts3plugin_version() {
	return "0.1";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	return "Archomeda";
}

/* Plugin description */
const char* ts3plugin_description() {
	return "This plugin adds some Guild Wars 2 features to the TeamSpeak 3 client.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {
	/* Your plugin init code here */
	debuglog("GW2Plugin: init\n");

	InitializeCriticalSection(&cs);
	
	threadStopRequested = 0;
	hThread = CreateThread(NULL, 0, mumbleLinkCheckLoop, NULL, 0, NULL);
	if (hThread == 0) {
		debuglog("\tCould not create thread to check for Guild Wars 2 updates through Mumble Link: %d\n", GetLastError());
		return 1;
	}

	/* In case the plugin was activated after a connection with the server has been made */
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (serverConnectionHandlerID != 0) {
		debuglog("GW2Plugin: Already online, sending and requesting GW2 info\n");
		sendGW2Info(serverConnectionHandlerID, PluginCommandTarget_SERVER, NULL);
		requestGW2Info(serverConnectionHandlerID, PluginCommandTarget_SERVER, NULL);
	}

	return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
	/* Your plugin cleanup code here */
	debuglog("GW2Plugin: shutdown\n");

	if (hThread != 0) {
		bool threadClosed = false;
		threadStopRequested = 1;
		DWORD threadReturn = WaitForSingleObject(hThread, 1000);
		switch (threadReturn) {
			case WAIT_ABANDONED:
				debuglog("\tGuild Wars 2 checker thread has been abandoned\n");
				break;
			case WAIT_OBJECT_0:
				debuglog("\tGuild Wars 2 checker thread has exited\n");
				CloseHandle(hThread);
				threadClosed = true;
				break;
			case WAIT_TIMEOUT:
				debuglog("\tGuild Wars 2 checker thread has timed out\n");
				break;
			case WAIT_FAILED:
				debuglog("\tWaiting on Guild Wars 2 checker thread has failed: %d\n", GetLastError());
				break;
		}

		/* In case the thread has not been exited within the given timeout, terminate it to prevent hanging */
		if (!threadClosed) {
			TerminateThread(hThread, 0);
			CloseHandle(hThread);
		}
	}

	/* In case the plugin was deactivated without shutting down TeamSpeak */
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (serverConnectionHandlerID != 0) {
		debuglog("GW2Plugin: Sending offline GW2 info message\n");
		gw2Info.identity = "{}";
		sendGW2Info(serverConnectionHandlerID, PluginCommandTarget_SERVER, NULL);
	}
	DeleteCriticalSection(&cs);

	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	debuglog("GW2Plugin: offersConfigure\n");

	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
	return PLUGIN_OFFERS_NO_CONFIGURE;  /* In this case ts3plugin_configure does not need to be implemented */
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	debuglog("GW2Plugin: registerPluginID: %s\n", pluginID);
}


/* Client changed current server connection handler */
void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID) {
	debuglog("GW2Plugin: currentServerConnectionChanged %llu (%llu)\n", (long long unsigned int)serverConnectionHandlerID, (long long unsigned int)ts3Functions.getCurrentServerConnectionHandlerID());
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "Guild Wars 2";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	/* Save this in case the right panel needs to be updated visually without having to reselect the client */
	infoDataType = type;
	infoDataId = id;

	if (type == PLUGIN_CLIENT) {
		int clientDataRow = -1;
		debuglog("GW2Plugin: Client %d selected\n", id);
		try {
			if (getRowForClientGW2Data(serverConnectionHandlerID, (anyID)id, &clientDataRow) == 1) {
				debuglog("\tFound record in gw2RemoteInfos[%d]\n", clientDataRow);
				*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  /* Must be allocated in the plugin! */
				if (!gw2RemoteInfos[clientDataRow].info.isOnline) {
					snprintf(*data, INFODATA_BUFSIZE, "Currently offline");
				} else {
					// TODO: Get map and world names from GW2 api
					snprintf(*data, INFODATA_BUFSIZE, "Playing as %s (%s) in %d (%d)",
						gw2RemoteInfos[clientDataRow].info.characterName.c_str(),
						MumbleLink::getProfessionName((MumbleLink::Profession)gw2RemoteInfos[clientDataRow].info.professionId).c_str(),
						gw2RemoteInfos[clientDataRow].info.mapId,
						gw2RemoteInfos[clientDataRow].info.worldId);
				}
			} else {
				data = NULL;
			}
		} catch (int e) {
			debuglog("GW2Plugin: Exception caught while trying to display info data: ", e);
		}
	} else {
		data = NULL;
	}
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
	return 1;  /* 1 = request autoloaded, 0 = do not request autoload */
}


/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	/* Some example code following to show how to use the information query functions. */

	switch (newStatus) {
		case STATUS_DISCONNECTED: {
			int removedRecords = 0;
			debuglog("GW2Plugin: Disconnected; removing all previous received client data\n");
			
			EnterCriticalSection(&cs);
			removeAllClientGW2DataRecords(serverConnectionHandlerID, &removedRecords);
			LeaveCriticalSection(&cs);
			
			if (removedRecords) { debuglog("GW2Plugin: Removed %d record(s)\n", removedRecords); }
			break;
		}
		case STATUS_CONNECTION_ESTABLISHED:
			debuglog("GW2Plugin: Connection with server %d established\n", serverConnectionHandlerID);
			
			sendGW2Info(serverConnectionHandlerID, PluginCommandTarget_SERVER, NULL);
			requestGW2Info(serverConnectionHandlerID, PluginCommandTarget_SERVER, NULL);
			break;
	}
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	debuglog("GW2Plugin: Client %d has been kicked from server, removing received data\n", clientID);

	EnterCriticalSection(&cs);
	removeClientGW2DataRecord(serverConnectionHandlerID, clientID);
	LeaveCriticalSection(&cs);
}

int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage) {
	debuglog("GW2Plugin: onServerErrorEvent %llu %s %d %s\n", (long long unsigned int)serverConnectionHandlerID, errorMessage, error, (returnCode ? returnCode : ""));
	if (returnCode) {
		/* A plugin could now check the returnCode with previously (when calling a function) remembered returnCodes and react accordingly */
		/* In case of using a a plugin return code, the plugin can return:
		 * 0: Client will continue handling this error (print to chat tab)
		 * 1: Client will ignore this error, the plugin announces it has handled it */
		return 0;
	}
	return 0;  /* If no plugin return code was used, the return value of this function is ignored */
}

void ts3plugin_onServerStopEvent(uint64 serverConnectionHandlerID, const char* shutdownMessage) {
	int removedRecords = 0;
	debuglog("GW2Plugin: Server stopped; removing all previous received client data\n");
	removeAllClientGW2DataRecords(serverConnectionHandlerID, &removedRecords);
	if (removedRecords) { debuglog("GW2Plugin: Removed %d record(s)\n", removedRecords); }
}

/* Clientlib rare */

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand) {
	enum { CMD_NONE = 0, CMD_GW2INFO, CMD_REQUESTGW2INFO } cmd = CMD_NONE;

	debuglog("GW2Plugin: Received command '%s'\n", pluginCommand);
	vector<string> s = split(string(pluginCommand), ' ', 2);
	vector<string> params;
	debuglog("\tCommand: %s\n\tParameters: %s\n", s.at(0).c_str(), s.at(1).c_str());
	if (s.size() > 0) {
		if (s.at(0) == "GW2Info") {
			cmd = CMD_GW2INFO;
			if (s.size() > 1)
				params = split(s.at(1), ' ', 2);
		} else if (s.at(0) == "RequestGW2Info") {
			cmd = CMD_REQUESTGW2INFO;
			if (s.size() > 1)
				params.push_back(s.at(1));
		}
	}

	switch(cmd) {
		case CMD_NONE:
			debuglog("\tUnknown command\n");
			break;  /* Command not handled by plugin */
		case CMD_GW2INFO: {
			if (params.size() != 2) {
				debuglog("\tInvalid parameter count: %d\n", params.size());
				break;
			}
			debuglog("\tCommand: GW2Info\n\tClient: %s\n\tData: %s\n", params.at(0).c_str(), params.at(1).c_str());

			anyID clientID = (int)atoi(params.at(0).c_str());
			int existingRecord = -1;

			EnterCriticalSection(&cs);
			if (getRowForClientGW2Data(serverConnectionHandlerID, clientID, &existingRecord) == 1) {
				rapidjson::Document identityDocument;
				identityDocument.Parse<0>(params.at(1).c_str());
				gw2RemoteInfos[existingRecord].info.identity = params.at(1).c_str();
				if (!identityDocument.HasMember("name")) {
					gw2RemoteInfos[existingRecord].info.isOnline = false;
				} else {
					gw2RemoteInfos[existingRecord].info.isOnline = true;
					gw2RemoteInfos[existingRecord].info.characterName = identityDocument["name"].GetString();
					gw2RemoteInfos[existingRecord].info.professionId = identityDocument["profession"].GetInt();
					gw2RemoteInfos[existingRecord].info.mapId = identityDocument["map_id"].GetInt();
					gw2RemoteInfos[existingRecord].info.worldId = identityDocument["world_id"].GetInt();
					gw2RemoteInfos[existingRecord].info.teamColorId = identityDocument["team_color_id"].GetInt();
					gw2RemoteInfos[existingRecord].info.commander = identityDocument["commander"].GetBool();
				}
				debuglog("\tUpdated existing record in gw2RemoteInfos[%d]\n", existingRecord);
			} else {
				rapidjson::Document identityDocument;
				identityDocument.Parse<0>(params.at(1).c_str());
				GW2RemoteInfo gw2RemoteInfo;
				gw2RemoteInfo.clientID = clientID;
				gw2RemoteInfo.serverConnectionHandlerID = serverConnectionHandlerID;
				gw2RemoteInfo.info.identity = params.at(1).c_str();
				if (!identityDocument.HasMember("name")) {
					gw2RemoteInfo.info.isOnline = false;
				} else {
					gw2RemoteInfo.info.isOnline = true;
					gw2RemoteInfo.info.characterName = identityDocument["name"].GetString();
					gw2RemoteInfo.info.professionId = identityDocument["profession"].GetInt();
					gw2RemoteInfo.info.mapId = identityDocument["map_id"].GetInt();
					gw2RemoteInfo.info.worldId = identityDocument["world_id"].GetInt();
					gw2RemoteInfo.info.teamColorId = identityDocument["team_color_id"].GetInt();
					gw2RemoteInfo.info.commander = identityDocument["commander"].GetBool();
				}
				gw2RemoteInfos.push_back(gw2RemoteInfo);
				debuglog("\tAdded new record to gw2RemoteInfos\n");
			}
			LeaveCriticalSection(&cs);

			// Update right panel if active
			if (infoDataType > 0 && infoDataId > 0) ts3Functions.requestInfoUpdate(serverConnectionHandlerID, infoDataType, infoDataId);
			break;
		}
		case CMD_REQUESTGW2INFO: {
			if (params.size() != 1) {
				debuglog("\tInvalid parameter count: %d\n", params.size());
				break;
			}
			debuglog("\tCommand: RequestGW2Info\n\tClient: %s\n", params.at(0).c_str());

			anyID clientID = (int)atoi(params.at(0).c_str());
			sendGW2Info(serverConnectionHandlerID, PluginCommandTarget_CLIENT, &clientID);
			break;
		}
	}
}


int getRowForClientGW2Data(uint64 serverConnectionHandlerID, anyID clientID, int* result) {
	unsigned int i = 0;
	while (i < gw2RemoteInfos.size()) {
		GW2RemoteInfo currentRow = gw2RemoteInfos.at(i);
		if (currentRow.clientID == clientID && currentRow.serverConnectionHandlerID == serverConnectionHandlerID) {
			debuglog("GW2Plugin: Found existing character data in row %d\n", i);
			debuglog("\tclientID: %d\tserverHandlerID: %d\n", clientID, serverConnectionHandlerID);
			*result = i;
			return 1;
		}
		i++;
	}

	return 0;
}

int removeClientGW2DataRecord(uint64 serverConnectionHandlerID, anyID clientID) {
	int existingRecord = -1;
	if (getRowForClientGW2Data(serverConnectionHandlerID, clientID, &existingRecord) == 1) {
		gw2RemoteInfos.erase(gw2RemoteInfos.begin() + existingRecord);
		return 1;
	}
	return 0;
}

void removeAllClientGW2DataRecords(uint64 serverConnectionHandlerID, int* removedRecords) {
	unsigned int i = 0;
	while (i < gw2RemoteInfos.size()) {
		if (gw2RemoteInfos[i].serverConnectionHandlerID == serverConnectionHandlerID
			&& removeClientGW2DataRecord(serverConnectionHandlerID, gw2RemoteInfos[i].clientID) == 1) {
				*removedRecords++;
		} else {
			i++;
		}
	}
}

void requestGW2Info(uint64 serverConnectionHandlerID, int targetMode, anyID* targetIDs) {
	anyID myID;
	if (!pluginID) {
		debuglog("GW2Plugin: Plugin not registered, unable to broadcast Guild Wars 2 info\n");
		return;
	}
	if (ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
		debuglog("GW2Plugin: Failed to get own ID\n");
		return;
	}

	char command[COMMAND_BUFSIZE];
	snprintf(command, sizeof(command), "RequestGW2Info %d", myID);
	ts3Functions.sendPluginCommand(serverConnectionHandlerID, pluginID, command, PluginCommandTarget_SERVER, targetIDs, NULL);
}

void sendGW2Info(uint64 serverConnectionHandlerID, int targetMode, anyID* targetIDs) {
	anyID myID;
	if (!pluginID) {
		debuglog("GW2Plugin: Plugin not registered, unable to broadcast Guild Wars 2 info\n");
		return;
	}
	if (ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
		debuglog("GW2Plugin: Failed to get own ID\n");
		return;
	}

	char command[COMMAND_BUFSIZE];
	snprintf(command, sizeof(command), "GW2Info %d %s", myID, gw2Info.identity.c_str());
	ts3Functions.sendPluginCommand(serverConnectionHandlerID, pluginID, command, PluginCommandTarget_SERVER, targetIDs, NULL);
}


DWORD WINAPI mumbleLinkCheckLoop (LPVOID lpParam) {
	MumbleLink::initLink();
	debuglog("GW2Plugin: Mumble Link created\n");

	while (!threadStopRequested) {
		bool updated = false;

		bool newIsOnline;
		string newIdentity;

		if (MumbleLink::getStatus() == 1 && MumbleLink::isGW2()) {
			newIsOnline = true;
			newIdentity = MumbleLink::getIdentity();
		} else {
			newIsOnline = false;
			newIdentity = "";
		}

		if (gw2Info.isOnline != newIsOnline) {
			if (newIsOnline) {
				debuglog("GW2Plugin: Guild Wars 2 linked\n");
			} else {
				debuglog("GW2Plugin: Guild Wars 2 unlinked\n");
				gw2Info.identity = "{}";
			}
			gw2Info.isOnline = newIsOnline;
			updated = true;
		}

		if (gw2Info.isOnline) {
			if (gw2Info.identity != newIdentity) {
				debuglog("GW2Plugin: New Guild Wars 2 identity: %s\n", newIdentity.c_str());
				gw2Info.identity = newIdentity;
				updated = true;
			}
		}

		if (updated)
			sendGW2Info(ts3Functions.getCurrentServerConnectionHandlerID(), PluginCommandTarget_SERVER, NULL);

		Sleep(200);
	}
	return 0;
}