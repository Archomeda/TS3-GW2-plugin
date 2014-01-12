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

#include <stdio.h>
#include <string>
#include <vector>
#include <Windows.h>
#include "public_definitions.h"

#if _DEBUG
#define debuglog(str, ...) printf(str, __VA_ARGS__);
#else
#define debuglog(str, ...)
#endif

struct GW2Info {
	bool isOnline;
	std::string identity;
	std::string characterName;
	int professionId;
	int mapId;
	int worldId;
	int teamColorId;
	bool commander;
};

struct GW2RemoteInfo {
	anyID clientID;
	uint64 serverConnectionHandlerID;
	GW2Info info;
};


class GW2RemoteInfoContainer {

private:
	std::vector<GW2RemoteInfo> gw2RemoteInfos;

protected:
	CRITICAL_SECTION cs;
	bool getRemoteGW2InfoRowID(uint64 serverConnectionHandlerID, anyID clientID, int* result);

public:
	GW2RemoteInfoContainer();
	~GW2RemoteInfoContainer();

	std::string getInfoData(uint64 serverConnectionHandlerID, anyID clientID, enum PluginItemType type);
	bool getInfoData(uint64 serverConnectionHandlerID, anyID clientID, enum PluginItemType type, std::string* data);

	bool getRemoteGW2Info(uint64 serverConnectionHandlerID, anyID clientID, GW2RemoteInfo* result);
	void updateRemoteGW2Info(GW2RemoteInfo& data);
	bool removeRemoteGW2InfoRecord(uint64 serverConnectionHandlerID, anyID clientID);
	void removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID);
	void removeAllRemoteGW2InfoRecords(uint64 serverConnectionHandlerID, int* removedRecords);

};
