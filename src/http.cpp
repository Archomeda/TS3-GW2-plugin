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
#include <Windows.h>
#include <WinInet.h>
#include "http.h"
using namespace std;

bool getFromHttpUrl(const string& url, string* result, long unsigned* lastError) {
	HINTERNET hSession = InternetOpenA("TS3GW2Plugin", 0, NULL, NULL, 0);
	if (hSession == NULL) {
		*lastError = GetLastError();
		return false;
	}

	HINTERNET hOpenUrl = InternetOpenUrlA(hSession, url.c_str(), NULL, 0, 1, 1);
	if (hOpenUrl == NULL) {
		*lastError = GetLastError();
		InternetCloseHandle(hOpenUrl);
		return false;
	}

	char* buffer = new char[1025];
	DWORD bytesRead = 0;
	while (true) {
		if (InternetReadFile(hOpenUrl, buffer, 1024, &bytesRead)) {
			if (bytesRead == 0)
				break;
			buffer[bytesRead] = 0;
			*result += buffer;
		} else {
			*lastError = GetLastError();
			delete[] buffer;
			InternetCloseHandle(hOpenUrl);
			InternetCloseHandle(hSession);
			return false;
		}
	}
	delete[] buffer;

	InternetCloseHandle(hOpenUrl);
	InternetCloseHandle(hSession);
	return true;
}
