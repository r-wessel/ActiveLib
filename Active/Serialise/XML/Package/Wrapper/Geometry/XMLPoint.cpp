/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPoint.h"

#include "Active/Geometry/Point.h"
#include "Active/Serialise/XML/Item/XMLLength.h"

#include <array>

using namespace active::geometry;
using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::utility;

using enum Identity::Role;

namespace {
	
		///The indices of the package items
	enum FieldIndex {
		x,
		y,
		z,
	};

		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"x"},
		Identity{"y"},
		Identity{"z"},
	};
	
		///The package inventory
	auto myInventory = Inventory{
		{
			{ fieldID[x], x, attribute },
			{ fieldID[y], y, attribute },
			{ fieldID[z], z, attribute },
		}
	}.withType(&typeid(XMLPoint));

}  // namespace

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor
 
	point: The point to wrap for (de)serialisation
  --------------------------------------------------------------------*/
XMLPoint::XMLPoint(Point& point) : base(point) {}


/*--------------------------------------------------------------------
	Constructor
 
	point: The point to wrap for (de)serialisation
	customTag: A custom tag (overrides the default tag)
  --------------------------------------------------------------------*/
XMLPoint::XMLPoint(Point& point, const utility::String& customTag) : base(point), m_tag(customTag) {}

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool XMLPoint::fillInventory(Inventory& inventory) const {
	inventory.merge(myInventory);
		//Only export non-zero z coords (minimises storage for 2D points)
	if (math::isZero(base::get().z))
		inventory.setRequired(false, FieldIndex::z, &typeid(XMLPoint));
	return true;
} //XMLPoint::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique XMLPoint::getCargo(const Inventory::Item& item) const {
	if (item.ownerType != &typeid(XMLPoint))
		return nullptr;
	switch (item.index) {
		case FieldIndex::x:
			return std::make_unique<XMLLength>(base::get().x);
		case FieldIndex::y:
			return std::make_unique<XMLLength>(base::get().y);
		case FieldIndex::z:
			return std::make_unique<XMLLength>(base::get().z);
		default:
			return nullptr;	//Requested an unknown index
	}
} //XMLPoint::getCargo

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void XMLPoint::setDefault() {
	base::get().x = base::get().y = base::get().z = 0.0;
} //XMLPoint::setDefault
