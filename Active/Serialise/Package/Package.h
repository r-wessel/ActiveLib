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
			Determine if the package is the outermost/root layer of a hierarchy
			@return True if the package is the outermost/root layer of a hierarchy
		*/
		bool isRoot() const { return m_isRoot; }
		/*!
			Determine if the package requires attributes to be imported first (primarily for unordered serialisation, e.g. JSON)
			@return True if the package requires attributes first
		*/
		virtual bool isAttributeFirst() const { return false; }

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
		virtual bool read(const utility::String& source) { return true; }
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
		
	private:
			///True if the package is the outermost/root layer of a hierarchy
		bool m_isRoot = false;
	};
	
}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_PACKAGE
