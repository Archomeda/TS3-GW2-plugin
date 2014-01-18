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
#include <string>

namespace Gw2Api {

	namespace Requests {

		const std::string url_map_floor = "https://api.guildwars2.com/v1/map_floor.json";
		const std::string url_maps = "https://api.guildwars2.com/v1/maps.json";
		const std::string url_world_names = "https://api.guildwars2.com/v1/world_names.json";

		struct ApiRequest {
			ApiRequest() { }

			ApiRequest(const std::string& url) {
				this->url = url;
			}

			std::string url;
			std::map<std::string, std::string> parameters;

			std::string getFullUrl() const {
				std::string partParameters = std::string();
				for (std::map<std::string, std::string>::const_iterator i = parameters.begin(); i != parameters.end(); i++) {
					if (!partParameters.empty())
						partParameters += "&";
					partParameters += i->first;
					partParameters += "=";
					partParameters += i->second;
				}

				std::string url = std::string(this->url);
				if (!partParameters.empty()) {
					url += "?";
					url += partParameters;
				}

				return url;
			}
		};

		struct MapFloorRequest : ApiRequest {
			MapFloorRequest() {
				this->url = url_map_floor;
			}

			MapFloorRequest(const int continent_id, const int floor) {
				this->url = url_map_floor;
				setContinentID(continent_id);
				setFloor(floor);
			}

			void setContinentID(const int continent_id) {
				parameters["continent_id"] = std::to_string((long long)continent_id);
			}

			int getContinentID() {
				std::map<std::string, std::string>::iterator it = parameters.find("continent_id");
				if (it != parameters.end())
					return atoi(it->second.c_str());
				return 1;
			}

			void setFloor(const int floor) {
				parameters["floor"] = std::to_string((long long)floor);
			}

			int getFloor() {
				std::map<std::string, std::string>::iterator it = parameters.find("floor");
				if (it != parameters.end())
					return atoi(it->second.c_str());
				return 1;
			}
		};

		struct MapsRequest : ApiRequest {
			MapsRequest() {
				this->url = url_maps;
			}

			MapsRequest(const int map_id) {
				this->url = url_maps;
				setMapID(map_id);
			}

			void setMapID(const int map_id) {
				if (map_id > 0) {
					parameters["map_id"] = std::to_string((long long)map_id);
				} else {
					parameters.erase("map_id");
				}
			}
			
			int getMapID() {
				std::map<std::string, std::string>::iterator it = parameters.find("map_id");
				if (it != parameters.end())
					return atoi(it->second.c_str());
				return 0;
			}

			void removeMapID() {
				setMapID(0);
			}
		};

		struct WorldNamesRequest : ApiRequest {
			WorldNamesRequest() {
				this->url = url_world_names;
			}
		};

	}

}
