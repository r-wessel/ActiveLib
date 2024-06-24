/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/TextEncoding.h"

#include "Active/Utility/String.h"

#include <algorithm>
#include <map>

using namespace active::utility;

namespace {
	
		//All recognised encoding types
	std::map<TextEncoding, String> encodingNames = {
		{ TextEncoding::UTF8, "utf-8" },
		{ TextEncoding::UTF16, "utf-16" },
		{ TextEncoding::UTF32, "utf-32" },
		{ TextEncoding::ISO8859_1, "iso-8859-1" },
	};
	
}  // namespace

/*--------------------------------------------------------------------
	Convert an encoding name to a type
 
	name: The encoding name
 
	return: The encoding type (nullopt if no match found)
 --------------------------------------------------------------------*/
std::optional<TextEncoding> active::utility::encodingFromName(const String& name) {
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
std::optional<String> active::utility::nameFromEncoding(TextEncoding encoding) {
	if (auto match = encodingNames.find(encoding); match != encodingNames.end()) {
		return match->second;
	}
	return std::nullopt;
} //nameFromEncoding
