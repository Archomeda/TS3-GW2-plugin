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

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <Windows.h>
#include <locale>
#include <codecvt>
#include <string>
#include "mumblelink.h"
using namespace std;

wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;

namespace MumbleLink {

	LinkedMem *lm;
	uint32_t lastTick;

	void initLink() {
		lm = NULL;
		lastTick = 0;

		HANDLE hMapObject = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink");
		if (hMapObject == NULL) {
			printf("GW2Plugin: Mumble file mapping failed\n");
			return;
		}

		lm = (LinkedMem*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
		if (lm == NULL) {
			printf("GW2Plugin: Mumble file mapping viewing failed\n");
			CloseHandle(hMapObject);
			hMapObject = NULL;
			return;
		}
	}

	int getStatus() {
		if (lm->uiTick > lastTick) {
			lastTick = lm->uiTick;
			return 1;
		}
		return 0;
	}

	bool isGW2() {
		return getGame() == "Guild Wars 2";
	}

	string getGame() {
		return converter.to_bytes(lm->name);
	}

	string getIdentity() {
		return converter.to_bytes(lm->identity);
	}

	MumbleContext* getContext() {
		return (MumbleContext*)lm->context;
	}

	string getProfessionName(Profession profession) {
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
}