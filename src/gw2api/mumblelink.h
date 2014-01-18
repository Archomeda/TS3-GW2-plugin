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
#include <codecvt>
#include <locale>
#include <stdint.h>
#include <string>
#include <Windows.h>
#include "rapidjson/document.h"
#include "math.h"
#include "parsers.h"

namespace Gw2Api {

	namespace MumbleLink {

		struct LinkedMem {
			uint32_t uiVersion;
			uint32_t uiTick;
			float	fAvatarPosition[3];
			float	fAvatarFront[3];
			float	fAvatarTop[3];
			wchar_t	name[256];
			float	fCameraPosition[3];
			float	fCameraFront[3];
			float	fCameraTop[3];
			wchar_t	identity[256];
			uint32_t context_len;
			unsigned char context[256];
			wchar_t description[2048];
		};

		static LinkedMem* lm;
		static uint32_t lastTick;
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		enum Profession {
			Guardian = 1,
			Warrior,
			Engineer,
			Ranger,
			Thief,
			Elementalist,
			Mesmer,
			Necromancer
		};

		inline std::string getProfessionName(Profession profession) {
			switch (profession) {
				case Guardian: return "Guardian";
				case Warrior: return "Warrior";
				case Engineer: return "Engineer";
				case Ranger: return "Ranger";
				case Thief: return "Thief";
				case Elementalist: return "Elementalist";
				case Mesmer: return "Mesmer";
				case Necromancer: return "Necromancer";
				default: return "Unknown profession";
			}
		}

		struct MumbleIdentity {
			std::string name;
			Profession profession;
			int map_id;
			int world_id;
			int team_color_id;
			bool commander;

			friend bool operator==(const MumbleIdentity& lhs, const MumbleIdentity& rhs) {
				return lhs.name == rhs.name && lhs.profession == rhs.profession && lhs.map_id == rhs.map_id
					&& lhs.world_id == rhs.world_id && lhs.team_color_id == rhs.team_color_id
					&& lhs.commander == rhs.commander;
			}

			friend bool operator!=(const MumbleIdentity& lhs, const MumbleIdentity& rhs) {
				return !(lhs == rhs);
			}
		};

		struct MumbleContext {
			byte serverAddress[28]; // contains sockaddr_in or sockaddr_in6
			unsigned mapId;
			unsigned mapType;
			unsigned shardId;
			unsigned instance;
			unsigned buildId;
		};

		inline bool initLink() {
			lm = NULL;
			lastTick = 0;

			HANDLE hMapObject = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink");
			if (hMapObject == NULL) {
				return false;
			}

			lm = (LinkedMem*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
			if (lm == NULL) {
				CloseHandle(hMapObject);
				hMapObject = NULL;
				return false;
			}

			return true;
		}

		inline bool isActive() {
			if (lm->uiTick > lastTick) {
				lastTick = lm->uiTick;
				return true;
			}
			return false;
		}

		inline std::string getGame() {
			return converter.to_bytes(lm->name);
		}

		inline bool isGW2() {
			return getGame() == "Guild Wars 2";
		}

		inline MumbleIdentity getIdentity() {
			MumbleIdentity mumbleIdentity;

			std::string identity = converter.to_bytes(lm->identity);
			Parsers::RJDoc json;
			json.Parse<0>(identity.c_str());

			const Parsers::RJValue& rj_name = json["name"];
			const Parsers::RJValue& rj_profession = json["profession"];
			const Parsers::RJValue& rj_map_id = json["map_id"];
			const Parsers::RJValue& rj_world_id = json["world_id"];
			const Parsers::RJValue& rj_team_color_id = json["team_color_id"];
			const Parsers::RJValue& rj_commander = json["commander"];

			if (!rj_name.IsNull() && rj_name.IsString())				mumbleIdentity.name = rj_name.GetString();
			if (!rj_profession.IsNull() && rj_profession.IsInt())		mumbleIdentity.profession = (Profession)rj_profession.GetInt();
			if (!rj_map_id.IsNull() && rj_map_id.IsInt())				mumbleIdentity.map_id = rj_map_id.GetInt();
			if (!rj_world_id.IsNull() && rj_world_id.IsInt())			mumbleIdentity.world_id = rj_world_id.GetInt();
			if (!rj_team_color_id.IsNull() && rj_team_color_id.IsInt())	mumbleIdentity.team_color_id = rj_team_color_id.GetInt();
			if (!rj_commander.IsNull() && rj_commander.IsInt())			mumbleIdentity.commander = rj_commander.GetInt() != 0;

			return mumbleIdentity;
		}

		inline Vector3D getAvatarPosition() {
			return Vector3D(lm->fAvatarPosition[0], lm->fAvatarPosition[1], lm->fAvatarPosition[2]);
		}

		inline MumbleContext* getContext() {
			return (MumbleContext*)lm->context;
		}
	}

}