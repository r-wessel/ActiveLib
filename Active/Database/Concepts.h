/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_DATABASE_CONCEPTS
#define ACTIVE_DATABASE_CONCEPTS

#include <type_traits>
#include <memory>

namespace active::database {
		
		///Concept for serialisation wrapper types (objects are derived from a common base, deserialisation requires a wrapper)
	template<typename Obj, typename ObjWrapper>
	concept CanWrap = requires(Obj& obj, ObjWrapper wrap) {
		{ ObjWrapper{obj} };
		{ dynamic_cast<const Obj*>(wrap.releaseIncoming().get()) };
	};

		///Concept for flat serialisation container (all objects are the same type)
	template<typename Obj, typename ObjWrapper>
	concept FlatType = std::is_same_v<Obj, ObjWrapper>;

}  // namespace active::database

#endif	//ACTIVE_UTILITY_CONCEPTS
