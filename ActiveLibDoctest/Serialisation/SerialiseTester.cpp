/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "ActiveLibDoctest/Serialisation/SerialiseTester.h"

#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPolygon.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

using namespace active;
using namespace active::geometry;
using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::utility;

using enum Entry::Type;

namespace {
	
		///The indices of the package items
	enum FieldIndex {
		name,
		shape,
	};

		///The package inventory
	auto myInventory = Inventory{
		{
			{ {"name"}, name, attribute },
			{ {"shape"}, shape, element },
		},
	};

}  // namespace

/*!
	Fill an inventory with the package items
	@param inventory The inventory to receive the package items
	@return True if the package has added items to the inventory
*/
bool SerialiseTester::fillInventory(Inventory& inventory) const {
	inventory.merge(myInventory);
	return true;
}


/*!
	Get the specified cargo
	@param item The inventory item to retrieve
	@return The requested cargo (nullptr on failure)
*/
Cargo::Unique SerialiseTester::getCargo(const Inventory::Item& item) const {
	switch (item.index) {
		case FieldIndex::name:
			return std::make_unique<StringWrap>(m_name);
		case FieldIndex::shape:
			return std::make_unique<XMLPolygon<>>(m_shape);
		default:
			return nullptr;	//Requested an unknown index
	}
}
