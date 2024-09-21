/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_MOVER
#define ACTIVE_SERIALISE_MOVER

#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Package/Wrapper/Handler.h"
#include "Active/Serialise/Package/Wrapper/PackageUniqueWrap.h"

namespace active::serialise {
	
	/*!
	 Wrapper to box/unbox objects during (de)serialisation, including reading/writing a specified attribute to determine object type
	 
	 This wrapper is intended to be used for serialisation of polymorphic objects, i.e. any number of objects derived from a common base. There
	 are 4 different scenarios for using the wrapper:
	 1) Where it is known that an object is to be serialised (only), the wrapper is constructed with a reference to the target package
	 2) Where an object is held as a unique pointer and might be serialised or deserialised, the wrapper is constructed  with a reference to
	 the unique pointer member wrapper. On deserialisation, the wrapper will determine the correct type (using the supplied handler) and popuate the
	 unique pointer accordingly
	 3) As above, but the object can only be one type. in this case, the handler is nullptr and an instance of the object can be made (if necessary)
	 by the wrapper
	 4) Where a list of polymorphic objects is deserialised, the wrapper is constructed with no package references. It will create a new instance
	 based on the deserialised data and release it to the list for insertion.
	 The wrapper should be populated by a package handler that manages the correct object type and type tagging during (de)serialisation.
	 Note that a subclass of this wrapper can instantiate and populate the handler, allowing the wrapper to be created via a default constructor that is
	 automatically bound to a set of internally defined object types
	 */
	class Mover : public active::serialise::Package {
	public:

		// MARK: - Constructors
		
		/*!
		 Constructor (when it will be used for serialisation only on an existing object)
		 @param package A reference to the member variable
		 @param handler A package handler to tag outgoing packages
		 */
		Mover(const Package& package, Handler::Shared handler);
		/*!
		 Constructor (when a reference to a unique ptr member package variable is supplied)
		 @param package A reference to a unique ptr member variable
		 @param handler A package handler to tag outgoing packages (nullptr = fixed type, use the wrapper object maker)
		 */
		Mover(PackageUniqueWrap&& package, Handler::Shared handler = nullptr);
		/*!
		 Constructor (primarily for a list of polymorphic objects)
		 @param handler A package handler to reconstruct incoming packages
		 */
		Mover(Handler::Shared handler);

		// MARK: - Functions (const)
		
		/*!
		 Determine if the cargo has a null value (undefined)
		 @return True if the cargo data is undefined
		*/
		bool isNull() const override { return m_package == nullptr; }
			///The attributes for a serialised `Foo` identify its type and guid - these are tagged as attributes and must be deserialised first
		bool isAttributeFirst() const override { return m_isReadingAttributes.value_or(false); }
		/*!
		 Fill an inventory with the package items
		 @param inventory The inventory to receive the package items
		 @return True if the package has added items to the inventory
		 */
		bool fillInventory(active::serialise::Inventory& inventory) const override;
		/*!
		 Get the specified cargo
		 @param item The inventory item to retrieve
		 @return The requested cargo (nullptr on failure)
		 */
		Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override;
		/*!
		 Get the deserialised package
		 @return The deserialised package (nullptr on failure)
		 */
		Package* getIncoming() const {
			return m_wrapper.get();
		}
		/*!
		 Release the deserialised package
		 @return The deserialised package (nullptr on failure)
		 */
		Package::Unique releaseIncoming() const {
			return std::move(m_wrapper);
		}

		// MARK: - Functions (mutating)
		
		/*!
			Clear the data content (typically a reset to defaults)
		*/
		void clear() override { if (!isNull()) m_package->clear(); }
		/*!
		 Set to the default package content
		 */
		void setDefault() override;
		/*!
		 Validate the cargo data
		 @return True if the data has been validated
		 */
		bool validate() override;
		/*!
			Finalise the package attributes (called when isAttributeFirst = true and attributes have been imported)
			@return True if the attributes have been successfully finalised (returning false will cause an exception to be thrown)
		*/
		bool finaliseAttributes() override;
		/*!
		 Read the cargo data from the specified string
		 @param source The string to read
		 @return True if the data was successfully read
		 */
		bool read(const utility::String& source) override;
		/*!
		 Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
		 @param cargo The cargo to insert
		 @param item The inventory item linked with the cargo
		 */
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override;
		
	private:
			///Handler for package identification and unboxing
		Handler::Shared m_handler;
			///The type name extracted from an incoming attribute (used during deserialisation)
		utility::String m_typeName;
			///Bugger for creating a new object during deserialisation
		mutable Package::Unique m_wrapper;
			///Pointer to to the target object (can be to an external object or one of the internal members above)
		mutable Package* m_package = nullptr;
			///Unused for serialisation - true when deserialising the type and guid attributes
		std::optional<bool> m_isReadingAttributes;
		
		std::optional<PackageUniqueWrap> m_unique;
	};

}
	
#endif //ACTIVE_SERIALISE_MOVER
