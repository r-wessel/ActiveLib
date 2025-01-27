/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_PACKAGE
#define ACTIVE_SERIALISE_PACKAGE

#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/Inventory/Inventory.h"

namespace active::serialise {
	
	/*!
		Interface for package objects (encapsulating multiple data items and/or packages) that support serialisation
	*/
	class Package : public virtual Cargo {
	public:

			///Unique pointer
		using Unique = std::unique_ptr<Package>;
			///Shared pointer
		using Shared = std::shared_ptr<Package>;
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the cargo is an item, e.g. a single/homogenous value type (not an object)
			@return True if the cargo is an item
		*/
		bool isItem() const override { return false; }
		/*!
			Determine if the package is the outermost/root layer of a hierarchy
			@return True if the package is the outermost/root layer of a hierarchy
		*/
		bool isRoot() const { return m_isRoot; }
		/*!
			Determine if the package requires attributes to be imported first (primarily for unordered serialisation, e.g. JSON)
			@return True if the package requires attributes first
		*/
		virtual bool isAttributeFirst() const { return false; }
		/*!
			Get the serialisation type for the cargo value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Type> type() const override { return Cargo::Type::package; }
		/*!
			Write the item data to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override  { return true; }	//Most packages do not write a data value independent of a specific field

		// MARK: - Functions (mutating)
		
		/*!
			Set to the default package content
		*/
		void setDefault() override {}
		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
			@return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		*/
		virtual bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) { return true; }
		/*!
			Import the package from a string
			@param source The string to read
			@return True if the package was successfully read
		*/
		bool read(const utility::String& source) override { return true; }	//Most packages do not read a data value independent of a specific field
		/*!
			Set whether the package is the outermost/root layer of a hierarchy
			@param state True if the package is the outermost/root layer of a hierarchy
		*/
		void setRoot(bool state) { m_isRoot = state; }
		/*!
			Finalise the package attributes (called when isAttributeFirst = true and attributes have been imported)
			@return True if the attributes have been successfully finalised (returning false will cause an exception to be thrown)
		*/
		virtual bool finaliseAttributes() { return true; }
		/*!
		 Allocate inventory for new (incoming) cargo
		 @param inventory The inventory to extend
		 @param identity The cargo identity
		 @param enclosing The enclosing cargo identity
		 @return An iterator pointing to the allocated item (returns end() if inventory cannot be allocated)
		 */
		virtual Inventory::iterator allocate(Inventory& inventory, const Identity& identity, const Identity& enclosing) { return inventory.end(); }
		/*!
		 Allocate an existing inventory item as an array
		 @param inventory The parent inventory
		 @param item The inventory item to reallocate as an array
		 @return An iterator pointing to the reallocated item (returns end() if the item cannot be reallocated as an array)
		 */
		virtual Inventory::iterator allocateArray(Inventory& inventory, Inventory::iterator item) { return inventory.end(); }
		
	private:
			///True if the package is the outermost/root layer of a hierarchy
		bool m_isRoot = false;
	};
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_PACKAGE
