/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/DataFormat.h"

#include <array>

using namespace active::utility;

using enum TextEncoding;

namespace {

	std::array utf8BOM = { '\xEF', '\xBB', '\xBF' };

	std::array utf16BEBOM = { '\xFE', '\xFF' };

	std::array utf16LEBOM = { '\xFF', '\xFE' };

	std::array utf32BEBOM = { '\x00', '\x00', '\xFE', '\xFF' };

	std::array utf32LEBOM = { '\xFF', '\xFE', '\x00', '\x00' };
}  // namespace

/*--------------------------------------------------------------------
	Create a data format from a BOM signature
 
	bom: A series of bytes to test for a BOM signature
	len: The number of available bytes
 
	return: The data format paired with the number of bytes in the BOM (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::pair<DataFormat, Memory::size_type>> DataFormat::fromBOM(const char* bom, Memory::size_type len) {
	if (len < 4)
		return std::nullopt;	//Insufficient data to be an encoding and a character
	if ((bom[0] == utf32BEBOM[0]) && (bom[1] == utf32BEBOM[1]) && (bom[2] == utf32BEBOM[2]) && (bom[3] == utf32BEBOM[3]))
		return std::make_pair(DataFormat{UTF32, true, true}, 4);
	if ((bom[0] == utf32LEBOM[0]) && (bom[1] == utf32LEBOM[1]) && (bom[2] == utf32LEBOM[2]) && (bom[3] == utf32LEBOM[3]))
		return std::make_pair(DataFormat{UTF32, true, false}, 4);
	if ((bom[0] == utf8BOM[0]) && (bom[1] == utf8BOM[1]) && (bom[2] == utf8BOM[2]))
		return std::make_pair(DataFormat{UTF8, true, true}, 3);
	if ((bom[0] == utf16BEBOM[0]) && (bom[1] == utf16BEBOM[1]))
		return std::make_pair(DataFormat{UTF16, true, true}, 2);
	if ((bom[0] == utf16LEBOM[0]) && (bom[1] == utf16LEBOM[1]))
		return std::make_pair(DataFormat{UTF16, true, false}, 2);
	return std::nullopt;
} //DataFormat::bomToFormat


/*--------------------------------------------------------------------
	Get a BOM signature for the format
 
	return: The BOM signature (nullopt if none defined, e.g. for ascii)
  --------------------------------------------------------------------*/
Memory::Option DataFormat::toBOM() const {
	switch (encoding) {
		case UTF8:
			return Memory{utf8BOM.data(), utf8BOM.size()};
			break;
		case UTF16:
			return Memory{isBigEndian ? utf16BEBOM.data() : utf16LEBOM.data(), utf16BEBOM.size()};
			break;
		case UTF32:
			return Memory{isBigEndian ? utf32BEBOM.data() : utf32LEBOM.data(), utf32BEBOM.size()};
			break;
		default:
			break;
	}
	return std::nullopt;
} //DataFormat::toBOM
