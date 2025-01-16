/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/XML/Package/XMLProlog.h"

#include "Active/Serialise/Item/Encoding.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

#include <array>

using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::utility;

using enum Identity::Role;

namespace {
	
		///The indices of the package items
	enum FieldIndex {
		version,
		encoding,
	};

		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"version"},
		Identity{"encoding"},
	};
	
		///The package inventory
	auto myInventory = Inventory{
		{
			{ fieldID[version], version, attribute },
			{ fieldID[encoding], encoding, attribute },
		},
	};

}  // namespace

/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void XMLProlog::setDefault() {
	version = "1.0";
	encoding = TextEncoding::UTF8;
} //XMLProlog::setDefault


/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool XMLProlog::fillInventory(Inventory& inventory) const {
	inventory.merge(myInventory);
	return true;
} //XMLProlog::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique XMLProlog::getCargo(const Inventory::Item& item) const {
	switch (item.index) {
		case FieldIndex::version:
			return std::make_unique<StringWrap>(version);
		case FieldIndex::encoding:
			return std::make_unique<ValueWrap<TextEncoding>>(encoding);
		default:
			return nullptr;	//Requested an unknown index
	}
} //XMLProlog::getCargo
