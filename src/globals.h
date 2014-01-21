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
#include "ts3_functions.h"

#define PLUGIN_NAME "Guild Wars 2 Plugin"
#define PLUGIN_VERSION "0.1-a3"
#define PLUGIN_API_VERSION 19
#define PLUGIN_AUTHOR "Archomeda"
#define PLUGIN_DESCRIPTION "This plugin adds some Guild Wars 2 features to the TeamSpeak 3 client."

#define DEFAULTCONFIG_LOCATIONTRANSMISSIONTHRESHOLD 3
#define DEFAULTCONFIG_ONLINESTATETRANSMISSIONTHRESHOLD 15
#define DEFAULTCONFIG_DISTANCETRANSMISSIONTHRESHOLD 10


namespace Globals {
	extern TS3Functions ts3Functions;
	extern char* pluginID;

	extern int locationTransmissionThreshold;
	extern int onlineStateTransmissionThreshold;
	extern int distanceTransmissionThreshold;

	void loadConfig();

	std::string getConfigFilePath();
}
