/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_CONCEPTS
#define ACTIVE_UTILITY_CONCEPTS

#include "Active/Utility/String.h"

#include <type_traits>
#include <memory>

namespace active::utility {
		
		///Type has a default constructor
	template<typename T>
	concept DefaultConstructable = requires(const T& t) {
		std::is_default_constructible_v<T>;
	};

		///Type can be dereferenced - suggests pointer behaviour, e.g. unique_ptr, shared_ptr
	template<typename T>
	concept Dereferenceable = requires(const T& t) {
		*t;
	};

		///String can be constructed from type
	template<typename T>
	concept StringConstructable = requires(T t, String test) {
		test = String{t};
	};

}  // namespace active::utility

#endif	//ACTIVE_UTILITY_CONCEPTS
