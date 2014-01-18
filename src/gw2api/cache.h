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
#include <map>
#include "objects.h"
#include "requests.h"

namespace Gw2Api {
	
	namespace Cache {

		static std::map<std::string, ApiResponseObject*> cacheObjects;

		inline void removeCacheObject(const std::string& url) {
			std::map<std::string, ApiResponseObject*>::iterator it = cacheObjects.find(url);
			if (it != cacheObjects.end()) {
				delete it->second;
				cacheObjects.erase(it);
			}
		}

		inline void clearCache() {
			for (std::map<std::string, ApiResponseObject*>::iterator it = cacheObjects.begin(); it != cacheObjects.end(); it++) {
				delete it->second;
			}
			cacheObjects.clear();
		}

		template<class T>
		inline void addCacheObject(T* object) {
			T* obj = new T(*object);
			ApiResponseObject* cacheObject = obj;
			T* temp = static_cast<T*>(cacheObject);
			cacheObject->isCached = true;
			std::string url = cacheObject->request.getFullUrl();
			cacheObjects[url] = cacheObject;
		}


		static bool getNewerCachedObject(const std::string& urlA, const std::string& urlB, ApiResponseObject** object) {
			std::map<std::string, ApiResponseObject*>::iterator itA = cacheObjects.find(urlA);
			std::map<std::string, ApiResponseObject*>::iterator itB = cacheObjects.find(urlB);
			if (itA != cacheObjects.end() && itB != cacheObjects.end()) {
				if (itA->second->requestTime > itB->second->requestTime) {
					*object = itA->second;
				} else {
					*object = itB->second;
				}
				return true;
			} else if (itA != cacheObjects.end()) {
				*object = itA->second;
				return true;
			} else if (itB != cacheObjects.end()) {
				*object = itB->second;
				return true;
			}
			return false;
		}


		template<class T>
		inline bool getCachedObject(const Requests::ApiRequest& request, T* response) {
			std::map<std::string, ApiResponseObject*>::iterator it = cacheObjects.find(request.getFullUrl());
			if (it != cacheObjects.end()) {
				*response = T(*dynamic_cast<T*>(it->second));
				return true;
			}
			return false;
		}

		template<>
		inline bool getCachedObject(const Requests::ApiRequest& request, MapEntries* response) {
			ApiResponseObject* object = NULL;
			if (getNewerCachedObject(request.url, request.getFullUrl(), &object)) {
				*response = MapEntries(*dynamic_cast<MapEntries*>(object));
				return true;
			}
			return false;
		}

	}
	
}
