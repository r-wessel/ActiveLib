/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_LAYERED
#define ACTIVE_PRIMITIVE_LAYERED

#include <cstdint>

namespace active::primitive {
	
		///Interface for layered 2D graphics primitived
	struct Layered {
			///Display order (0 = bottom, higher number = higher layer)
		int32_t displayOrder = 0;
	};

}
	
#endif //ACTIVE_PRIMITIVE_LAYERED
