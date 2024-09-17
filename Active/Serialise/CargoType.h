/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CARGO_TYPE
#define ACTIVE_SERIALISE_CARGO_TYPE

#include "Active/Serialise/Item/Item.h"
#include "Active/Serialise/Item/NullItem.h"
#include "Active/Serialise/Item/Wrapper/ValueItem.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Package/NullPackage.h"

namespace active::serialise {
	
	template<typename T>
	concept IsPackageBase = std::is_base_of_v<Package, T>;

	
	template<typename T>
	concept IsValueBase = std::is_base_of_v<ValueItem, T>;

	
	template<typename T>
	concept IsItemBase = !IsValueBase<T> && std::is_base_of_v<Item, T>;
	
	
	template<class T>
	struct CargoPicker {
	};


	template<IsPackageBase X>
	struct CargoPicker<X> {
		using CargoType = NullPackage;
	};


	template<IsItemBase X>
	struct CargoPicker<X> {
		using CargoType = NullItem;
	};


	template<IsValueBase X>
	struct CargoPicker<X> {
		using CargoType = typename X::value_t;
	};
	
}

#endif	//ACTIVE_SERIALISE_CARGO_TYPE
