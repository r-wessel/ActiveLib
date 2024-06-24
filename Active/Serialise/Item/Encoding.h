/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_ENCODING
#define ACTIVE_SERIALISE_ENCODING

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Utility/TextEncoding.h"

namespace active::serialise {
	
	/*!
		Import the object from the specified string (specialisations for TextEncoding)
		@param source The string to read
		@return True if the data was successfully read
	*/
	template<> inline
	bool ValueWrap<utility::TextEncoding>::read(const utility::String& source) {
		if (auto encoding = encodingFromName(source); encoding) {
			base::get() = *encoding;
			return true;
		}
		return false;
	} //ValueWrap<bool>::read
	
	
	/*!
		Export the object to the specified string (specialisations for TextEncoding)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueWrap<utility::TextEncoding>::write(utility::String& dest) const {
		if (auto name = nameFromEncoding(base::get()); name) {
			dest = *name;
			return true;
		}
		return false;
	} //ValueWrap<bool>::write
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_TEXT_ENCODING
