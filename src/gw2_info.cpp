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
#include "rapidjson/document.h"
#include "gw2_info.h"
#include "http.h"
#include "mumblelink.h"
#include "stringutils.h"
#include "ts3_functions.h"
using namespace std;

#define GW2APIURL_MAP "https://api.guildwars2.com/v1/maps.json?map_id=%d"
#define GW2APIURL_WORLDS "https://api.guildwars2.com/v1/world_names.json"


namespace GW2CacheData {

	std::map<int, MapData> mapData;
	std::map<int, std::string> worldData;

	bool getMapData(int mapID, MapData* data) {
		if (mapData.find(mapID) != mapData.end()) {
			*data = mapData.at(mapID);
			debuglog("GW2Plugin: Got cached data of map %d: %s - %s\n", mapID, data->regionName.c_str(), data->mapName.c_str());
			return true;
		}
	
		char urlBuffer[64];
		sprintf_s(urlBuffer, GW2APIURL_MAP, mapID);
		string result;
		try {
			if (getFromHttpUrl(urlBuffer, &result, NULL)) {
				debuglog("GW2Plugin: Downloaded data from %s\n", urlBuffer);
				rapidjson::Document json;
				json.Parse<0>(result.c_str());
				if (json.HasMember("maps")) {
					const rapidjson::Value& jsonMaps = json["maps"];
					if (jsonMaps.HasMember(to_string(mapID).c_str())) {
						const rapidjson::Value& jsonMap = jsonMaps[to_string(mapID).c_str()];
						if (jsonMap.HasMember("map_name")) data->mapName = jsonMap["map_name"].GetString();
						if (jsonMap.HasMember("region_id")) data->regionID = jsonMap["region_id"].GetUint();
						if (jsonMap.HasMember("region_name")) data->regionName = jsonMap["region_name"].GetString();
						if (!data->mapName.empty()) {
							mapData.insert(pair<int, MapData>(mapID, *data));
							debuglog("GW2Plugin: Downloaded data of map %d: %s - %s\n", mapID, data->regionName.c_str(), data->mapName.c_str());
							return true;
						}
					}
				}
			}
		} catch(int e) {
			debuglog("GW2Plugin: Error while getting map %d: %d", mapID, e);
		}

		debuglog("GW2Plugin: Map %d unknown\n", mapID);
		data->mapName = "Map " + to_string(mapID);
		data->regionName = "Region " + to_string(data->regionID);
		return false;
	}

	bool getWorldName(int worldID, string* worldName) {
		if (worldData.find(worldID) != worldData.end()) {
			*worldName = worldData.at(worldID);
			debuglog("GW2Plugin: Got cached name of world %d: %s\n", worldID, (*worldName).c_str());
			return true;
		}
	
		string result;
		bool found = false;
		try {
			if (getFromHttpUrl(GW2APIURL_WORLDS, &result, NULL)) {
				debuglog("GW2Plugin: Downloaded data from %s\n", GW2APIURL_WORLDS);
				rapidjson::Document json;
				json.Parse<0>(result.c_str());
				if (json.IsArray()) {
					for (rapidjson::SizeType i = 0; i < json.Size(); i++) {
						if (json[i].HasMember("id") && json[i].HasMember("name")) {
							int id = (int)atoi(json[i]["id"].GetString());
							string name = json[i]["name"].GetString();
							if (worldData.find(id) == worldData.end()) {
								worldData.insert(pair<int, string>(id, name));
								debuglog("GW2Plugin: Downloaded name of world %d: %s\n", id, name.c_str());
							}
							if (id == worldID) {
								found = true;
								*worldName = name;
							}
						}
					}
				}
			}
		} catch(int e) {
			debuglog("GW2Plugin: Error while getting world %d: %d", worldID, e);
		}

		if (!found) {
			debuglog("GW2Plugin: World %d unknown\n", worldID);
			*worldName = "World " + to_string(worldID);
		}
		return found;
	}

}


GW2RemoteInfoContainer::GW2RemoteInfoContainer() {
	InitializeCriticalSection(&cs);
}

GW2RemoteInfoContainer::~GW2RemoteInfoContainer() {
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
				*data = "Playing as " + gw2RemoteInfo.info.characterName +
					" (" + MumbleLink::getProfessionName((MumbleLink::Profession)gw2RemoteInfo.info.professionId) + ")\n" +
					gw2RemoteInfo.info.regionName + " - " + gw2RemoteInfo.info.mapName + " (" + gw2RemoteInfo.info.worldName + ")";
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
