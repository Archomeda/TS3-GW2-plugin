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

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "commands.h"
#include "plugin.h"
#include "globals.h"
#include "gw2_info.h"
#include "mumblelink.h"
#include "stringutils.h"
#include "updatechecker.h"
using namespace std;
using namespace Globals;

GW2Info gw2Info;
GW2RemoteInfoContainer gw2RemoteInfoContainer;

static PluginItemType infoDataType = (PluginItemType)0;
static uint64 infoDataId = 0;

time_t lastUpdateCheck = 0;
bool threadStopRequested = false;
HANDLE hThread = 0;

DWORD WINAPI checkForUpdatesAsync(LPVOID lpParam);
DWORD WINAPI mumbleLinkCheckLoop(LPVOID lpParam);


/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
	return PLUGIN_NAME;
}

/* Plugin version */
const char* ts3plugin_version() {
	return PLUGIN_VERSION;
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	return PLUGIN_AUTHOR;
}

/* Plugin description */
const char* ts3plugin_description() {
	return PLUGIN_DESCRIPTION;
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

	threadStopRequested = false;
	hThread = CreateThread(NULL, 0, mumbleLinkCheckLoop, NULL, 0, NULL);
	if (hThread == 0) {
		debuglog("\tCould not create thread to check for Guild Wars 2 updates through Mumble Link: %d\n", GetLastError());
		return 1;
	}

	/* In case the plugin was activated after a connection with the server has been made */
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (serverConnectionHandlerID != 0) {
		debuglog("GW2Plugin: Already online, checking for updates\n");
		checkForUpdates();
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
		threadStopRequested = true;
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

	/* In case the plugin was deactivated without shutting down TeamSpeak, we need to let the other clients know */
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (serverConnectionHandlerID != 0) {
		debuglog("GW2Plugin: Sending offline GW2 info message\n");
		gw2Info.jsonData = "";
		Commands::sendGW2Info(serverConnectionHandlerID, gw2Info, PluginCommandTarget_SERVER, NULL);
	}

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
	strcpy_s(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
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
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 clientID, enum PluginItemType type, char** data) {
	/* Save this in case the right panel needs to be updated visually without having to reselect the client */
	/* This also has a purpose of getting the last selected row and check if it's the same as the current one */
	bool isNew = infoDataId != clientID;
	infoDataType = type;
	infoDataId = clientID;

	try {
		string result = gw2RemoteInfoContainer.getInfoData(serverConnectionHandlerID, (anyID)clientID, type);
		if (result.length() > 0) {
			*data = _strdup(result.c_str());
		} else {
			*data = NULL;
		}
	} catch (int e) {
		debuglog("GW2Plugin: Exception caught while trying to display info data: %d", e);
		*data = NULL;
	}

	if (isNew && type == PLUGIN_CLIENT)
		Commands::requestGW2Info(serverConnectionHandlerID, PluginCommandTarget_CLIENT, (anyID*)&clientID);
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
	switch (newStatus) {
		case STATUS_DISCONNECTED: {
			debuglog("GW2Plugin: Disconnected; removing all previous received client data\n");			
			gw2RemoteInfoContainer.removeAllRemoteGW2InfoRecords(serverConnectionHandlerID);
			break;
		}
		case STATUS_CONNECTION_ESTABLISHED:
			debuglog("GW2Plugin: Connection with server %d established\n", serverConnectionHandlerID);
			checkForUpdates();
			break;
	}
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
	debuglog("GW2Plugin: Client %d has been kicked from server, removing received data\n", clientID);
	gw2RemoteInfoContainer.removeRemoteGW2InfoRecord(serverConnectionHandlerID, clientID);
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
	debuglog("GW2Plugin: Server stopped; removing all previous received client data\n");
	gw2RemoteInfoContainer.removeAllRemoteGW2InfoRecords(serverConnectionHandlerID);
}

/* Clientlib rare */

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand) {
	debuglog("GW2Plugin: Received command '%s'\n", pluginCommand);

	Commands::CommandType commandType;
	vector<string> commandParameters;
	Commands::parseCommand(pluginCommand, commandType, commandParameters);

	switch(commandType) {
		case Commands::CMD_NONE:
			debuglog("\tUnknown command\n");
			break;  /* Command not handled by plugin */
		case Commands::CMD_GW2INFO: {
			GW2RemoteInfo gw2RemoteInfo;
			gw2RemoteInfo.serverConnectionHandlerID = serverConnectionHandlerID;
			gw2RemoteInfo.clientID = (int)atoi(commandParameters.at(0).c_str());
			if (commandParameters.size() == 1) {
				debuglog("\tCommand: GW2Info\n\tClient: %s\n\tData:\n", commandParameters.at(0).c_str());
				gw2RemoteInfo.info.isOnline = false;
			} else {
				debuglog("\tCommand: GW2Info\n\tClient: %s\n\tData: %s\n", commandParameters.at(0).c_str(), commandParameters.at(1).c_str());
				rapidjson::Document json;
				json.Parse<0>(commandParameters.at(1).c_str());
				gw2RemoteInfo.info.isOnline = true;
				if (json.HasMember("name")) gw2RemoteInfo.info.characterName = json["name"].GetString();
				if (json.HasMember("profession")) gw2RemoteInfo.info.professionId = json["profession"].GetInt();
				if (json.HasMember("map_id")) gw2RemoteInfo.info.mapId = json["map_id"].GetUint();
				if (json.HasMember("map_name")) gw2RemoteInfo.info.mapName = json["map_name"].GetString();
				if (json.HasMember("region_id")) gw2RemoteInfo.info.regionId = json["region_id"].GetUint();
				if (json.HasMember("region_name")) gw2RemoteInfo.info.regionName = json["region_name"].GetString();
				if (json.HasMember("world_id")) gw2RemoteInfo.info.worldId = json["world_id"].GetUint();
				if (json.HasMember("world_name")) gw2RemoteInfo.info.worldName = json["world_name"].GetString();
				if (json.HasMember("team_color_id")) gw2RemoteInfo.info.teamColorId = json["team_color_id"].GetInt();
				if (json.HasMember("commander")) gw2RemoteInfo.info.commander = json["commander"].GetBool();
			}
			gw2RemoteInfoContainer.updateRemoteGW2Info(gw2RemoteInfo);
			updateInfoPanel();
			break;
		}
		case Commands::CMD_REQUESTGW2INFO: {
			if (commandParameters.size() != 1) {
				debuglog("\tInvalid parameter count: %d\n", commandParameters.size());
				break;
			}
			debuglog("\tCommand: RequestGW2Info\n\tClient: %s\n", commandParameters.at(0).c_str());

			anyID clientID = (int)atoi(commandParameters.at(0).c_str());
			Commands::sendGW2Info(serverConnectionHandlerID, gw2Info, PluginCommandTarget_CLIENT, &clientID);
			break;
		}
	}
}

void updateInfoPanel() {
	if (infoDataType > 0 && infoDataId > 0) ts3Functions.requestInfoUpdate(ts3Functions.getCurrentServerConnectionHandlerID(), infoDataType, infoDataId);
}

bool checkForUpdates() {
#ifndef _DEBUG
	HANDLE hThread = CreateThread(NULL, 0, checkForUpdatesAsync, NULL, 0, NULL);
	return hThread != 0;
#else
	return false;
#endif
}

DWORD WINAPI checkForUpdatesAsync(LPVOID lpParam) {
	try {
		// Delay update checks for an hour since last check
		if (difftime(time(NULL), lastUpdateCheck) >= 3600) {
			Version newVersion;
			string url;
			if (checkForUpdate(true, newVersion, url)) {
				string updateMessage = "[color=blue]Guild Wars 2 plugin version " + newVersion.getVersionString() + " is now available.[/color] " + 
					"[url=" + url + "]Click here to download.[/url]";
				ts3Functions.printMessageToCurrentTab(updateMessage.c_str());
			}
			lastUpdateCheck = time(NULL);
		}
	} catch (...) {
		ts3Functions.printMessageToCurrentTab("Error while checking for Guild Wars 2 plugin updates.");
	}
	return 0;
}


DWORD WINAPI mumbleLinkCheckLoop(LPVOID lpParam) {
	MumbleLink::initLink();
	debuglog("GW2Plugin: Mumble Link created\n");

	time_t lastMumbleLinkUpdate = 0;
	time_t lastOffline = 0;
	while (!threadStopRequested) {
		// Delay too frequent changes with 5 seconds since last change, otherwise we might spam the server with commands
		if (difftime(time(NULL), lastMumbleLinkUpdate) < 5) {
			Sleep(50);
			continue;
		}

		bool updated = false;

		bool newIsOnline;
		string newIdentity;

		// Check if GW2 is active and wait for 20 seconds otherwise before considering that GW2 is offline (after recently being online)
		if (MumbleLink::isActive() == 1 && MumbleLink::isGW2()) {
			newIsOnline = true;
			newIdentity = MumbleLink::getIdentity();
			lastOffline = 0;
		} else if (difftime(time(NULL), lastOffline) < 20) {
			Sleep(50);
			continue;
		} else if (lastOffline == 0) {
			lastOffline = time(NULL);
			Sleep(50);
			continue;
		} else {
			newIsOnline = false;
			newIdentity = "";
		}

		if (gw2Info.isOnline != newIsOnline) {
			if (newIsOnline) {
				debuglog("GW2Plugin: Guild Wars 2 linked\n");
			} else {
				debuglog("GW2Plugin: Guild Wars 2 unlinked\n");
				gw2Info.jsonData = "";
				gw2Info.identity = "";
			}
			gw2Info.isOnline = newIsOnline;
			updated = true;
		}

		if (gw2Info.isOnline) {
			if (gw2Info.identity != newIdentity) {
				gw2Info.identity = newIdentity;
				gw2Info.isOnline = true;
				rapidjson::Document json;
				json.Parse<0>(newIdentity.c_str());
				if (json.HasMember("name")) gw2Info.characterName = json["name"].GetString();
				if (json.HasMember("profession")) gw2Info.professionId = json["profession"].GetInt();
				if (json.HasMember("map_id")) gw2Info.mapId = json["map_id"].GetUint();
				if (json.HasMember("world_id")) gw2Info.worldId = json["world_id"].GetUint();
				if (json.HasMember("team_color_id")) gw2Info.teamColorId = json["team_color_id"].GetInt();
				if (json.HasMember("commander")) gw2Info.commander = json["commander"].GetBool();
				GW2CacheData::MapData mapData;
				GW2CacheData::getMapData(gw2Info.mapId, mapData);
				gw2Info.mapName = mapData.mapName;
				gw2Info.regionId = mapData.regionID;
				gw2Info.regionName = mapData.regionName;
				GW2CacheData::getWorldName(gw2Info.worldId, gw2Info.worldName);
				json.AddMember("map_name", gw2Info.mapName.c_str(), json.GetAllocator());
				json.AddMember("world_name", gw2Info.worldName.c_str(), json.GetAllocator());
				json.AddMember("region_id", gw2Info.regionId, json.GetAllocator());
				json.AddMember("region_name", gw2Info.regionName.c_str(), json.GetAllocator());

				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				json.Accept(writer);
				gw2Info.jsonData = buffer.GetString();

				debuglog("GW2Plugin: Got new Guild Wars 2 data: %s\n", gw2Info.jsonData.c_str());
				updated = true;
			}
		}

		if (updated) {
			lastMumbleLinkUpdate = time(NULL);
			Commands::sendGW2Info(ts3Functions.getCurrentServerConnectionHandlerID(), gw2Info, PluginCommandTarget_SERVER, NULL);
		}

		Sleep(50);
	}
	return 0;
}
