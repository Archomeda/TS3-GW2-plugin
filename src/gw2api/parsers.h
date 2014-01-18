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
#include "rapidjson/document.h"
#include "objects.h"
#include "requests.h"


namespace Gw2Api {

	namespace Parsers {

		typedef rapidjson::Document RJDoc;
		typedef rapidjson::Value RJValue;
		typedef rapidjson::SizeType RJSizeType;
		typedef rapidjson::Value::ConstMemberIterator RJIterator;


		template<class T>
		class ApiResponseParser {
		protected:
			virtual bool parseJsonString(const std::string& jsonString, RJDoc* result) const {
				result->Parse<0>(jsonString.c_str());
				return true;
			}

		public:
			virtual bool parse(const RJValue& jsonValue, T* result) const = 0;

			virtual bool parse(const std::string& jsonString, T* result) const {
				RJDoc jsonObj;
				parseJsonString(jsonString, &jsonObj);
				return parse(jsonObj, result);
			}
		};

		template<class T>
		class ArrayParser : public ApiResponseParser<std::vector<T>> {
		public:
			bool parse(const RJValue& jsonValue, std::vector<T>* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsArray()) return false;

				for (RJSizeType i = 0; i < jsonValue.Size(); i++) {
					if (!jsonValue[i].IsNull() && jsonValue[i].IsInt()) result->push_back((T)jsonValue[i].GetInt());
					else if (!jsonValue[i].IsNull() && jsonValue[i].IsInt64()) result->push_back((T)jsonValue[i].GetInt64());
					else if (!jsonValue[i].IsNull() && jsonValue[i].IsUint()) result->push_back((T)jsonValue[i].GetUint64());
					else if (!jsonValue[i].IsNull() && jsonValue[i].IsUint64()) result->push_back((T)jsonValue[i].GetUint64());
					else if (!jsonValue[i].IsNull() && jsonValue[i].IsDouble()) result->push_back((T)jsonValue[i].GetDouble());
				}
				return true;
			}
		};

		template<class T, class P>
		class EntryCollectionParser : public ApiResponseParser<EntryCollection<T>> {
		public:
			bool parse(const RJValue& jsonValue, EntryCollection<T>* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsArray()) return false;
				
				P parser;
				for (RJSizeType i = 0; i < jsonValue.Size(); i++) {
					T entry;
					if (parser.parse(jsonValue[i], &entry)) {
						result->push_back(entry);
					} else {
						return false;
					}
				}
				return true;
			}
		};
		
		template<class K, class V, class P>
		class EntryDictionaryParser : public ApiResponseParser<EntryDictionary<K, V>> {
		public:
			bool parse(const RJValue& jsonValue, EntryDictionary<K, V>* result) const {
				return false;
			}
		};

		template<class V, class P>
		class EntryDictionaryParser<std::string, V, P> : public ApiResponseParser<EntryDictionary<std::string, V>> {
		public:
			bool parse(const RJValue& jsonValue, EntryDictionary<std::string, V>* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				P parser;
				for (RJIterator i = jsonValue.MemberBegin(); i != jsonValue.MemberEnd(); i++) {
					std::string key = i->name.GetString();
					V entry;
					if (parser.parse(i->value, &entry)) {
						result->insert(EntryDictionary<std::string, V>::value_type(key, entry));
					} else {
						return false;
					}
				}
				return true;
			}
		};

		template<class V, class P>
		class EntryDictionaryParser<int, V, P> : public ApiResponseParser<EntryDictionary<int, V>> {
		public:
			bool parse(const RJValue& jsonValue, EntryDictionary<int, V>* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				P parser;
				for (RJIterator i = jsonValue.MemberBegin(); i != jsonValue.MemberEnd(); i++) {
					int key = atoi(i->name.GetString());
					V entry;
					if (parser.parse(i->value, &entry)) {
						result->insert(EntryDictionary<int, V>::value_type(key, entry));
					} else {
						return false;
					}
				}
				return true;
			}
		};


		class Vector2DParser : public ApiResponseParser<Vector2D> {
		public:
			bool parse(const RJValue& jsonValue, Vector2D* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsArray()) return false;

				const RJValue& rj_x = jsonValue[0u];
				const RJValue& rj_y = jsonValue[1];

				if (!rj_x.IsNull() && rj_x.IsNumber()) result->x = rj_x.GetDouble();
				if (!rj_y.IsNull() && rj_y.IsNumber()) result->y = rj_y.GetDouble();
				return true;
			}
		};

		class RectParser : public ApiResponseParser<Rect> {
		public:
			bool parse(const RJValue& jsonValue, Rect* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsArray()) return false;

				const RJValue& rj_upperleft = jsonValue[0u];
				const RJValue& rj_bottomright = jsonValue[1];

				bool success = true;
				Vector2DParser vector2DParser;
				if (!rj_upperleft.IsNull())		success &= vector2DParser.parse(rj_upperleft, &result->upperLeft);
				if (!rj_bottomright.IsNull())	success &= vector2DParser.parse(rj_bottomright, &result->bottomRight);
				return success;
			}
		};

		class PointOfInterestParser : public ApiResponseParser<PointOfInterestEntry> {
		public:
			bool parse(const RJValue& jsonValue, PointOfInterestEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_poi_id = jsonValue["poi_id"];
				const RJValue& rj_name = jsonValue["name"];
				const RJValue& rj_type = jsonValue["type"];
				const RJValue& rj_floor = jsonValue["floor"];
				const RJValue& rj_coord = jsonValue["coord"];

				if (!rj_poi_id.IsNull() && rj_poi_id.IsInt())	result->poi_id = rj_poi_id.GetInt();
				if (!rj_name.IsNull() && rj_name.IsString())	result->name = rj_name.GetString();
				if (!rj_type.IsNull() && rj_type.IsString())	result->type = rj_type.GetString();
				if (!rj_floor.IsNull() && rj_floor.IsInt())		result->floor = rj_floor.GetInt();
				bool success = true;
				Vector2DParser vector2DParser;
				if (!rj_coord.IsNull()) success &= vector2DParser.parse(rj_coord, &result->coord);
				return success;
			}
		};
		typedef EntryCollectionParser<PointOfInterestEntry, PointOfInterestParser> PointsOfInterestParser;

		class TaskParser : public ApiResponseParser<TaskEntry> {
		public:
			bool parse(const RJValue& jsonValue, TaskEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_task_id = jsonValue["task_id"];
				const RJValue& rj_objective = jsonValue["objective"];
				const RJValue& rj_level = jsonValue["level"];
				const RJValue& rj_coord = jsonValue["coord"];

				if (!rj_task_id.IsNull() && rj_task_id.IsInt())			result->task_id = rj_task_id.GetInt();
				if (!rj_objective.IsNull() && rj_objective.IsString())	result->objective = rj_objective.GetString();
				if (!rj_level.IsNull() && rj_level.IsInt())				result->level = rj_level.GetInt();
				bool success = true;
				Vector2DParser vector2DParser;
				if (!rj_coord.IsNull()) success &= vector2DParser.parse(rj_coord, &result->coord);
				return success;
			}
		};
		typedef EntryCollectionParser<TaskEntry, TaskParser> TasksParser;

		class SkillChallengeParser : public ApiResponseParser<SkillChallengeEntry> {
		public:
			bool parse(const RJValue& jsonValue, SkillChallengeEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_coord = jsonValue["coord"];

				bool success = true;
				Vector2DParser vector2DParser;
				if (!rj_coord.IsNull()) success &= vector2DParser.parse(rj_coord, &result->coord);
				return success;
			}
		};
		typedef EntryCollectionParser<SkillChallengeEntry, SkillChallengeParser> SkillChallengesParser;

		class SectorParser : public ApiResponseParser<SectorEntry> {
		public:
			bool parse(const RJValue& jsonValue, SectorEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_sector_id = jsonValue["sector_id"];
				const RJValue& rj_name = jsonValue["name"];
				const RJValue& rj_level = jsonValue["level"];
				const RJValue& rj_coord = jsonValue["coord"];

				if (!rj_sector_id.IsNull() && rj_sector_id.IsInt())	result->sector_id = rj_sector_id.GetInt();
				if (!rj_name.IsNull() && rj_name.IsString())		result->name = rj_name.GetString();
				if (!rj_level.IsNull() && rj_level.IsInt())			result->level = rj_level.GetInt();
				bool success = true;
				Vector2DParser vector2DParser;
				if (!rj_coord.IsNull()) success &= vector2DParser.parse(rj_coord, &result->coord);
				return success;
			}
		};
		typedef EntryCollectionParser<SectorEntry, SectorParser> SectorsParser;

		class MapFloorParser : public ApiResponseParser<MapFloorEntry> {
		public:
			bool parse(const RJValue& jsonValue, MapFloorEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_name = jsonValue["name"];
				const RJValue& rj_min_level = jsonValue["min_level"];
				const RJValue& rj_max_level = jsonValue["max_level"];
				const RJValue& rj_default_floor = jsonValue["default_floor"];
				const RJValue& rj_map_rect = jsonValue["map_rect"];
				const RJValue& rj_continent_rect = jsonValue["continent_rect"];
				const RJValue& rj_points_of_interest = jsonValue["points_of_interest"];
				const RJValue& rj_tasks = jsonValue["tasks"];
				const RJValue& rj_skill_challenges = jsonValue["skill_challenges"];
				const RJValue& rj_sectors = jsonValue["sectors"];

				if (!rj_name.IsNull() && rj_name.IsString())				result->name = rj_name.GetString();
				if (!rj_min_level.IsNull() && rj_min_level.IsInt())			result->min_level = rj_min_level.GetInt();
				if (!rj_max_level.IsNull() && rj_max_level.IsInt())			result->max_level = rj_max_level.GetInt();
				if (!rj_default_floor.IsNull() && rj_default_floor.IsInt())	result->default_floor = rj_default_floor.GetInt();
				bool success = true;
				RectParser rectParser;
				PointsOfInterestParser pointsOfInterestParser;
				TasksParser tasksParser;
				SkillChallengesParser skillChallengesParser;
				SectorsParser sectorsParser;
				if (!rj_map_rect.IsNull())				success &= rectParser.parse(rj_map_rect, &result->map_rect);
				if (!rj_continent_rect.IsNull())		success &= rectParser.parse(rj_continent_rect, &result->continent_rect);
				if (!rj_points_of_interest.IsNull())	success &= pointsOfInterestParser.parse(rj_points_of_interest, &result->points_of_interest);
				if (!rj_tasks.IsNull())					success &= tasksParser.parse(rj_tasks, &result->tasks);
				if (!rj_skill_challenges.IsNull())		success &= skillChallengesParser.parse(rj_skill_challenges, &result->skill_challenges);
				if (!rj_sectors.IsNull())				success &= sectorsParser.parse(rj_sectors, &result->sectors);
				return success;
			}
		};
		typedef EntryDictionaryParser<int, MapFloorEntry, MapFloorParser> MapFloorsParser;

		class MapFloorRegionParser : public ApiResponseParser<MapFloorRegionEntry> {
		public:
			bool parse(const RJValue& jsonValue, MapFloorRegionEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_name = jsonValue["name"];
				const RJValue& rj_label_coord = jsonValue["label_coord"];
				const RJValue& rj_maps = jsonValue["maps"];

				if (!rj_name.IsNull() && rj_name.IsString()) result->name = rj_name.GetString();
				bool success = true;
				Vector2DParser vector2DParser;
				MapFloorsParser mapFloorsParser;
				if (!rj_label_coord.IsNull())	success &= vector2DParser.parse(rj_label_coord, &result->label_coord);
				if (!rj_maps.IsNull())			success &= mapFloorsParser.parse(rj_maps, &result->maps);
				return success;
			}
		};
		typedef EntryDictionaryParser<int, MapFloorRegionEntry, MapFloorRegionParser> MapFloorRegionsParser;

		class MapFloorRootParser : public ApiResponseParser<MapFloorRootEntry> {
		public:
			bool parse(const RJValue& jsonValue, MapFloorRootEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_texture_dims = jsonValue["texture_dims"];
				const RJValue& rj_clamped_view = jsonValue["clamped_view"];
				const RJValue& rj_regions = jsonValue["regions"];

				bool success = true;
				Vector2DParser vector2DParser;
				RectParser rectParser;
				MapFloorRegionsParser mapFloorRegionsParser;
				if (!rj_texture_dims.IsNull())	success &= vector2DParser.parse(rj_texture_dims, &result->texture_dims);
				if (!rj_clamped_view.IsNull())	success &= rectParser.parse(rj_clamped_view, &result->clamped_view);
				if (!rj_regions.IsNull())		success &= mapFloorRegionsParser.parse(rj_regions, &result->regions);
				return success;
			}
		};

		class MapParser : public ApiResponseParser<MapEntry> {
		public:
			bool parse(const RJValue& jsonValue, MapEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;

				const RJValue& rj_map_name = jsonValue["map_name"];
				const RJValue& rj_min_level = jsonValue["min_level"];
				const RJValue& rj_max_level = jsonValue["max_level"];
				const RJValue& rj_default_floor = jsonValue["default_floor"];
				const RJValue& rj_floors = jsonValue["floors"];
				const RJValue& rj_region_id = jsonValue["region_id"];
				const RJValue& rj_region_name = jsonValue["region_name"];
				const RJValue& rj_continent_id = jsonValue["continent_id"];
				const RJValue& rj_continent_name = jsonValue["continent_name"];
				const RJValue& rj_map_rect = jsonValue["map_rect"];
				const RJValue& rj_continent_rect = jsonValue["continent_rect"];

				if (!rj_map_name.IsNull() && rj_map_name.IsString())				result->map_name = rj_map_name.GetString();
				if (!rj_min_level.IsNull() && rj_min_level.IsInt())					result->min_level = rj_min_level.GetInt();
				if (!rj_max_level.IsNull() && rj_max_level.IsInt())					result->max_level = rj_max_level.GetInt();
				if (!rj_default_floor.IsNull() && rj_default_floor.IsInt())			result->default_floor = rj_default_floor.GetInt();
				if (!rj_region_id.IsNull() && rj_region_id.IsInt())					result->region_id = rj_region_id.GetInt();
				if (!rj_region_name.IsNull() && rj_region_name.IsString())			result->region_name = rj_region_name.GetString();
				if (!rj_continent_id.IsNull() && rj_continent_id.IsInt())			result->continent_id = rj_continent_id.GetInt();
				if (!rj_continent_name.IsNull() && rj_continent_name.IsString())	result->continent_name = rj_continent_name.GetString();
				bool success = true;
				ArrayParser<int> arrayParserInt;
				RectParser rectParser;
				if (!rj_floors.IsNull())			success &= arrayParserInt.parse(rj_floors, &result->floors);
				if (!rj_map_rect.IsNull())			success &= rectParser.parse(rj_map_rect, &result->map_rect);
				if (!rj_continent_rect.IsNull())	success &= rectParser.parse(rj_continent_rect, &result->continent_rect);
				return success;
			}
		};
		typedef EntryDictionaryParser<int, MapEntry, MapParser> MapsParser;
		
		class MapsRootParser : public ApiResponseParser<MapsRootEntry> {
		public:
			bool parse(const RJValue& jsonValue, MapsRootEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject() || jsonValue["maps"].IsNull()) return false;
				
				const RJValue& rj_maps = jsonValue["maps"];

				bool success = true;
				MapsParser mapsParser;
				if (!rj_maps.IsNull()) success &= mapsParser.parse(rj_maps, &result->maps);
				return success;
			}
		};
		
		class WorldNameParser : public ApiResponseParser<WorldNameEntry> {
		public:
			bool parse(const RJValue& jsonValue, WorldNameEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsObject()) return false;
				
				const RJValue& rj_id = jsonValue["id"];
				const RJValue& rj_name = jsonValue["name"];

				if (!rj_id.IsNull() && rj_id.IsString()) result->id = atoi(rj_id.GetString());
				if (!rj_name.IsNull() && rj_name.IsString()) result->name = rj_name.GetString();
				return true;
			}
		};
		typedef EntryCollectionParser<WorldNameEntry, WorldNameParser> WorldNamesParser;

		class WorldNamesRootParser : public ApiResponseParser<WorldNamesRootEntry> {
		public:
			bool parse(const RJValue& jsonValue, WorldNamesRootEntry* result) const {
				if (jsonValue.IsNull() || !jsonValue.IsArray()) return false;
				
				WorldNamesParser worldNamesParser;
				EntryCollection<WorldNameEntry> worldNameEntries;
				if (worldNamesParser.parse(jsonValue, &worldNameEntries)) {
					for (size_t i = 0; i < worldNameEntries.size(); i++) {
						result->world_names[worldNameEntries[i].id] = worldNameEntries[i];
					}
					return true;
				}
				return false;
			}
		};
	
	}

}
