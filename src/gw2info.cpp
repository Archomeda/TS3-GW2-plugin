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

#include "plugin_definitions.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "gw2info.h"
using namespace std;
using namespace Gw2Api;
using namespace Gw2Api::MumbleLink;

#if _DEBUG
#define debuglog(str, ...) printf(str, __VA_ARGS__);
#else
#define debuglog(str, ...)
#endif


Gw2Info::Gw2Info(string jsonString) {
	rapidjson::Document json;
	if (!json.Parse<0>(jsonString.c_str()).HasParseError()) {
		const rapidjson::Value& rj_character_name = json["character_name"];
		const rapidjson::Value& rj_profession = json["profession"];
		const rapidjson::Value& rj_map_id = json["map_id"];
		const rapidjson::Value& rj_map_name = json["map_name"];
		const rapidjson::Value& rj_region_id = json["region_id"];
		const rapidjson::Value& rj_region_name = json["region_name"];
		const rapidjson::Value& rj_continent_id = json["continent_id"];
		const rapidjson::Value& rj_continent_name = json["continent_name"];
		const rapidjson::Value& rj_world_id = json["world_id"];
		const rapidjson::Value& rj_world_name = json["world_name"];
		const rapidjson::Value& rj_waypoint_id = json["waypoint_id"];
		const rapidjson::Value& rj_waypoint_name = json["waypoint_name"];
		const rapidjson::Value& rj_waypoint_distance = json["waypoint_distance"];
		const rapidjson::Value& rj_team_color_id = json["team_color_id"];
		const rapidjson::Value& rj_commander = json["commander"];
		const rapidjson::Value& rj_avatar_position = json["avatar_position"];
		const rapidjson::Value& rj_plugin_version = json["plugin_version"];

		if (!rj_character_name.IsNull() && rj_character_name.IsString())		characterName = rj_character_name.GetString();
		if (!rj_profession.IsNull() && rj_profession.IsInt())					profession = (Profession)rj_profession.GetInt();
		if (!rj_map_id.IsNull() && rj_map_id.IsInt())							mapId = rj_map_id.GetInt();
		if (!rj_map_name.IsNull() && rj_map_name.IsString())					mapName = rj_map_name.GetString();
		if (!rj_region_id.IsNull() && rj_region_id.IsInt())						regionId = rj_region_id.GetInt();
		if (!rj_region_name.IsNull() && rj_region_name.IsString())				regionName = rj_region_name.GetString();
		if (!rj_continent_id.IsNull() && rj_continent_id.IsInt())				continentId = rj_continent_id.GetInt();
		if (!rj_continent_name.IsNull() && rj_continent_name.IsString())		continentName = rj_continent_name.GetString();
		if (!rj_world_id.IsNull() && rj_world_id.IsInt())						worldId = rj_world_id.GetInt();
		if (!rj_world_name.IsNull() && rj_world_name.IsString())				worldName = rj_world_name.GetString();
		if (!rj_waypoint_id.IsNull() && rj_waypoint_id.IsInt())					waypointId = rj_waypoint_id.GetInt();
		if (!rj_waypoint_name.IsNull() && rj_waypoint_name.IsString())			waypointName = rj_waypoint_name.GetString();
		if (!rj_waypoint_distance.IsNull() && rj_waypoint_distance.IsDouble())	waypointDistance = rj_waypoint_distance.GetDouble();
		if (!rj_team_color_id.IsNull() && rj_team_color_id.IsString())			teamColorId = rj_team_color_id.GetInt();
		if (!rj_commander.IsNull() && rj_commander.IsBool())					commander = rj_commander.GetBool();
		if (!rj_avatar_position.IsNull() && rj_avatar_position.IsArray()) {
			avatarPosition = Vector3D(
				rj_avatar_position[0u].GetDouble(),
				rj_avatar_position[1].GetDouble(),
				rj_avatar_position[2].GetDouble()
			);
		}
		if (!rj_plugin_version.IsNull() && rj_plugin_version.IsString())		pluginVersion = rj_plugin_version.GetString();
	}
}

string Gw2Info::toJson() const {
	rapidjson::Document json;
	json.Parse<0>("{}");
	json.AddMember("character_name", characterName.c_str(), json.GetAllocator());
	json.AddMember("profession", profession, json.GetAllocator());
	json.AddMember("map_id", mapId, json.GetAllocator());
	json.AddMember("map_name", mapName.c_str(), json.GetAllocator());
	json.AddMember("region_id", regionId, json.GetAllocator());
	json.AddMember("region_name", regionName.c_str(), json.GetAllocator());
	json.AddMember("continent_id", continentId, json.GetAllocator());
	json.AddMember("continent_name", continentName.c_str(), json.GetAllocator());
	json.AddMember("world_id", worldId, json.GetAllocator());
	json.AddMember("world_name", worldName.c_str(), json.GetAllocator());
	json.AddMember("waypoint_id", waypointId, json.GetAllocator());
	json.AddMember("waypoint_name", waypointName.c_str(), json.GetAllocator());
	json.AddMember("waypoint_distance", waypointDistance, json.GetAllocator());
	json.AddMember("team_color_id", teamColorId, json.GetAllocator());
	json.AddMember("commander", commander, json.GetAllocator());
	json.AddMember("avatar_position", "[]", json.GetAllocator());
	json["avatar_position"].SetArray();
	json["avatar_position"].PushBack(avatarPosition.x, json.GetAllocator());
	json["avatar_position"].PushBack(avatarPosition.y, json.GetAllocator());
	json["avatar_position"].PushBack(avatarPosition.z, json.GetAllocator());
	json.AddMember("plugin_version", pluginVersion.c_str(), json.GetAllocator());

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	json.Accept(writer);
	return buffer.GetString();
}


Gw2RemoteInfoContainer::Gw2RemoteInfoContainer() {
	InitializeCriticalSection(&cs);
}

Gw2RemoteInfoContainer::~Gw2RemoteInfoContainer() {
	DeleteCriticalSection(&cs);
}

string Gw2RemoteInfoContainer::getInfoData(uint64 serverConnectionHandlerID, anyID clientID, PluginItemType type) {
	string data;
	getInfoData(serverConnectionHandlerID, clientID, type, data);
	return data;
}

bool Gw2RemoteInfoContainer::getInfoData(uint64 serverConnectionHandlerID, anyID clientID, PluginItemType type, string& data) {
	if (type == PLUGIN_CLIENT) {
		Gw2RemoteInfo gw2RemoteInfo;
		if (getRemoteGW2Info(serverConnectionHandlerID, clientID, gw2RemoteInfo)) {
			debuglog("GW2Plugin: Parsing data for client %d\n", clientID);
			if (gw2RemoteInfo.characterName.empty()) {
				data = "Currently offline";
			} else {
				data = "Playing as [color=blue]" + gw2RemoteInfo.characterName + "[/color] (" + getProfessionName(gw2RemoteInfo.profession) + ")\n" +
					gw2RemoteInfo.regionName + " - [color=blue]" + gw2RemoteInfo.mapName + "[/color] (" + gw2RemoteInfo.worldName + ")";
				if (gw2RemoteInfo.waypointId > 0) {
					data += "\nNear [color=blue]" + gw2RemoteInfo.waypointName + "[/color]";
				} else {
					data += "\nNo near waypoint";
				}
			}
			return true;
		} else {
			debuglog("GW2Plugin: No data found for client %d\n", clientID);
		}
	}
	return false;
}


bool Gw2RemoteInfoContainer::getRemoteGW2InfoRowID(uint64 serverConnectionHandlerID, anyID clientID, int& result) {
	unsigned int i = 0;

	while (i < gw2RemoteInfos.size()) {
		Gw2RemoteInfo currentRow = gw2RemoteInfos.at(i);
		if (currentRow.clientID == clientID && currentRow.serverConnectionHandlerID == serverConnectionHandlerID) {
			result = i;
			return true;
		}
		i++;
	}

	return false;
}

bool Gw2RemoteInfoContainer::getRemoteGW2Info(uint64 serverConnectionHandlerID, anyID clientID, Gw2RemoteInfo& result) {
	int row;
	if (getRemoteGW2InfoRowID(serverConnectionHandlerID, clientID, row)) {
		result = gw2RemoteInfos.at(row);
		return true;
	}

	return false;
}

void Gw2RemoteInfoContainer::updateRemoteGW2Info(const Gw2RemoteInfo& data) {
	int existingRecordId;

	EnterCriticalSection(&cs);
	if (getRemoteGW2InfoRowID(data.serverConnectionHandlerID, data.clientID, existingRecordId)) {
		gw2RemoteInfos[existingRecordId] = data;
		debuglog("GW2Plugin: Updated existing remote GW2 client record in row %d\n", existingRecordId);
	} else {
		gw2RemoteInfos.push_back(data);
		debuglog("GW2Plugin: Added new remote GW2 client record for client %d\n", data.clientID);
	}
	LeaveCriticalSection(&cs);
}

bool Gw2RemoteInfoContainer::removeRemoteGW2InfoRecord(uint64 serverConnectionHandlerID, anyID clientID) {
	int existingRecord;

	EnterCriticalSection(&cs);
	if (getRemoteGW2InfoRowID(serverConnectionHandlerID, clientID, existingRecord)) {
		gw2RemoteInfos.erase(gw2RemoteInfos.begin() + existingRecord);
		debuglog("GW2Plugin: Removed remote GW2 client record for client %d\n", clientID);
		LeaveCriticalSection(&cs);
		return true;
	}
	LeaveCriticalSection(&cs);
	return false;
}

void Gw2RemoteInfoContainer::removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID) {
	removeAllRemoteGW2InfoRecords(serverConnectionHandlerID, NULL);
}

void Gw2RemoteInfoContainer::removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID, int* removedRecords) {
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
