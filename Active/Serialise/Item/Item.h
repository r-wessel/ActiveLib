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
			//Item value types
		enum class Type {
			text,
			number,
			boolean,
		};
		
		using enum Type;
		
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the cargo items. NB: Most items rely on the enclosing package to do this, so returns false by default
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		virtual bool fillInventory(Inventory& inventory) const { return false; }
		
		/*!
			Get the specified cargo. NB: Most items rely on the enclosing package to do this, so returns nullptr by default
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		virtual Cargo::Unique getCargo(const Inventory::Item& item) const { return nullptr; }
		/*!
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		virtual bool write(utility::String& dest) const = 0;
		/*!
			Write the cargo data to a specified setting
			@param dest The setting to write to
			@return True if the data was successfully written
		*/
		virtual bool write(setting::Value& dest) const;
		/*!
			Get the serialisation type for the item value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		virtual std::optional<Type> type() const { return std::nullopt; }
		
		// MARK: - Functions (mutating)
		
		/*!
			Read the cargo data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		virtual bool read(const utility::String& source) = 0;
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		virtual bool read(const setting::Value& source);
	};
	
}

#endif	//ACTIVE_SERIALISE_ITEM
