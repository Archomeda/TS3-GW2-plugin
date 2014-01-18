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
#include <Windows.h>
#include <WinInet.h>
#include "cache.h"
#include "parsers.h"
#include "requests.h"


namespace Gw2Api {

	static bool getFromHttpUrl(const std::string& url, std::string* result, long unsigned* lastError) {
		HINTERNET hSession = InternetOpenA("Guild Wars 2 C++ API Wrapper", 0, NULL, NULL, 0);
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

	template<class T>
	static bool handleRequest(const Requests::ApiRequest& request, const Parsers::ApiResponseParser<T>& parser, bool ignoreCache, T* response) {
		if (ignoreCache || !Cache::getCachedObject(request, response)) {
			std::string result;
			std::string url = request.getFullUrl();
			if (getFromHttpUrl(url, &result, NULL)) {
				if (parser.parse(result, response)) {
					response->request = request;
					response->requestTime = time(NULL);
					Cache::addCacheObject(response);
					return true;
				}
			}
			return false;
		}
		return true;
	}


	inline void clearCache() {
		Cache::clearCache();
	}


	inline bool getMapFloor(const int continent_id, const int floor, MapFloorRootEntry* mapFloorGlobalEntry) { 
		Requests::MapFloorRequest request = Requests::MapFloorRequest(continent_id, floor);
		Parsers::MapFloorRootParser parser;
		return handleRequest(request, parser, false, mapFloorGlobalEntry);
	}

	inline bool getMap(const int map_id, ApiInnerResponseObject<MapsRootEntry, MapEntry>* mapEntry) {
		Requests::MapsRequest request = Requests::MapsRequest(map_id);
		Parsers::MapsRootParser parser;
		if (handleRequest(request, parser, false, &mapEntry->root)) {
			MapEntries::iterator it = mapEntry->root.maps.find(map_id);
			if (it != mapEntry->root.maps.end()) {
				mapEntry->value = it->second;
				return true;
			} else {
				return false;
			}
		}
		return false;
	}

	inline bool getMaps(MapsRootEntry* mapsRootEntry) {
		Requests::MapsRequest request;
		Parsers::MapsRootParser parser;
		return handleRequest(request, parser, false, mapsRootEntry);
	}

	inline bool getWorldNames(WorldNamesRootEntry* worldNamesRootEntry) {
		Requests::WorldNamesRequest request;
		Parsers::WorldNamesRootParser parser;
		return handleRequest(request, parser, false, worldNamesRootEntry);
	}

}
