/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_PACKAGE_WRAP
#define ACTIVE_SERIALISE_PACKAGE_WRAP

#include "Active/Serialise/Package/Package.h"

namespace active::serialise {
	
	/*!
		Lightweight interface wrapper for passing a reference to a serialisable package into (de)serialisation functions
	*/
	class PackageWrap : public Package, public std::reference_wrapper<Package> {
	public:
		
		// MARK: - Types
		
			///Item reference base
		using base = std::reference_wrapper<Package>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param source The source package to wrap
		*/
		PackageWrap(Package& source) : base(source) {}
		/*!
			Constructor
			@param source The source package to wrap
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		PackageWrap(const Package& source) : base(const_cast<Package&>(source)) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		PackageWrap(const PackageWrap& source) : base(source) {}
		/*!
			Destructor
		*/
		~PackageWrap() override = default;
			
		// MARK: - Functions (const)
		
		/*!
			Determine if the package is the outermost/root layer of a hierarchy
			@return True if the package is the outermost/root layer of a hierarchy
		*/
		bool isRoot() const { return get().isRoot(); }
		/*!
			Determine if the cargo is an item, e.g. a single/homogenous value type (not an object)
			@return True if the cargo is an item
		*/
		bool isItem() const override { return get().isItem(); }
		/*!
			Determine if the package requires attributes to be imported first (primarily for unordered serialisation, e.g. JSON)
			@return True if the package requires attributes first
		*/
		bool isAttributeFirst() const override { return get().isAttributeFirst(); }
		/*!
			Get the serialisation type for the cargo value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Type> type() const override { return get().type(); }
		/*!
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override  { return get().write(dest); }
		/*!
			Fill an inventory with the cargo items
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override	{ return get().fillInventory(inventory); }
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override { return get().getCargo(item); }
	
		// MARK: - Functions (mutating)
		
		/*!
			Clear the data content (typically a reset to defaults)
		*/
		void clear() override { get().clear(); }
		/*!
			Set to the default package content
		*/
		void setDefault() override { get().setDefault(); }
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		bool validate() override { return get().validate(); }
		/*!
			Read the cargo data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override { return get().read(source); }
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read or ignored (use false only for a genuine error - it will trigger a process failure)
		*/
		bool readSetting(const setting::Value& source) override { return get().readSetting(source); }
		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
		*/
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override { return get().insert(std::move(cargo), item); }
		/*!
			Finalise the package attributes (called when isAttributeFirst = true and attributes have been imported)
			@return True if the attributes have been successfully finalised (returning false will cause an exception to be thrown)
		*/
		bool finaliseAttributes() override { return get().finaliseAttributes(); }
		/*!
		 Allocate inventory for new (incoming) cargo
		 @param inventory The inventory to extend
		 @param identity The cargo identity
		 @return An iterator pointing to the allocated item (returns end() if inventory cannot be allocated)
		 */
		Inventory::iterator allocate(Inventory& inventory, const Identity& identity) override { return get().allocate(inventory, identity); }
		/*!
		 Allocate an existing inventory item as an array
		 @param inventory The parent inventory
		 @param item The inventory item to reallocate as an array
		 @return An iterator pointing to the reallocated item (returns end() if the item cannot be reallocated as an array)
		 */
		Inventory::iterator allocateArray(Inventory& inventory, Inventory::iterator item) override
				{ return get().allocateArray(inventory, item); }
	};
	
}

#endif	//ACTIVE_SERIALISE_PACKAGE_WRAP
