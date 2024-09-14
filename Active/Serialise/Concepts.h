/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CONCEPTS
#define ACTIVE_SERIALISE_CONCEPTS

#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/Package/Package.h"

#include <type_traits>
#include <memory>

namespace active::serialise {
		
		///Concept for tranportable objects
	template<typename Obj>
	concept IsCargo = std::is_base_of_v<serialise::Cargo, Obj>;

			///Concept for tranportable package objects
	template<typename Obj>
	concept IsPackageCargo = std::is_base_of_v<Package, Obj>;


			///Concept for tranportable item objects
	template<typename Obj>
	concept IsItemCargo = std::is_base_of_v<Item, Obj>;
	
		///Concept for wrappable values, e.g. double, int32_t, String
	template<typename Obj>
	concept IsWrappableValue = requires(Obj obj) {
		ValueWrap<Obj>(obj).read(active::utility::String{});
	};

}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_CONCEPTS
