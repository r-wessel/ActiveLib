/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CARGO
#define ACTIVE_SERIALISE_CARGO

#include "Active/Serialise/Inventory/Inventory.h"

namespace active::serialise {
	
	class Manager;
	
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
			Determine if the cargo has a null value (undefined)
			@return True if the cargo data is undefined
		*/
		virtual bool isNull() const { return false; }
		/*!
			Get the recommended cargo entry type
			@return The cargo entry type (nullopt = deduce automatically from cargo characteristics)
		*/
		virtual std::optional<Entry::Type> entryType() const { return m_type; }
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
		/*!
			Use a manager in (de)serialisation processes
			@param manager The manager to use
		*/
		void useManager(Manager* manager) const { m_manager = manager; }
		/*!
			Get the cargo manager
			@return The active manager
		*/
		template<typename T>
		T* getManager() const { return dynamic_cast<T*>(m_manager); }
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the recommended cargo type
			@param type The cargo type, e.g. force an object to export as an array in JSON
			@return A reference to this
		*/
		virtual Cargo& asType(Entry::Type type) {
			m_type = type;
			return *this;
		}
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
		
	private:
			///Optional recommended cargo type - useful for some forms of transport which might incorrectly deduce it
		std::optional<Entry::Type> m_type;
			///Optional cargo manager (migration handling etc)
		mutable Manager* m_manager = nullptr;
	};

		///Transportable concept for classes/functions representing transportable cargo
	template<class T>
	concept Transportable = std::is_base_of<active::serialise::Cargo, T>::value;
	
	
}

#endif	//ACTIVE_SERIALISE_CARGO
