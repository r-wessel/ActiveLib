/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CARGO
#define ACTIVE_SERIALISE_CARGO

#include "Active/Serialise/Inventory/Inventory.h"

namespace active::serialise {
	
	class Management;
	
	/*!
		Interface for data entities that support serialisation for transport
	*/
	class Cargo {
	public:
		
		// MARK: - Types
		
			//Cargo types
		enum class Type {
			text, ///<Text value
			number, ///<Numeric value
			boolean, ///<Boolean value
			package, ///<Packaged cargo, not a unary item and therefore not necessarily a specific value type
		};
			///Serialisation date/time format
		enum class TimeFormat {
			iso8601,			///< ISO 8601
			secondsSince1970,	///< Unix (posix) epoch
		};
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
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		virtual bool write(utility::String& dest) const = 0;
		/*!
			Get the serialisation type for the cargo value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		virtual std::optional<Type> type() const { return std::nullopt; }
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
			Use a specified date/time format for serialisation
			@param format The date/time format
		*/
		virtual void useTimeFormat(TimeFormat format) const {}
		/*!
			Use a manager in (de)serialisation processes
			@param management The management to use
		*/
		virtual void useManagement(Management* management) const { m_management = management; }
		/*!
			Get the cargo management
			@return The active management
		*/
		virtual Management* management() const { return m_management; }
		
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
			Read the cargo data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		virtual bool read(const utility::String& source) = 0;
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
			///Optional cargo management (migration handling etc)
		mutable Management* m_management = nullptr;
	};

		///Transportable concept for classes/functions representing transportable cargo
	template<class T>
	concept Transportable = std::is_base_of<active::serialise::Cargo, T>::value;
	
}

#endif	//ACTIVE_SERIALISE_CARGO
