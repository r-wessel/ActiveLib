/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_UNKNOWN
#define ACTIVE_SERIALISE_UNKNOWN

#include "Active/Serialise/Package/Package.h"

namespace active::serialise {
	
	/*!
		A package for dealing with unknown/unwanted cargo
	*/
	class Unknown : public Package {
	public:
		
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		virtual bool fillInventory(Inventory& inventory) const {
			return true;	//This package doesn't accept anything, i.e. skips all content
		}
		
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		virtual Cargo::Unique getCargo(const Inventory::Item& item) const { return nullptr; }	//This package doesn't read/write anything
	};
	
}

#endif	//ACTIVE_SERIALISE_UNKNOWN
