/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Item/Encoding.h"

#include "Active/string/string_utf8.h"

#include <algorithm>
#include <map>

using namespace active;

namespace {
	
		//All recognised encoding types
	std::map<text_encoding, String> encodingNames = {
		{ text_encoding::UTF8, "utf-8" },
		{ text_encoding::UTF16, "utf-16" },
		{ text_encoding::UTF32, "utf-32" },
		{ text_encoding::ISO8859_1, "iso-8859-1" },
	};
	
}  // namespace

/*--------------------------------------------------------------------
	Convert an encoding name to a type
 
	name: The encoding name
 
	return: The encoding type (nullopt if no match found)
 --------------------------------------------------------------------*/
std::optional<text_encoding> active::serialise::encodingFromName(const String& name) {
	if (auto match = std::find_if(encodingNames.begin(), encodingNames.end(), [&](const auto& i){ return (i.second == name); });
			match != encodingNames.end()) {
		return match->first;
	}
	return std::nullopt;
} //encodingFromName


/*--------------------------------------------------------------------
	Convert an encoding type to a name
 
	encoding: The encoding type
 
	return: The name of the encoding type (nullopt if no match found)
  --------------------------------------------------------------------*/
std::optional<String> active::serialise::nameFromEncoding(text_encoding encoding) {
	if (auto match = encodingNames.find(encoding); match != encodingNames.end()) {
		return match->second;
	}
	return std::nullopt;
} //nameFromEncoding
