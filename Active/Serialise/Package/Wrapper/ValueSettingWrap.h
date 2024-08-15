/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_VALUE_SETTING_WRAP
#define ACTIVE_SERIALISE_VALUE_SETTING_WRAP

#include "Active/Serialise/Package/Package.h"
#include "Active/Setting/ValueSetting.h"
#include "Active/Utility/String.h"

namespace active::serialise {

	/*!
		A serialisation wrapper for a setting containing a value, e.g. string, bool etc.
	*/
	class ValueSettingWrap : public Package, public std::reference_wrapper<setting::ValueSetting> {
	public:
		
			//Base class type
		using base = std::reference_wrapper<setting::ValueSetting>;
		
		// MARK: - Static variables
		
			///The default element tag
		static inline utility::String tag = "setting";
			
		// MARK: - Constructor

		/*!
			Constructor
			@param setting The setting to wrap for (de)serialisation
		*/
		ValueSettingWrap(setting::ValueSetting& setting) : base{setting} {}
		/*!
			Constructor
			@param setting The setting to wrap for (de)serialisation
		*/
		ValueSettingWrap(const setting::ValueSetting& setting) :
					//NB: const_cast only used for export, which never mutates the data
				ValueSettingWrap(const_cast<setting::ValueSetting&>(setting)) {}
		
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
			///Temporary mutable storage for incoming values
		mutable utility::String m_value;
	};

}

#endif	//ACTIVE_SERIALISE_VALUE_SETTING_WRAP
