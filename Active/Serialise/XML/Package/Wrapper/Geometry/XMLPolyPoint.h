/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_POLY_POINT
#define ACTIVE_SERIALISE_XML_XML_POLY_POINT

#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPoint.h"

namespace active::geometry {
	
	class PolyPoint;
	
}

namespace active::serialise::xml {

	/*!
		A serialisation wrapper for a PolyPoint, typically used as a Polygon vertex
	 
		While explicitly supporting XML, this class should work equally well for JSON and probably CSV serialisation
	*/
	class XMLPolyPoint : public XMLPoint {
	public:
		
		// MARK: - Types
		
		using base = XMLPoint;
		
			///The element tag
		static utility::String tag;
			
		// MARK: - Constructor

		/*!
			Constructor
			@param point The poly-point to wrap for (de)serialisation
		*/
		XMLPolyPoint(geometry::PolyPoint& point);
		/*!
			Constructor
			@param point The poly-point to wrap for (de)serialisation
			@param customTag A custom tag (overrides the default tag)
		*/
		XMLPolyPoint(geometry::PolyPoint& point, const utility::String& customTag);
		
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
		/*!
			Get the cargo as a PolyPoint
			@return The cargo as a PolyPoint (nullptr on failure)
		*/
		geometry::PolyPoint* getPolyPoint() const;
	};
	
}

#endif	//ACTIVE_SERIALISE_XML_XML_POLY_POINT
