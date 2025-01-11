/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Package/Wrapper/ValueSettingWrap.h"

#include "Active/Serialise/Item/Wrapper/AnyValueWrap.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

#include <array>

using namespace active::serialise;
using namespace active::setting;
using namespace active::utility;

namespace {
	
	const inline int16_t valueIndex = 0;
	
}

/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool ValueSettingWrap::fillInventory(Inventory& inventory) const {
	using enum Identity::Role;
	inventory.merge(Inventory{
		{
			{ String{"value"}, valueIndex, static_cast<uint32_t>(get().size()), std::nullopt, (get().size() > 0), &typeid(ValueSettingWrap) },
		}
	});
	return true;
} //ValueSettingWrap::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique ValueSettingWrap::getCargo(const Inventory::Item& item) const {
	if (item.ownerType != &typeid(ValueSettingWrap))
		return nullptr;
	switch (item.index) {
		case 0:
			if (item.available < static_cast<uint32_t>(get().size()))
				return std::make_unique<AnyValueWrap>(*(get()[item.available]));
			return std::make_unique<AnyValueWrap>();
		default:
			return nullptr;	//Requested an unknown index
	}
} //ValueSettingWrap::getCargo


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void ValueSettingWrap::setDefault() {
	base::get().clear();	//The default state is an empty setting
} //ValueSettingWrap::setDefault


/*--------------------------------------------------------------------
	Validate the cargo data
 
	return: True if the data has been validated
  --------------------------------------------------------------------*/
bool ValueSettingWrap::validate() {
	return true;
} //ValueSettingWrap::validate


/*--------------------------------------------------------------------
	Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
 
	cargo: The cargo to insert
	item: The inventory item linked with the cargo
 
	return: True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
  --------------------------------------------------------------------*/
bool ValueSettingWrap::insert(Cargo::Unique&& cargo, const Inventory::Item& item) {
	if (item.ownerType != &typeid(ValueSettingWrap))
		return true;
	switch (item.index) {
		case valueIndex:
			if (auto* anyValue = dynamic_cast<AnyValueWrap*>(cargo.get()); anyValue != nullptr) {
				if (auto newValue = anyValue->releaseIncoming(); newValue)
					base::get().emplace_back(newValue);
			}
			break;
		default:
			break;
	}
	return true;
} //ValueSettingWrap::insert
