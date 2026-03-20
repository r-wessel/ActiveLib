/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_ENCODING
#define ACTIVE_SERIALISE_ENCODING

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/string/text_encoding.h"

namespace active::serialise {
	
	/*!
		Convert an encoding name to a type
		@param name The encoding name
		@return The encoding type (nullopt if no match found)
	*/
	std::optional<text_encoding> encodingFromName(const String& name);

	
	/*!
		Convert an encoding type to a name
		@param encoding The encoding type
		@return The name of the encoding type (nullopt if no match found)
	*/
	std::optional<String> nameFromEncoding(text_encoding encoding);
	
	
	/*!
		Import the object from the specified string (specialisations for text_encoding)
		@param source The string to read
		@return True if the data was successfully read
	*/
	template<> inline
	bool ValueWrap<text_encoding>::read(const String& source) {
		if (auto encoding = encodingFromName(source); encoding) {
			base::get() = *encoding;
			return true;
		}
		return false;
	} //ValueWrap<bool>::read
	
	
	/*!
		Export the object to the specified string (specialisations for text_encoding)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueWrap<text_encoding>::write(String& dest) const {
		if (auto name = nameFromEncoding(base::get()); name) {
			dest = *name;
			return true;
		}
		return false;
	} //ValueWrap<bool>::write
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_TEXT_ENCODING
