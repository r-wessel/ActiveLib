/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_ITEM_WRAP
#define ACTIVE_SERIALISE_ITEM_WRAP

#include "Active/Serialise/Item/Item.h"

namespace active::serialise {
	
	/*!
		Lightweight interface wrapper for passing a reference to a serialisable item into (de)serialisation functions
	*/
	class ItemWrap : public Item, public std::reference_wrapper<Item> {
	public:
		
		// MARK: - Types
		
			///Item reference base
		using base = std::reference_wrapper<Item>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param source The source package to wrap
		*/
		ItemWrap(Item& source) : base(source) {}
		/*!
			Constructor
			@param source The source package to wrap
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		ItemWrap(const Item& source) : base(const_cast<Item&>(source)) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		ItemWrap(const ItemWrap& source) : base(source.get()) {}
		/*!
			Destructor
		*/
		~ItemWrap() override = default;
			
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the cargo items. NB: Most items rely on the enclosing package to do this, so returns false by default
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override	{ return base::get().fillInventory(inventory); }
		
		/*!
			Get the specified cargo. NB: Most items rely on the enclosing package to do this, so returns nullptr by default
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override	{ return base::get().getCargo(item); }
		/*!
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override { return base::get().write(dest); }
		/*!
			Write the cargo data to a specified setting
			@param dest The setting to write to
			@return True if the data was successfully written
		*/
		bool writeSetting(setting::Value& dest) const override { return base::get().writeSetting(dest); }

		// MARK: - Functions (mutating)
		
		/*!
			Clear the data content (typically a reset to defaults)
		*/
		void clear() override { base::get().clear(); }
		/*!
			Set to the default package content
		*/
		void setDefault() override { base::get().setDefault(); }
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		bool validate() override { return base::get().validate(); }
		/*!
			Read the cargo data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override { return base::get().read(source); }
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		bool readSetting(const setting::Value& source) override { return base::get().readSetting(source); }
	};
	
}

#endif	//ACTIVE_SERIALISE_ITEM_WRAP
