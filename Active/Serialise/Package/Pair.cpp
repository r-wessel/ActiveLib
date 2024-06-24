/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Package/Pair.h"

#include "Active/Serialise/Item/Wrapper/ItemWrap.h"

#include <array>

using namespace active::serialise;
using namespace active::utility;

namespace active::serialise {
	
	using enum Entry::Type;
	
		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"key"},
		Identity{"value"},
	};
	
		///The default inventory item for the pair key
	Inventory::Item Pair::keyEntry = Inventory::Item{ fieldID[key], key, attribute, true, &typeid(Pair)};
		///The default inventory item for the pair value
	Inventory::Item Pair::valueEntry = Inventory::Item{ fieldID[value], value, attribute, true, &typeid(Pair)};

}  // namespace active::serialise

/*--------------------------------------------------------------------
	Constructor
	
	key: The pair key
	value: The pair value
	name: Optional name for the key/value pair
  --------------------------------------------------------------------*/
Pair::Pair(Cargo::Unique key, Cargo::Unique value, String::Option name) : Package(), m_key(std::move(key)), m_value(std::move(value)), m_name(name) {
} //Pair::Pair


/*--------------------------------------------------------------------
	Fill an inventory with the package items
	
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool Pair::fillInventory(Inventory& inventory) const {
		//If the supplied key doesn't define an inventory, use the default
	if (!m_key->fillInventory(inventory))
		inventory.push_back(keyItem());
		//If the supplied value doesn't define an inventory, use the default
	if (!m_value->fillInventory(inventory))
		inventory.push_back(valueItem());
		//Add a name for the pair when specified
	return true;
} //Pair::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique Pair::getCargo(const Inventory::Item& item) const {
		//If this object is the source of the inventory, it also needs to supply the requested cargo items
	if (item.ownerType == &typeid(Pair)) {
			//If the cargo doesn't supply an inventory, we assume it's a (single-value) item
		switch (item.index) {
			case key:
				if (auto* item = dynamic_cast<Item*>(m_key.get()); item != nullptr)
					return std::make_unique<ItemWrap>(*item);
				break;
			case value:
				if (auto* item = dynamic_cast<Item*>(m_value.get()); item != nullptr)
					return std::make_unique<ItemWrap>(*item);
				break;
		}
		return nullptr;
	}
		//Otherwise pass the request to the key and value in turn to see if either can fulfil the request
	if (m_key) {
		if (auto cargo = m_key->getCargo(item); cargo)
			return cargo;
	}
	if (m_value) {
		if (auto cargo = m_value->getCargo(item); cargo)
			return cargo;
	}
	return nullptr;
} //Pair::getCargo


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void Pair::setDefault() {
	if (m_key)
		m_key->setDefault();
	if (m_value)
		m_value->setDefault();
} //Pair::setDefault


/*--------------------------------------------------------------------
	Validate the cargo data
 
	return: True if the data has been validated
  --------------------------------------------------------------------*/
bool Pair::validate() {
	return ((!m_key || m_key->validate()) && (!m_value || m_value->validate()));
} //Pair::validate
