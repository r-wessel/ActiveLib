/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPolyPoint.h"

#include "Active/Geometry/PolyPoint.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

#include <array>

using namespace active::geometry;
using namespace active::math;
using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::utility;

using enum Entry::Type;

namespace {
	
		///The indices of the package items
	enum FieldIndex {
		sweep,
		id,
	};

		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"sweep"},
		Identity{"id"},
	};
	
		///The package inventory
	auto myInventory = Inventory{
		{
			{ fieldID[sweep], sweep, attribute },
			{ fieldID[id], id, attribute },
		}
	}.withType(&typeid(XMLPolyPoint));

}  // namespace

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor
 
	point: The poly-point to wrap for (de)serialisation
  --------------------------------------------------------------------*/
XMLPolyPoint::XMLPolyPoint(geometry::PolyPoint& point) : XMLPoint(point) {}


/*--------------------------------------------------------------------
	Constructor
 
	point: The poly-point to wrap for (de)serialisation
	customTag: A custom tag (overrides the default tag)
  --------------------------------------------------------------------*/
XMLPolyPoint::XMLPolyPoint(geometry::PolyPoint& point, const utility::String& customTag) : XMLPoint(point, customTag) {}

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool XMLPolyPoint::fillInventory(Inventory& inventory) const {
	inventory.merge(myInventory);
	base::fillInventory(inventory);
	if (auto* polyPoint = getPolyPoint(); polyPoint != nullptr) {
			//Only export non-zero sweep angle (minimises storage for straight edges)
		if (isZero(polyPoint->sweep))
			inventory.setRequired(false, FieldIndex::sweep, &typeid(XMLPolyPoint));
			//Only export non-zero vertex IDs (minimise storage for anonymous vertices)
		if (polyPoint->id != 0)
			inventory.setRequired(false, FieldIndex::id, &typeid(XMLPolyPoint));
	}
	return true;
} //XMLPolyPoint::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique XMLPolyPoint::getCargo(const Inventory::Item& item) const {
	if (item.ownerType != &typeid(XMLPolyPoint))
		return base::getCargo(item);
	if (auto* polyPoint = getPolyPoint(); polyPoint != nullptr) {
		switch (item.index) {
			case FieldIndex::sweep:
				return std::make_unique<ValueWrap<double>>(polyPoint->sweep);
			case FieldIndex::id:
				return std::make_unique<ValueWrap<vertex_id>>(polyPoint->id);
			default:
				break;	//Requested an unknown index
		}
	}
	return nullptr;
} //XMLPolyPoint::getCargo


/*--------------------------------------------------------------------
	Get the cargo as a PolyPoint
 
	return: The cargo as a PolyPoint (nullptr on failure)
  --------------------------------------------------------------------*/
PolyPoint* XMLPolyPoint::getPolyPoint() const {
	return dynamic_cast<PolyPoint*>(&base::get());
}

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void XMLPolyPoint::setDefault() {
	base::setDefault();
	if (auto* polyPoint = getPolyPoint(); polyPoint != nullptr) {
		polyPoint->sweep = 0.0;
		polyPoint->id = 0;
	}
} //XMLPolyPoint::setDefault
