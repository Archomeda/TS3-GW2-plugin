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

// Slightly modified source from http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C.2B.2B

#include <string>
#include <vector>

namespace Gw2Api {

	const static char encodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const static char padCharacter = '=';
	std::string base64Encode(std::vector<unsigned char> inputBuffer)
	{
		std::string encodedString;
		encodedString.reserve(((inputBuffer.size()/3) + (inputBuffer.size() % 3 > 0)) * 4);
		long temp;
		std::vector<unsigned char>::iterator cursor = inputBuffer.begin();
		for(size_t idx = 0; idx < inputBuffer.size()/3; idx++)
		{
			temp  = (*cursor++) << 16; //Convert to big endian
			temp += (*cursor++) << 8;
			temp += (*cursor++);
			encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
			encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
			encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
			encodedString.append(1,encodeLookup[(temp & 0x0000003F)      ]);
		}
		switch(inputBuffer.size() % 3)
		{
		case 1:
			temp  = (*cursor++) << 16; //Convert to big endian
			encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
			encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
			encodedString.append(2,padCharacter);
			break;
		case 2:
			temp  = (*cursor++) << 16; //Convert to big endian
			temp += (*cursor++) << 8;
			encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
			encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
			encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
			encodedString.append(1,padCharacter);
			break;
		}
		return encodedString;
	}

}
