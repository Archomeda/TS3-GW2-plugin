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

#ifndef MUMBLELINK_H
#define MUMBLELINK_H

#include <stdint.h>
#include <string>

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
	extern LinkedMem *lm;

	struct MumbleContext {
		byte serverAddress[28]; // contains sockaddr_in or sockaddr_in6
		unsigned mapId;
		unsigned mapType;
		unsigned shardId;
		unsigned instance;
		unsigned buildId;
	};

	void initLink();
	int getStatus();
	bool isGW2();
	std::string getGame();
	std::string getIdentity();
	MumbleContext* getContext();

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

	std::string getProfessionName(Profession profession);
}

#endif
