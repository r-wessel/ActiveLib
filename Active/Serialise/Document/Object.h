/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_DOC_OBJECT
#define ACTIVE_SERIALISE_DOC_OBJECT

#include "Active/Serialise/Document/Handler.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Setting/ValueSetting.h"

namespace active::serialise::doc {
	
	class Object;
	
	/*!
		A class to represent an object in a persisted document model (via serialisation)
		
		This class forms the basis for a simple document storage model, e.g. a single Object can represent an entire document. Supporting
		transformations to/from doc::Object provides an easy way to (de)serialise domain-specific content without
		writing any code directly related to serialisation. This simple document model can be sent to/retrieved from JSON or XML
	 
		Refer to the Handler class for more information about defining domain-specific classes that can be deserialised from JSON/XML
	*/
	class Object : public serialise::Package {
	public:

		// MARK: - Constants

			///The Object tag
		static inline utility::String defaultTag = "obj";

		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Object>;
			///Shared pointer
		using Shared = std::shared_ptr<Object>;
			///Optional
		using Option = std::optional<Object>;
			///Base container - supports an document object hierarchy
		using base = std::vector<Object>;
			///Values container - holds all single-value variables from the original class
		using Values = std::vector<setting::ValueSetting>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		Object() {}
		/*!
			Constructor
			@param type The document object type - used to reconstruct the original object. NB: Can use type_info.name, but isn't consistent
			@param objTag An optional serialisation tag (differentiating the object role when there are many of the same type for different purposes)
		*/
		Object(const utility::String& type,
			   utility::String::Option objTag = std::nullopt);
			///Use the default copy
		Object(const Object& source) = default;
			///Use the default rvalue copy
		Object(Object&& source) noexcept = default;
		
		// MARK: - Public variables
		
			///The document object type - can be used to reconstruct the original object
		utility::String docType;
			///A serialisation tag (identifying the object role)
		utility::String tag;
			///Child objects of this object
		std::vector<Object> objects;
			///Child values of this object, i.e. single-value variables in the original object
		Values values;
		
		// MARK: Operators
		
			///Use default assignment
		Object& operator=(Object&& source) = default;
		/*!
			Write operator
			@param object A object to move to the object
			@return A reference to this
		*/
		Object& operator<<(Object& object) {
			objects.emplace_back(std::move(object));
			return *this;
		}
		/*!
			Write operator
			@param object A object to move to the object
			@return A reference to this
		*/
		Object& operator<<(Object&& object) {
			objects.emplace_back(std::move(object));
			return *this;
		}
		/*!
			Write operator
			@param value A value to move to the object
			@return A reference to this
		*/
		Object& operator<<(setting::ValueSetting&& value) {
			values.emplace_back(std::move(value));
			return *this;
		}
		
		// MARK: - Functions (const)
		
		/*!
			Get an object by tag
			@param tag The required object tag
			@return The requested object (nullptr if not found)
		*/
		const Object* object(const utility::String& tag) const;
		/*!
			Make an object from this object
			@param tag The required object tag (empty to retrieve this object as an object
			@return An equivalent object (nullptr on failure)
		*/
		template<typename T>
		std::unique_ptr<T> object(const utility::String& tag = utility::String{}) const {
			if (!m_handler)
				return nullptr;
			if (!tag.empty()) {
				if (const auto* source = object(tag); source != nullptr) {
					return source->object<T>();
				}
				return nullptr;
			}
			return std::unique_ptr<T>{m_handler->reconstruct<T>(*this)};
		}
		/*!
			Get an object value by name
			@param name The value name
			@return The requested value (nullopt on failure)
		*/
		const setting::ValueSetting::Option value(const utility::String& name) const;
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		bool fillInventory(serialise::Inventory& inventory) const override;
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		serialise::Cargo::Unique getCargo(const serialise::Inventory::Item& item) const override;
		/*!
			Define the handler to reconstruct objects
			@param handler The handler to reconstruct objects
			@return A reference to this
		*/
		const Object& usingHandler(std::shared_ptr<Handler> handler) const;
		
		// MARK: - Functions (mutating)
		
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
			Insert specified cargo into the reconstruct objects - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
			@return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		*/
		bool insert(Cargo::Unique&& cargo, const serialise::Inventory::Item& item) override;
		/*!
			Set the object type
			@param newType The document object type
			@return A reference to this
		*/
		Object& withType(const utility::String& newType) {
			docType = newType;
			return *this;
		}
		/*!
			Set the object tag
			@param newTag The object tag
			@return A reference to this
		*/
		Object& withTag(const utility::String& newTag) {
			tag = newTag;
			return *this;
		}
		
	private:
		mutable std::shared_ptr<Handler> m_handler;
		mutable setting::ValueSetting m_incoming;
	};

}  // namespace active::serialise::doc

#endif	//ACTIVE_SERIALISE_DOC_OBJECT
