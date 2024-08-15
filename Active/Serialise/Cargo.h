/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CARGO
#define ACTIVE_SERIALISE_CARGO

#include "Active/Serialise/Inventory/Inventory.h"

namespace active::serialise {
	
	/*!
		Interface for data entities that support serialisation for transport
	*/
	class Cargo {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Cargo>;
			///Shared pointer
		using Shared = std::shared_ptr<Cargo>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		Cargo() {}
		/*!
			Copy constructor
			@param source The cargo to copy
		*/
		Cargo(const Cargo& source) {}
		/*!
			Destructor
		*/
		virtual ~Cargo() = default;
		
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the cargo items
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		virtual bool fillInventory(Inventory& inventory) const = 0;

		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		virtual Cargo::Unique getCargo(const Inventory::Item& item) const = 0;
		
		// MARK: - Functions (mutating)
		
		/*!
			Clear the data content (typically a reset to defaults)
		*/
		virtual void clear() { setDefault(); }
		/*!
			Set to the default package content
		*/
		virtual void setDefault() = 0;
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		virtual bool validate() { return true; }
	};

		///Transportable concept for classes/functions representing transportable cargo
	template<class T>
	concept Transportable = std::is_base_of<active::serialise::Cargo, T>::value;
	
	
}

#endif	//ACTIVE_SERIALISE_CARGO
