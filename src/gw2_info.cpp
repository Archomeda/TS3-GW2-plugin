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

#include <vector>
#include "plugin_definitions.h"
#include "gw2_info.h"
#include "mumblelink.h"
#include "stringutils.h"
#include "ts3_functions.h"
using namespace std;


GW2RemoteInfoContainer::GW2RemoteInfoContainer(void) {
	InitializeCriticalSection(&cs);
}

GW2RemoteInfoContainer::~GW2RemoteInfoContainer(void) {
	DeleteCriticalSection(&cs);
}

string GW2RemoteInfoContainer::getInfoData(uint64 serverConnectionHandlerID, anyID clientID, enum PluginItemType type) {
	string data;
	getInfoData(serverConnectionHandlerID, clientID, type, &data);
	return data;
}

bool GW2RemoteInfoContainer::getInfoData(uint64 serverConnectionHandlerID, anyID clientID, enum PluginItemType type, string* data) {
	if (type == PLUGIN_CLIENT) {
		GW2RemoteInfo gw2RemoteInfo;
		if (getRemoteGW2Info(serverConnectionHandlerID, clientID, &gw2RemoteInfo)) {
			debuglog("GW2Plugin: Parsing data for client %d\n", clientID);
			if (!gw2RemoteInfo.info.isOnline) {
				*data = "Currently offline";
			} else {
				// TODO: Get map and world names from GW2 api
				*data =  "Playing as " + gw2RemoteInfo.info.characterName +
					" (" + MumbleLink::getProfessionName((MumbleLink::Profession)gw2RemoteInfo.info.professionId) + ")" +
					" in " + to_string(gw2RemoteInfo.info.mapId) +
					" (" + to_string(gw2RemoteInfo.info.worldId) + ")";
			}
			return true;
		} else {
			debuglog("GW2Plugin: No data found for client %d\n", clientID);
		}
	}
	return false;
}


bool GW2RemoteInfoContainer::getRemoteGW2InfoRowID(uint64 serverConnectionHandlerID, anyID clientID, int* result) {
	unsigned int i = 0;

	while (i < gw2RemoteInfos.size()) {
		GW2RemoteInfo currentRow = gw2RemoteInfos.at(i);
		if (currentRow.clientID == clientID && currentRow.serverConnectionHandlerID == serverConnectionHandlerID) {
			*result = i;
			return true;
		}
		i++;
	}

	return false;
}

bool GW2RemoteInfoContainer::getRemoteGW2Info(uint64 serverConnectionHandlerID, anyID clientID, GW2RemoteInfo* result) {
	int row;
	if (getRemoteGW2InfoRowID(serverConnectionHandlerID, clientID, &row)) {
		*result = gw2RemoteInfos.at(row);
		return true;
	}

	return false;
}

void GW2RemoteInfoContainer::updateRemoteGW2Info(GW2RemoteInfo& data) {
	int existingRecordId;

	EnterCriticalSection(&cs);
	if (getRemoteGW2InfoRowID(data.serverConnectionHandlerID, data.clientID, &existingRecordId)) {
		gw2RemoteInfos[existingRecordId] = data;
		debuglog("GW2Plugin: Updated existing remote GW2 client record in row %d\n", existingRecordId);
	} else {
		gw2RemoteInfos.push_back(data);
		debuglog("GW2Plugin: Added new remote GW2 client record for client %d\n", data.clientID);
	}
	LeaveCriticalSection(&cs);
}

bool GW2RemoteInfoContainer::removeRemoteGW2InfoRecord(uint64 serverConnectionHandlerID, anyID clientID) {
	int existingRecord;

	EnterCriticalSection(&cs);
	if (getRemoteGW2InfoRowID(serverConnectionHandlerID, clientID, &existingRecord)) {
		gw2RemoteInfos.erase(gw2RemoteInfos.begin() + existingRecord);
		debuglog("GW2Plugin: Removed remote GW2 client record for client %d\n", clientID);
		LeaveCriticalSection(&cs);
		return true;
	}
	LeaveCriticalSection(&cs);
	return false;
}

void GW2RemoteInfoContainer::removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID) {
	removeAllRemoteGW2InfoRecords(serverConnectionHandlerID, NULL);
}

void GW2RemoteInfoContainer::removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID, int* removedRecords) {
	unsigned int i = 0;
	int removed = 0;

	EnterCriticalSection(&cs);
	while (i < gw2RemoteInfos.size()) {
		if (gw2RemoteInfos[i].serverConnectionHandlerID == serverConnectionHandlerID) {
			gw2RemoteInfos.erase(gw2RemoteInfos.begin() + i);
			removed++;
		} else {
			i++;
		}
	}
	LeaveCriticalSection(&cs);

	debuglog("GW2Plugin: Removed %d remote GW2 client record(s)\n", removed);

	if (removedRecords != NULL)
		*removedRecords = removed;
}
