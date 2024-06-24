/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_VALUE_SETTING
#define ACTIVE_SERIALISE_XML_XML_VALUE_SETTING

#include "Active/Serialise/Package/Package.h"
#include "Active/Setting/ValueSetting.h"
#include "Active/Utility/String.h"

namespace active::serialise::xml {

	/*!
		A serialisation wrapper for a setting containing a value, e.g. string, bool etc (potentially multiple values)
	*/
	class XMLValueSetting : public Package, public std::reference_wrapper<setting::ValueSetting> {
	public:
		
		// MARK: - Types
		
			///Serialisaton fields
		enum FieldIndex {
			name,
			valGuid,
			columns,
			type,
			className,
			value,
		};
		
			//Base class type
		using base = std::reference_wrapper<setting::ValueSetting>;
		
		// MARK: - Static variables
		
			///The default element tag
		static inline utility::String tag = "setting";
			
		// MARK: - Constructor

		/*!
			Constructor
			@param setting The setting to wrap for (de)serialisation
			@param isTyped True if the value is serialised as a specific type
			@param className Optional class name for the value container
		*/
		XMLValueSetting(setting::ValueSetting& setting, bool isTyped = false, std::optional<utility::String> className = std::nullopt);
		/*!
			Constructor
			@param setting The setting to wrap for (de)serialisation
			@param isTyped True if the value is serialised as a specific type
			@param className Optional class name for the value container
		*/
		XMLValueSetting(const setting::ValueSetting& setting, bool isTyped = false, std::optional<utility::String> className = std::nullopt) :
					//NB: const_cast only used for export, which never mutates the data
				XMLValueSetting(const_cast<setting::ValueSetting&>(setting), isTyped, className) {}
		/*!
			Constructor
			@param setting The setting to wrap for (de)serialisation
			@param customTag A custom tag (overrides the default tag)
			@param isTyped True if the value is serialised as a specific type
			@param className Optional class name for the value container
		*/
		XMLValueSetting(setting::ValueSetting& setting, const utility::String& customTag,
						bool isTyped = false, std::optional<utility::String> className = std::nullopt);
		/*!
			Constructor
			@param setting The setting to wrap for (de)serialisation
			@param customTag A custom tag (overrides the default tag)
			@param isTyped True if the value is serialised as a specific type
			@param className Optional class name for the value container
		*/
		XMLValueSetting(const setting::ValueSetting& setting, const utility::String& customTag,
						bool isTyped = false, std::optional<utility::String> className = std::nullopt) :
					//NB: const_cast only used for export, which never mutates the data
				XMLValueSetting(const_cast<setting::ValueSetting&>(setting), customTag, isTyped, className) {}
		
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override;
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override;
		
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
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
			@return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		*/
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override;
		
	private:
			///Optional explicit tag (overrides default)
		utility::String::Option m_tag;
			///Temporary mutable storage of the setting name
		mutable utility::String m_name;
			///Temporary mutable storage of the setting guid
		mutable utility::String m_id;
			///Storage for the value column count
		mutable int32_t m_columns = 1;
			///Temporary mutable storage for incoming values
		mutable utility::String m_value;
			///Optional value type name
		mutable utility::String m_typeName;
			///Optional value class name
		mutable utility::String m_className;
			///True if the value is serialised as a specific type
		bool m_isTyped = false;
	};

}

#endif	//ACTIVE_SERIALISE_XML_XML_VALUE_SETTING
