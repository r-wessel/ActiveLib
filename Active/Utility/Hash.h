/*!
@copyright Copyright 2024 Ralph Wessel and Hugh Wessel
@license Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_HASH
#define ACTIVE_UTILITY_HASH

#include "Active/Utility/Case.h"

namespace active::utility {

		///Formatting options for hash generators
	struct HashFormat {
		
			//Hash representation type
		enum Type {
			asHex,	///>Format the hash as hex digits
			asBase64,	///>Format the hash as base64 digits
			asGuid,	///>Format as a GUID (128 bits - loses complexity)
		} type = asBase64;
		
			//Hash representation case (not relevant to all representations)
		Case inCase = uppercase;
		
	};
	
}

#endif	//ACTIVE_UTILITY_HASH
