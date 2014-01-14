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

#include <string>
#include "public_definitions.h"
#include "public_errors.h"
#include "public_rare_definitions.h"
#include "rapidjson/document.h"
#include "commands.h"
#include "globals.h"
#include "gw2_info.h"
#include "plugin.h"
#include "stringutils.h"
using namespace std;
using namespace Globals;

namespace Commands {

	bool getOwnClientID(uint64 serverConnectionHandlerID, anyID* myID) {
		if (!pluginID) {
			debuglog("GW2Plugin: Plugin not registered, unable to broadcast Guild Wars 2 info\n");
			return false;
		}

		if (ts3Functions.getClientID(serverConnectionHandlerID, myID) != ERROR_ok) {
			debuglog("GW2Plugin: Failed to get own ID\n");
			return false;
		}
		return true;
	}

	bool parseCommand(const string& command, CommandType& commandType, vector<string>& commandParameters) {
		commandType = CMD_NONE;
		vector<string> s = split(string(command), ' ', 2);
		vector<string> params;
		if (s.size() > 0) {
			if (s.at(0) == "GW2Info") {
				commandType = CMD_GW2INFO;
				if (s.size() > 1)
					params = split(s.at(1), ' ', 2);
			} else if (s.at(0) == "RequestGW2Info") {
				commandType = CMD_REQUESTGW2INFO;
				if (s.size() > 1)
					params.push_back(s.at(1));
			} else {
				return false;
			}
		}
		commandParameters = params;
		return true;
	}

	void send(uint64 serverConnectionHandlerID, CommandType type, const string& parameters, int targetMode, const anyID* targetIDs, const char* returnCode) {
		string command;

		switch (type) {
			case CMD_GW2INFO:
				command = "GW2Info";
				break;
			case CMD_REQUESTGW2INFO:
				command = "RequestGW2Info";
				break;
		}

		command += " " + parameters;
		ts3Functions.sendPluginCommand(serverConnectionHandlerID, pluginID, command.c_str(), targetMode, targetIDs, returnCode);
	}

	void requestGW2Info(uint64 serverConnectionHandlerID, int targetMode, const anyID* targetIDs) {
		anyID myID;
		if (!getOwnClientID(serverConnectionHandlerID, &myID))
			return;

		string parameters = to_string(myID);
		send(serverConnectionHandlerID, CMD_REQUESTGW2INFO, parameters, PluginCommandTarget_SERVER, targetIDs, NULL);
	}

	void sendGW2Info(uint64 serverConnectionHandlerID, const GW2Info& gw2Info, int targetMode, const anyID* targetIDs) {
		anyID myID;
		if (!getOwnClientID(serverConnectionHandlerID, &myID))
			return;

		string parameters = to_string(myID) + " " + gw2Info.jsonData.c_str();
		send(serverConnectionHandlerID, CMD_GW2INFO, parameters, PluginCommandTarget_SERVER, targetIDs, NULL);
	}

}
