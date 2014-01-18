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

class Version {

private:
	std::string versionString;
	int major;
	int minor;
	int build;
	int revision;
	std::string postfixUnstable;
	int postfixUnstableNumber;

public:
	Version() {	major = minor = build = revision = postfixUnstableNumber = 0; }
	/* Version strings here need to be in the format of major[.minor[.revision[.build]]][-postfix[number]] */
	Version(const std::string& versionString);

	std::string getVersionString() { return versionString; }
	int getMajor() { return major; }
	int getMinor() { return minor; }
	int getRevision() { return revision; }
	int getBuild() { return build; }
	std::string getPostfixUnstable() { return postfixUnstable; }
	int getPostfixUnstableNumber() { return postfixUnstableNumber; }

	friend inline bool operator==(const Version& lhs, const Version& rhs) {
		return lhs.major == rhs.major && lhs.minor == rhs.minor
			&& lhs.build == rhs.build && lhs.revision == rhs.revision
			&& lhs.postfixUnstable == rhs.postfixUnstable && lhs.postfixUnstableNumber == rhs.postfixUnstableNumber;
	}
	friend inline bool operator!=(const Version& lhs, const Version& rhs) { return !operator==(lhs, rhs); }
	friend inline bool operator<(const Version& lhs, const Version& rhs) {
		return lhs.major < rhs.major || lhs.minor < rhs.minor
			|| lhs.build < rhs.build || lhs.revision < rhs.revision
			|| !lhs.postfixUnstable.empty() && (rhs.postfixUnstable.empty() || lhs.postfixUnstable < rhs.postfixUnstable || lhs.postfixUnstableNumber < rhs.postfixUnstableNumber);
	}
	friend inline bool operator>(const Version& lhs, const Version& rhs) { return operator<(rhs, lhs); }
	friend inline bool operator<=(const Version& lhs, const Version& rhs) { return !operator<(rhs, lhs); }
	friend inline bool operator>=(const Version& lhs, const Version& rhs) { return !operator<(lhs, rhs); }

};


bool checkForUpdate(Version& version, std::string& url);
bool checkForUpdate(bool includeUnstable, Version& version, std::string& url);

