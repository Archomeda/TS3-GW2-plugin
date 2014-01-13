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

#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>
using namespace std;

void split(const string &s, char delim, size_t limit, vector<string> &elems) {
	size_t pos = 0;
	size_t i = 0;
	while (pos < s.length()) {
		size_t endPos = s.find(delim, pos);

		if (endPos < 0 || i + 1 >= limit) {
			string item = s.substr(pos, s.length() - pos);
			elems.push_back(item);
			break;
		}

		string item = s.substr(pos, endPos - pos);
		elems.push_back(item);
		pos = endPos + 1;
		i++;
	}
}

void split(const string &s, char delim, vector<string> &elems) {
	split(s, delim, 0, elems);
}

vector<string> split(const string &s, char delim, int limit) {
	vector<string> elems;
	split(s, delim, limit, elems);
	return elems;
}

vector<string> split(const string &s, char delim) {
	return split(s, delim, 0);
}

string to_string(int value) {
	return to_string((long long)value);
}
