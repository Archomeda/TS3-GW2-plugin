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
#include <vector>
#include <time.h>
#include "math.h"
#include "requests.h"

namespace Gw2Api {

	struct ApiResponseObject {
		virtual ~ApiResponseObject() { }

		Requests::ApiRequest request;
		time_t requestTime;
		double getAge() { return difftime(time(NULL), requestTime); }
		bool isCached;
	};

	template<class T>
	struct EntryCollection : public std::vector<T> {
		~EntryCollection() { }
	};

	template<class K, class V>
	struct EntryDictionary : public std::map<K, V> { 
		~EntryDictionary() { }
	};


	template<class R, class V>
	struct ApiInnerResponseObject {
		ApiInnerResponseObject() { }

		ApiInnerResponseObject(const R* root, const V* value) {
			this->root = *root;
			this->value = *value;
		}

		R root;
		V value;
	};


	struct PointOfInterestEntry {
		int poi_id;
		std::string name;
		std::string type;
		int floor;
		Vector2D coord;
	};
	typedef EntryCollection<PointOfInterestEntry> PointOfInterestEntries;

	struct TaskEntry {
		int task_id;
		std::string objective;
		int level;
		Vector2D coord;
	};
	typedef EntryCollection<TaskEntry> TaskEntries;

	struct SkillChallengeEntry {
		Vector2D coord;
	};
	typedef EntryCollection<SkillChallengeEntry> SkillChallengeEntries;

	struct SectorEntry {
		int sector_id;
		std::string name;
		int level;
		Vector2D coord;
	};
	typedef EntryCollection<SectorEntry> SectorEntries;

	struct MapFloorEntry {
		std::string name;
		int min_level;
		int max_level;
		int default_floor;
		Rect map_rect;
		Rect continent_rect;
		PointOfInterestEntries points_of_interest;
		TaskEntries tasks;
		SkillChallengeEntries skill_challenges;
		SectorEntries sectors;
	};

	struct MapFloorEntries : public ApiResponseObject, public EntryDictionary<int, MapFloorEntry> {
		~MapFloorEntries() { }
	};

	struct MapFloorRegionEntry {
		std::string name;
		Vector2D label_coord;
		MapFloorEntries maps;
	};
	typedef EntryDictionary<int, MapFloorRegionEntry> MapFloorRegionEntries;

	struct MapFloorRootEntry : public ApiResponseObject {
		~MapFloorRootEntry() { }

		Vector2D texture_dims;
		Rect clamped_view;
		MapFloorRegionEntries regions;
	};

	struct MapEntry {
		std::string map_name;
		int min_level;
		int max_level;
		int default_floor;
		std::vector<int> floors;
		int region_id;
		std::string region_name;
		int continent_id;
		std::string continent_name;
		Rect map_rect;
		Rect continent_rect;
	};
	typedef EntryDictionary<int, MapEntry> MapEntries;

	struct MapsRootEntry : public ApiResponseObject {
		~MapsRootEntry() { }

		MapEntries maps;
	};

	struct WorldNameEntry {
		int id;
		std::string name;
	};
	typedef EntryDictionary<int, WorldNameEntry> WorldNameEntries;

	struct WorldNamesRootEntry : public ApiResponseObject {
		~WorldNamesRootEntry() { }

		WorldNameEntries world_names;
	};

}
