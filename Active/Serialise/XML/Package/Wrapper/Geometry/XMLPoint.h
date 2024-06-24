/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_POINT
#define ACTIVE_SERIALISE_XML_XML_POINT

#include "Active/Serialise/Package/Package.h"
#include "Active/Utility/String.h"

namespace active::geometry {
	
	class Point;
	
}

namespace active::serialise::xml {

	/*!
		A serialisation wrapper for a Point
	 
		While explicitly supporting XML, this class should work equally well for JSON and probably CSV serialisation
	*/
	class XMLPoint : public Package, public std::reference_wrapper<geometry::Point> {
	public:
		
		// MARK: - Types
		
			//Base class type
		using base = std::reference_wrapper<geometry::Point>;
		
		// MARK: - Static variables
		
			///The element tag
		static inline utility::String tag = "point";
			
		// MARK: - Constructor

		/*!
			Constructor
			@param point The point to wrap for (de)serialisation
		*/
		XMLPoint(geometry::Point& point);
		/*!
			Constructor
			@param point The point to wrap for (de)serialisation
			@param customTag A custom tag (overrides the default tag)
		*/
		XMLPoint(geometry::Point& point, const utility::String& customTag);
		
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
		
	private:
			//Optional explicit tag (overrides default)
		utility::String::Option m_tag;
	};
	
}

#endif	//ACTIVE_SERIALISE_XML_XML_POINT
