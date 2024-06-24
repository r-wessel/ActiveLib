/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_PROLOG
#define ACTIVE_SERIALISE_XML_XML_PROLOG

#include "Active/Serialise/Package/Package.h"
#include "Active/Utility/String.h"

namespace active::serialise::xml {

	/*!
		XML prolog class
	 
		Used at the head of XML content to establish content standards, e.g. version and encoding
	*/
	class XMLProlog : public Package {
	public:
			
		// MARK: - Static variables
		
			///The prolog tag
		inline static utility::String tag = "xml";
			
		// MARK: - Public variables
		
			///XML version
		utility::String version = "1.0";
			///Text encoding type
		utility::TextEncoding encoding = utility::TextEncoding::UTF8;
		
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
	};
	
}  // namespace active::serialise::xml

#endif	//ACTIVE_SERIALISE_XML_XML_PROLOG
