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
			Determine if the package requires attributes to be imported first (primarily for unordered serialisation, e.g. JSON)
			@return True if the package requires attributes first
		*/
		bool isAttributeFirst() const override { return get().isAttributeFirst(); }
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
		/*!
			Use a manager in (de)serialisation processes
			@param management The management to use
		*/
		void useManagement(Management* management) const override { get().useManagement(management); }
	
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
	};
	
}

#endif	//ACTIVE_SERIALISE_PACKAGE_WRAP
