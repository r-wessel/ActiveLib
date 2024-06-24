/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_TEXT_ENCODING
#define ACTIVE_UTILITY_TEXT_ENCODING

#include <optional>

namespace active::utility {
	
	class String;
		
	enum class TextEncoding {
		UTF8 = 0,
		UTF16,
		UTF32,
		ascii,
		ISO8859_1,
	};

	
	/*!
		Convert an encoding name to a type
		@param name The encoding name
		@return The encoding type (nullopt if no match found)
	*/
	std::optional<TextEncoding> encodingFromName(const String& name);

	
	/*!
		Convert an encoding type to a name
		@param encoding The encoding type
		@return The name of the encoding type (nullopt if no match found)
	*/
	std::optional<String> nameFromEncoding(TextEncoding encoding);

}  // namespace active::utility

#endif	//ACTIVE_UTILITY_TEXT_ENCODING
