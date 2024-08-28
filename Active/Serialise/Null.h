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
			Set to the default package content
		*/
		void setDefault() override {}
	};
	
}

#endif	//ACTIVE_SERIALISE_NULL
