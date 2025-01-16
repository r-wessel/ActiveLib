/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_ITEM
#define ACTIVE_SERIALISE_ITEM

#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/Inventory/Inventory.h"

namespace active::setting {
	
	class Value;
	
}

namespace active::serialise {
	
	/*!
		Interface for unary data items (representing a single value like String, double etc) enabling serialise for transport
	*/
	class Item : public virtual Cargo {
	public:
		
		using enum Cargo::Type;
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the cargo is an item, e.g. a single/homogenous value type (not an object)
			@return True if the cargo is an item
		*/
		bool isItem() const override { return true; }
		/*!
			Fill an inventory with the cargo items. NB: Most items rely on the enclosing package to do this, so returns false by default
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override { return false; }
		
		/*!
			Get the specified cargo. NB: Most items rely on the enclosing package to do this, so returns nullptr by default
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override { return nullptr; }
		/*!
			Write the cargo data to a specified setting
			@param dest The setting to write to
			@return True if the data was successfully written
		*/
		bool writeSetting(setting::Value& dest) const override;
		
		// MARK: - Functions (mutating)
		
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		bool readSetting(const setting::Value& source) override;
	};
	
}

#endif	//ACTIVE_SERIALISE_ITEM
