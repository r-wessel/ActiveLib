/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_PACKAGE_UNO_WRAP
#define ACTIVE_SERIALISE_PACKAGE_UNO_WRAP

#include "Active/Serialise/Package/Package.h"
#include "Active/Utility/Concepts.h"

namespace active::serialise {
	
	/*!
		Lightweight interface wrapper for passing a reference to a unique_ptr of a serialisable package into (de)serialisation functions
	*/
	template<typename T> requires std::is_base_of_v<active::serialise::Package, T>
	class PackageUnoWrap : public Package, public std::reference_wrapper<std::unique_ptr<T>> {
	public:
		
		// MARK: - Types
		
			///The wrapped value type
		using value_t = std::unique_ptr<T>;
			///Item reference base
		using base = std::reference_wrapper<value_t>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param package The package to wrap
		*/
		PackageUnoWrap(value_t& package) : base(package) {}
		/*!
			Constructor
			@param package The package to wrap
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		PackageUnoWrap(const value_t& package) : base(const_cast<value_t&>(package)) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		PackageUnoWrap(const PackageUnoWrap& source) : base(source) {}
		/*!
			Destructor
		*/
		~PackageUnoWrap() override = default;
			
		// MARK: - Functions (const)
		
		/*!
			Determine if the cargo has a null value (undefined)
			@return True if the cargo data is undefined
		*/
		bool isNull() const override { return !base::get().operator bool(); }
		/*!
			Fill an inventory with the cargo items
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override	{
			if (isNull())
				return T{}.fillInventory(inventory);
			return base::get()->fillInventory(inventory);
		}
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override {
			if (isNull()) {
					//The 'available' count starts from zero for sending a cargo - if the object is null, we shouldn't be sending anything
				if (item.available == 0)
					return nullptr;	//TODO: Consider throwing an exception instead - sending a null object shouldn't reach this point
				base::get() = std::make_unique<T>();
			}
			return base::get()->getCargo(item);
		}
	
		// MARK: - Functions (mutating)
		
		/*!
			Clear the data content (typically a reset to defaults)
		*/
		void clear() override {
			if (!isNull())
				base::get()->clear();
		}
		/*!
			Set to the default package content
		*/
		void setDefault() override { base::get().reset(); }
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		bool validate() override {
			if (isNull())
				return true;	//Null is fine for an optional type
			return base::get()->validate();
		}
		/*!
			Read the cargo data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override {
			if (source.empty())
				return true;
			if (isNull())
				base::get() = std::make_unique<T>();
			return base::get()->read(source);
		}
		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
		*/
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override {
			if (isNull())
				base::get() = std::make_unique<T>();
			return base::get()->insert(std::move(cargo), item);
		}
	};
	
}

#endif	//ACTIVE_SERIALISE_PACKAGE_UNO_WRAP
