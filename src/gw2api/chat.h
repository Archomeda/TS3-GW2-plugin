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
#include <stdint.h>
#include <vector>
#include "base64.h"

namespace Gw2Api {

	namespace ChatLink {

		inline std::string poiToChatLink(uint32_t poi_id) {
			std::vector<unsigned char> input;
			input.push_back(0x04); // Map link ID
			input.push_back((unsigned char)poi_id);
			input.push_back((unsigned char)(poi_id >> 8));
			input.push_back((unsigned char)(poi_id >> 16));
			input.push_back((unsigned char)(poi_id >> 24));
			return base64Encode(input);
		}

	}

}