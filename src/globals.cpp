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

#include "globals.h"
#include <QtCore/QSettings>

namespace Globals {
	TS3Functions ts3Functions;
	char* pluginID;

	int locationTransmissionThreshold = DEFAULTCONFIG_LOCATIONTRANSMISSIONTHRESHOLD;
	int onlineStateTransmissionThreshold = DEFAULTCONFIG_ONLINESTATETRANSMISSIONTHRESHOLD;
	int distanceTransmissionThreshold = DEFAULTCONFIG_DISTANCETRANSMISSIONTHRESHOLD;

	void loadConfig() {
		QSettings cfg(QString::fromStdString(getConfigFilePath()), QSettings::IniFormat);
		locationTransmissionThreshold = cfg.value("locationTransmissionThreshold", DEFAULTCONFIG_LOCATIONTRANSMISSIONTHRESHOLD).toInt();
		onlineStateTransmissionThreshold = cfg.value("onlineStateTransmissionThreshold", DEFAULTCONFIG_ONLINESTATETRANSMISSIONTHRESHOLD).toInt();
		distanceTransmissionThreshold = cfg.value("distanceTransmissionThreshold", DEFAULTCONFIG_DISTANCETRANSMISSIONTHRESHOLD).toInt();
	}

	std::string getConfigFilePath() {
		char* configPath = (char*)malloc(512);
		ts3Functions.getConfigPath(configPath, 512);
		std::string path = configPath;
		free(configPath);
		path.append("GW2Plugin.ini");
		return path;
	}
}
