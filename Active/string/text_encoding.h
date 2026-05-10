/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_TEXT_ENCODING
#define ACTIVE_TEXT_ENCODING

namespace active {
		
	enum class text_encoding {
		UTF8 = 0,
		UTF16,
		UTF32,
		ascii,
		ISO8859_1,
	};

}  // namespace active

#endif	//ACTIVE_TEXT_ENCODING
