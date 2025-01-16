/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_NULL
#define ACTIVE_SERIALISE_NULL

#include "Active/Serialise/Cargo.h"

namespace active::serialise {
	
	/*!
		Interface for null cargo, i.e. an item/package with no defined content (equates to 'null' in JSON)
	*/
	class Null : public Cargo {
	public:

		// MARK: - Functions (const)
		
		/*!
			Determine if the cargo has a null value (undefined)
			@return True if the cargo data is undefined
		*/
		bool isNull() const override { return true; }
		/*!
			Determine if the cargo is an item, e.g. a single/homogenous value type (not an object)
			@return True if the cargo is an item
		*/
		bool isItem() const override { return true; }
		/*!
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override { return true; }

		/*!
			Fill an inventory with the cargo items
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override { return true; }
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Null::Unique getCargo(const Inventory::Item& item) const override { return nullptr; }

		// MARK: - Functions (mutating)

		/*!
			Read the cargo data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override { return true; }
		/*!
			Set to the default package content
		*/
		void setDefault() override {}
	};
	
}

#endif	//ACTIVE_SERIALISE_NULL
