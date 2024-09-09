/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_CONCEPTS
#define ACTIVE_UTILITY_CONCEPTS

#include <type_traits>
#include <memory>

namespace active::utility {
		
		///Type can be dereferenced - suggests pointer behaviour, e.g. unique_ptr, shared_ptr
	template<typename T>
	concept Dereferenceable = requires(const T& t) {
		*t;
	};

}  // namespace active::utility

#endif	//ACTIVE_UTILITY_CONCEPTS
