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
#include "public_definitions.h"
#include "gw2_info.h"

#if _DEBUG
#define debuglog(str, ...) printf(str, __VA_ARGS__);
#else
#define debuglog(str, ...)
#endif

namespace Commands {

	enum CommandType { 
		CMD_NONE = 0,
		CMD_GW2INFO, 
		CMD_REQUESTGW2INFO
	};

	bool parseCommand(std::string command, CommandType* commandType, std::vector<std::string>* commandParameters);

	void send(uint64 serverConnectionHandlerID, CommandType type, std::string parameters, PluginTargetMode targetMode, const anyID* targetIDs, const char* returnCode);
	void requestGW2Info(uint64 serverConnectionHandlerID, int targetMode, const anyID* targetIDs);
	void sendGW2Info(uint64 serverConnectionHandlerID, GW2Info gw2Info, int targetMode, const anyID* targetIDs);

}
