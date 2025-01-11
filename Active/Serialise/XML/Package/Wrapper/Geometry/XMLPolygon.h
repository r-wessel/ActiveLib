/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_XML_XML_POLYGON
#define ACTIVE_SERIALISE_XML_XML_POLYGON

#include "Active/Geometry/Polygon.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPolyPoint.h"
#include "Active/Utility/String.h"

#include <type_traits>

namespace active::serialise::xml {

		///Concept for classes derived from PolyPoint
	template<class Vert>
	concept IsPolyPoint = std::is_base_of<geometry::PolyPoint, Vert>::value;
		///Concept for classes derived from XMLPolyPoint
	template<class VertWrap>
	concept IsPointWrap = std::is_base_of<XMLPolyPoint, VertWrap>::value;

	/*!
		A serialisation wrapper for a solid Polygon (no holes)
	 
		While explicitly supporting XML, this class should work equally well for JSON and probably CSV serialisation
		This is a template to support the deserialisation of vertex objects derived from PolyPoint
	*/
	template<class Vert = geometry::PolyPoint, class VertWrap = XMLPolyPoint> requires IsPolyPoint<Vert> && IsPointWrap<VertWrap>
	class XMLSolidPolygon : public Package, public std::reference_wrapper<geometry::Polygon> {
	public:
		
		// MARK: - Types
		
			//Base class type
		using base = std::reference_wrapper<geometry::Polygon>;
		
			///Serialisation fields
		enum FieldIndex {
			vertex,
			topID,
		};
		
		// MARK: - Static variables
		
			///The default element tag
		static inline utility::String tag = "polygon";
		
			///Serialisation field IDs
		static inline std::array fieldID = {
			Identity{"vertex"},
			Identity{"topID"},
		};
			
		// MARK: - Constructor

		/*!
			Constructor
			@param polygon The polygon to wrap for (de)serialisation
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		XMLSolidPolygon(const geometry::Polygon& polygon) : base(const_cast<geometry::Polygon&>(polygon)) { m_topID = polygon.getTopID(); }
		/*!
			Constructor
			@param polygon The polygon to wrap for (de)serialisation
			@param customTag A custom tag (overrides the default tag)
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		XMLSolidPolygon(const geometry::Polygon& polygon, const utility::String& customTag) : base(const_cast<geometry::Polygon&>(polygon)), m_tag(customTag) {
			m_topID = polygon.getTopID();
		}
		
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override {
			using enum Identity::Role;
			inventory.merge(Inventory{
				{
					{ fieldID[topID], topID, attribute, (m_topID != 0) },
					{ fieldID[vertex], vertex, static_cast<uint32_t>(get().size()), std::nullopt, (get().size() > 0) },
				}
			}.withType(&typeid(XMLSolidPolygon<Vert, VertWrap>)));
			return true;
		}
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override {
			if (item.ownerType != &typeid(XMLSolidPolygon<Vert>))
				return nullptr;
			switch (item.index) {
				case FieldIndex::vertex:
					return std::make_unique<VertWrap>((item.available < static_cast<uint32_t>(get().vertSize())) ?
													  *(get()[static_cast<geometry::vertex_index>(item.available)]) : m_vertex);
				case FieldIndex::topID:
					return std::make_unique<ValueWrap<geometry::vertex_id>>(m_topID);
				default:
					return nullptr;	//Requested an unknown index
			}
		}
		
		// MARK: - Functions (mutating)
		
		/*!
			Set to the default package content
		*/
		void setDefault() override {
			base::get().clear();	//The default state is an empty polygon
		}
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		bool validate() override {
			base::get().setTopID(m_topID);
			return true;
		}
		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
			@return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		*/
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override {
			if (item.ownerType != &typeid(XMLSolidPolygon<Vert, VertWrap>))
				return true;
			switch (item.index) {
				case FieldIndex::vertex:
					base::get().emplace_back(m_vertex);
					break;
				default:
					break;
			}
			return true;
		}
		
	private:
			///Optional explicit tag (overrides default)
		utility::String::Option m_tag;
			///Temporary mutable storage for incoming vertices
		mutable Vert m_vertex;
			///Temporary mutable storage of the polygon top ID for i/o
		mutable geometry::vertex_id m_topID = 0;
	};
	
	
		///Concept for classes derived from Polygon
	template<class Hole>
	concept IsPolygon = std::is_base_of<geometry::Polygon, Hole>::value;
		///Concept for classes derived from XMLSolidPolygon
	template<class HoleWrap>
	concept IsHoleWrap = std::is_base_of<XMLSolidPolygon<>, HoleWrap>::value;

	
	/*!
		A serialisation wrapper for a Polygon (includng holes)
	 
		While explicitly supporting XML, this class should work equally well for JSON and probably CSV serialisation
		This is a template to support the deserialisation of vertex objects derived from PolyPoint
	*/
	template<class Vert = geometry::PolyPoint, class VertWrap = XMLPolyPoint,
			class Hole = geometry::Polygon, class HoleWrap = XMLSolidPolygon<Vert, VertWrap>>
			requires IsPolyPoint<Vert> && IsPointWrap<VertWrap> && IsPolygon<Hole> && IsHoleWrap<HoleWrap>
	class XMLPolygon : public XMLSolidPolygon<Vert, VertWrap> {
	public:
		
		// MARK: - Types
		
			//Base class type
		using base = XMLSolidPolygon<Vert, VertWrap>;
		
			///The package fields
		enum FieldIndex {
			hole,
		};
		
		// MARK: - Static variables
		
			///The default element tag
		static inline utility::String tag = "polygon";
		
			///Serialisation field IDs
		static inline std::array fieldID = {
			Identity{"hole"},
		};
			
		// MARK: - Constructor

		/*!
			Constructor
			@param polygon The polygon to wrap for (de)serialisation
		*/
		XMLPolygon(const geometry::Polygon& polygon) : base{polygon}	{}
		/*!
			Constructor
			@param polygon The polygon to wrap for (de)serialisation
			@param customTag A custom tag (overrides the default tag)
		*/
		XMLPolygon(const geometry::Polygon& polygon, const utility::String& customTag) : base{polygon, customTag}	{}
		
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		
		bool fillInventory(Inventory& inventory) const override {
			base::fillInventory(inventory);
			inventory.merge(Inventory{
				{
					{ fieldID[hole], hole, static_cast<uint32_t>(base::get().getHoleSize()), std::nullopt, (base::get().getHoleSize() > 0) },
				}
			}.withType(&typeid(XMLPolygon<Vert, VertWrap, Hole, HoleWrap>)));
			return true;
		}
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override {
			if (item.ownerType != &typeid(XMLPolygon<Vert>))
				return base::getCargo(item);
			switch (item.index) {
				case FieldIndex::hole:
					return std::make_unique<HoleWrap>((item.available < static_cast<uint32_t>(base::get().getHoleSize())) ?
													  *(base::get().getHole(static_cast<geometry::part_index>(item.available))) : m_hole);
				default:
					return nullptr;	//Requested an unknown index
			}
		}
		
		// MARK: - Functions (mutating)

		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
			@return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		*/
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override {
			if (item.ownerType != &typeid(XMLPolygon<Vert, VertWrap, Hole, HoleWrap>))
				return base::insert(std::move(cargo), item);
			switch (item.index) {
				case FieldIndex::hole:
					base::get().emplaceHole(clone(m_hole));
					break;
				default:
					break;
			}
			return true;
		}
		
	private:
			///Temporary mutable storage for incoming hole polygons
		mutable Hole m_hole;
	};

}  // namespace active::serialise::xml

#endif	//ACTIVE_SERIALISE_XML_XML_POLYGON
