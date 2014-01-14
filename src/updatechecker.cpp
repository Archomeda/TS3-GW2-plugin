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

#include <algorithm>
#include <string>
#include "rapidjson/document.h"
#include "globals.h"
#include "http.h"
#include "stringutils.h"
#include "updatechecker.h"
using namespace std;

#if _DEBUG
#define debuglog(str, ...) printf(str, __VA_ARGS__);
#else
#define debuglog(str, ...)
#endif

struct isnotdigit { bool operator()(char c) { return !isdigit(c); } };

const std::string githubAPI_tagsURL = "https://api.github.com/repos/Archomeda/TS3-GW2-plugin/tags";
const std::string github_releaseURL = "https://github.com/Archomeda/TS3-GW2-plugin/releases/tag/%s";


Version::Version(const string& versionString) {
	major = minor = build = revision = postfixUnstableNumber = 0;
	this->versionString = versionString;
	vector<string> version = split(versionString, '-', 10);
	if (version.size() == 0)
		return;
	vector<string> numbers = split(version.at(0), '.', 10);
	
	if (version.size() >= 2) {
		string s = version.at(1);
		string postfixNumber = s;
		postfixNumber.erase(remove_if(postfixNumber.begin(), postfixNumber.end(), isnotdigit()), postfixNumber.end());
		if (!postfixNumber.empty()) {
			postfixUnstable = s.substr(0, s.find(postfixNumber, 0));
			postfixUnstableNumber = atoi(postfixNumber.c_str());
		} else {
			postfixUnstable = s;
		}
	}
	if (numbers.size() >= 1)
		major = atoi(numbers.at(0).c_str());
	if (numbers.size() >= 2)
		minor = atoi(numbers.at(1).c_str());
	if (numbers.size() >= 3)
		build = atoi(numbers.at(2).c_str());
	if (numbers.size() >= 4)
		revision = atoi(numbers.at(3).c_str());
}

bool checkForUpdate(Version* version, string* url) {
	return checkForUpdate(false, version, url);
}

bool checkForUpdate(bool includeUnstable, Version* version, string* url) {
	Version currentVersion = Version(PLUGIN_VERSION);
	debuglog("GW2Plugin: Current version: %s (%d.%d.%d.%d-%s%d)\n", currentVersion.getVersionString().c_str(), currentVersion.getMajor(), currentVersion.getMinor(),
		currentVersion.getBuild(), currentVersion.getRevision(), currentVersion.getPostfixUnstable().c_str(), currentVersion.getPostfixUnstableNumber());

	string data;
	if (getFromHttpUrl(githubAPI_tagsURL, &data, NULL)) {
		rapidjson::Document json;
		json.Parse<0>(data.c_str());
		if (json.IsArray()) {
			for (rapidjson::SizeType i = 0; i < json.Size(); i++) {
				if (json[i].HasMember("name")) {
					string tagName = json[i]["name"].GetString();
					if (tagName.compare(0, 1, "v") == 0) {
						try {
							Version newVersion = Version(tagName.substr(1, tagName.length() - 1));
							if (!includeUnstable && !newVersion.getPostfixUnstable().empty())
								continue;

							debuglog("GW2Plugin: Found version: %s (%d.%d.%d.%d-%s%d)\n", newVersion.getVersionString().c_str(), newVersion.getMajor(), newVersion.getMinor(),
								newVersion.getBuild(), newVersion.getRevision(), newVersion.getPostfixUnstable().c_str(), newVersion.getPostfixUnstableNumber());
							if (newVersion > currentVersion) {
								*version = newVersion;
								char urlRelease[128];
								sprintf_s(urlRelease, github_releaseURL.c_str(), tagName.c_str());
								*url = string(urlRelease);
								debuglog("GW2Plugin: Newer version is available: %s\n", urlRelease);
								return true;
							} else {
								debuglog("GW2Plugin: No newer version is available\n");
								return false;
							}
						} catch (...) { }
					}
				}
			}
		}
	}
	debuglog("GW2Plugin: No newer version is available\n");
	return false;
}
