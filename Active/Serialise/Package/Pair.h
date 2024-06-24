/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_PAIR
#define ACTIVE_SERIALISE_PAIR

#include "Active/Serialise/Package/Package.h"

namespace active::serialise {
	
	/*!
		A serialisable pair, e.g. key and value
	 
		Use this class as a simple wrapper for serialisable data that should be stored in pairs, e.g. dictionary items.
		The wrapper acts as an enclosing package for the key and value. Both the key and value can define an inventory or inventory items,
		but this wrapper will provide defaults if these are undefined.
	*/
	class Pair : public Package {
	public:
		
		// MARK: - Types
		
			///The indices of the default package items
		enum FieldIndex {
			key,
			value,
		};

		// MARK: - Constants
		
			///The default inventory item for the pair key
		static Inventory::Item keyEntry;
			///The default inventory item for the pair value
		static Inventory::Item valueEntry;

		/*!
			Constructor
			@param key The pair key
		 	@param value The pair value
			@param name Optional name for the key/value pair
		*/
		Pair(Cargo::Unique key, Cargo::Unique value, utility::String::Option name = std::nullopt);
			///No copy constructor
		Pair(const Pair& source) = delete;
			///Destructor
		virtual ~Pair() = default;
		
		// MARK: - Functions (const)
		
		/*!
			Get the pair key
			@return The pair key (this package retains ownership)
		*/
		Cargo* getKey() const { return m_key.get(); }
		/*!
			Get the pair value
			@return The pair value (this package retains ownership)
		*/
		Cargo* getValue() const { return m_value.get(); }
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		virtual bool fillInventory(Inventory& inventory) const;
		/*!
			Get the default inventory item for the key
			@return The default key item
		*/
		virtual const Inventory::Item& keyItem() const { return keyEntry; }
		/*!
			Get the default inventory item for the value
			@return The default value item
		*/
		virtual const Inventory::Item& valueItem() const { return valueEntry; }
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		virtual Cargo::Unique getCargo(const Inventory::Item& item) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Set to the default package content
		*/
		virtual void setDefault();
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		virtual bool validate();
		/*!
			Release the pair key
			@return The pair key (caller takes ownership - the key held by this package is reset to nullptr)
		*/
		auto releaseKey() { return std::move(m_key); }
		/*!
			Release the pair value
			@return The pair value (caller takes ownership - the value held by this package is reset to nullptr)
		*/
		auto releaseData() { return std::move(m_value); }
		
	private:
			///The pair key
		Cargo::Unique m_key;
			///The pair value
		Cargo::Unique m_value;
			///Optional pair name
		utility::String::Option m_name;
	};
	
}

#endif	//ACTIVE_SERIALISE_PAIR
