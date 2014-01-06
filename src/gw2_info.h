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

#ifndef GW2_INFO_H
#define GW2_INFO_H

#include <string>

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

#endif
