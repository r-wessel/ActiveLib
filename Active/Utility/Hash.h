/*!
@copyright Copyright 2024 Ralph Wessel and Hugh Wessel
@license Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_HASH
#define ACTIVE_UTILITY_HASH

namespace active::utility {

		///Formatting options for hash generators
	enum HashFormat {
		asHex,	///>Format the hash as hex digits
		asBase64,	///>Format the hash as base64 digits
		asGuid,	///>Format as a GUID (128 bits - loses complexity)
	};
	
}

#endif	//ACTIVE_UTILITY_HASH
