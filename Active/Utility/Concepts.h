/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_CONCEPTS
#define ACTIVE_UTILITY_CONCEPTS

#include "Active/string/string_utf8.h"

#include <type_traits>
#include <memory>

namespace active {
		
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

		///string can be constructed from type
	template<typename T>
	concept StringConstructable = requires(T t, string test) {
		test = string{t};
	};

		///Object contains 3D coordinates (lower-case)
	template<typename T>
	concept IsCoordLower3D = requires(T t) {
		std::is_arithmetic_v<decltype(t.x)> && std::is_arithmetic_v<decltype(t.y)> && std::is_arithmetic_v<decltype(t.z)>;
	};

		///Object contains 3D coordinates (upper-case)
	template<typename T>
	concept IsCoordUpper3D = requires(T t) {
		std::is_arithmetic_v<decltype(t.X)> && std::is_arithmetic_v<decltype(t.Y)> && std::is_arithmetic_v<decltype(t.Z)>;
	};

		///Object contains 3D coordinates - used for interaction with 3rd-party coordinate types
	template<typename T>
	concept IsCoord3D = IsCoordLower3D<T> || IsCoordUpper3D<T>;

}  // namespace active

#endif	//ACTIVE_UTILITY_CONCEPTS
