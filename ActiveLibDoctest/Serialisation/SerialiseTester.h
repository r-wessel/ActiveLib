/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Polygon.h"
#include "Active/Serialise/Package/Package.h"

using namespace active;
using namespace active::geometry;
using namespace active::serialise;
using namespace active::utility;

/*!
	Serialisation test class
*/
class SerialiseTester : public Package {
public:
		
	// MARK: - Static variables
	
		///The prolog tag
	inline static utility::String tag = "tester";
		
	// MARK: - Constructor

	/*!
		Constructor
		@param polygon The polygon to wrap for (de)serialisation
	*/
	SerialiseTester(const String& name = String{}, const geometry::Polygon& polygon = geometry::Polygon{}) : m_name{name}, m_shape{polygon}	{}
		
	// MARK: - Operators

	/*!
		Equality operator
		@param ref The object to compare to this
		@return True if ref is equal to this
	*/
	bool operator==(const SerialiseTester& ref) const {
		return (m_name == ref.m_name) && m_shape.isEqual2D(ref.m_shape);
	}
	
	// MARK: - Functions (const)
	
	/*!
		Fill an inventory with the package items
		@param inventory The inventory to receive the package items
		@return True if the package has added items to the inventory
	*/
	bool fillInventory(Inventory& inventory) const override;
	/*!
		Get the specified cargo
		@param item The inventory item to retrieve
		@return The requested cargo (nullptr on failure)
	*/
	Cargo::Unique getCargo(const Inventory::Item& item) const override;
	/*!
		Validate the cargo data
		@return True if the data has been validated
	*/
	bool validate() override {
		return !m_name.empty() && m_shape.isValid();
	}
	
	// MARK: - Functions (mutating)
	
	/*!
		Set to the default package content
	*/
	void setDefault() override {
		m_name.clear();
		m_shape.clear();
	}
	
private:
	String m_name;
	active::geometry::Polygon m_shape;
};
