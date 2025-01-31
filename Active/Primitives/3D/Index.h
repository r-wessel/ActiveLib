/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_PRIMITIVE_INDEX
#define ACTIVE_PRIMITIVE_INDEX

#include <limits>
#include <stdint.h>

namespace active::primitive {
	
			///3D primitive index type
		using Index = uint32_t;

				///Value indicating an undefined or invalid index
		inline constexpr Index npos = std::numeric_limits<Index>::max();

}

#endif //ACTIVE_PRIMITIVE_INDEX
