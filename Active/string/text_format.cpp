/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/string/text_format.h"

#include <array>

using namespace active;

using enum text_encoding;

namespace {
	
		// MARK: - Constants
	
	const std::array utf8BOM = { '\xEF', '\xBB', '\xBF' };
	
	const std::array utf16BEBOM = { '\xFE', '\xFF' };
	
	const std::array utf16LEBOM = { '\xFF', '\xFE' };
	
	const std::array utf32BEBOM = { '\x00', '\x00', '\xFE', '\xFF' };
	
	const std::array utf32LEBOM = { '\xFF', '\xFE', '\x00', '\x00' };
	
}

/*--------------------------------------------------------------------
	Create a data format from a BOM signature
 
	bom: A series of bytes to test for a BOM signature
	len: The number of available bytes
 
	return: The data format paired with the number of bytes in the BOM (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::pair<text_format, size_t>> text_format::from_bom(const char* bom, size_t len) {
	if (len < 4)
		return std::nullopt;	//Insufficient data to be an encoding and a character
	if ((bom[0] == utf32BEBOM[0]) && (bom[1] == utf32BEBOM[1]) && (bom[2] == utf32BEBOM[2]) && (bom[3] == utf32BEBOM[3]))
		return std::make_pair(text_format{UTF32, true, true}, 4);
	if ((bom[0] == utf32LEBOM[0]) && (bom[1] == utf32LEBOM[1]) && (bom[2] == utf32LEBOM[2]) && (bom[3] == utf32LEBOM[3]))
		return std::make_pair(text_format{UTF32, true, false}, 4);
	if ((bom[0] == utf8BOM[0]) && (bom[1] == utf8BOM[1]) && (bom[2] == utf8BOM[2]))
		return std::make_pair(text_format{UTF8, true, true}, 3);
	if ((bom[0] == utf16BEBOM[0]) && (bom[1] == utf16BEBOM[1]))
		return std::make_pair(text_format{UTF16, true, true}, 2);
	if ((bom[0] == utf16LEBOM[0]) && (bom[1] == utf16LEBOM[1]))
		return std::make_pair(text_format{UTF16, true, false}, 2);
	return std::nullopt;
} //text_format::bomToFormat
