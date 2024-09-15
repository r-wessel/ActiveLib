/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_NULL_PACKAGE
#define ACTIVE_SERIALISE_NULL_PACKAGE

#include "Active/Serialise/Package/Package.h"

namespace active::serialise {
	
	/*!
		An empty package class
	 
		Can be used (for example) in cases where a remote function may return null but some kind of object is expected
	*/
	class NullPackage : public virtual Package {
	public:

			///Unique pointer
		using Unique = std::unique_ptr<NullPackage>;
			///Shared pointer
		using Shared = std::shared_ptr<NullPackage>;
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the cargo has a null value (undefined)
			@return True if the cargo data is undefined
		*/
		bool isNull() const override { return true; }
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
		Cargo::Unique getCargo(const Inventory::Item& item) const override { return nullptr; }
	};
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_NULL_PACKAGE
