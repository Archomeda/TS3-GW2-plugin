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

#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include "public_definitions.h"
#include "gw2api/math.h"
#include "gw2api/mumblelink.h"
#include "globals.h"

struct Gw2Info {
	std::string characterName;
	Gw2Api::MumbleLink::Profession profession;
	int mapId;
	std::string mapName;
	int regionId;
	std::string regionName;
	int continentId;
	std::string continentName;
	int worldId;
	std::string worldName;
	Gw2Api::Vector3D characterContinentPosition;
	int waypointId;
	std::string waypointName;
	Gw2Api::Vector2D waypointContinentPosition;
	int teamColorId;
	bool commander;
	std::string pluginVersion;

	Gw2Info() { clear(); }
	Gw2Info(std::string jsonString);

	std::string toJson() const;
	void clear() {
		characterName = "";
		profession = (Gw2Api::MumbleLink::Profession)0;
		mapId = 0;
		mapName = "";
		regionId = 0;
		regionName = "";
		continentId = 0;
		continentName = "";
		worldId = 0;
		worldName = "";
		characterContinentPosition = Gw2Api::Vector3D();
		waypointId = 0;
		waypointName = "";
		waypointContinentPosition = Gw2Api::Vector2D();
		teamColorId = 0;
		commander = false;
		pluginVersion = PLUGIN_VERSION;
	}
};

struct Gw2RemoteInfo : Gw2Info {
	uint64 serverConnectionHandlerID;
	anyID clientID;

	Gw2RemoteInfo() : Gw2Info() { }
	Gw2RemoteInfo(std::string jsonString, uint64 serverConnectionHandlerID, anyID clientID) : Gw2Info(jsonString) {
		this->serverConnectionHandlerID = serverConnectionHandlerID;
		this->clientID = clientID;
	}
};


class Gw2RemoteInfoContainer {

private:
	std::vector<Gw2RemoteInfo> gw2RemoteInfos;

protected:
	CRITICAL_SECTION cs;
	bool getRemoteGW2InfoRowID(uint64 serverConnectionHandlerID, anyID clientID, int& result);

public:
	Gw2RemoteInfoContainer();
	~Gw2RemoteInfoContainer();

	std::string getInfoData(uint64 serverConnectionHandlerID, anyID clientID, enum PluginItemType type);
	bool getInfoData(uint64 serverConnectionHandlerID, anyID clientID, enum PluginItemType type, std::string& data);

	bool getRemoteGW2Info(uint64 serverConnectionHandlerID, anyID clientID, Gw2RemoteInfo& result);
	void updateRemoteGW2Info(const Gw2RemoteInfo& data);
	bool removeRemoteGW2InfoRecord(uint64 serverConnectionHandlerID, anyID clientID);
	void removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID);
	void removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID, int* removedRecords);

};
