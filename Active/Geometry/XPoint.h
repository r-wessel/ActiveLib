/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_X_POINT
#define ACTIVE_GEOMETRY_X_POINT

#include "Active/Geometry/XInfo.h"

#include <array>

namespace active::geometry {
	
	/*!
		Class to represent an intersection point
	 
		NB: The word "intersection" is generally abbreviated to "X" in class names, e.g. `XInfo`, `XList` etc.
		An intersection in this context is always between just 2 geometric components, e.g. a line and arc. Each component is nominally considered
		to have a role in the intersection – as if one component is cutting the other – namely `blade` (the cutting component) and `target` (the
		component to be cut). Every function calculating an intersection will clearly identity the role of each component, as will the `XList`
		collecting the intersections.
		Every intersection point will contain 2 `XInfo` objects that describe its relationship to the `blade` and `target` (see the docs for `XInfo`).
	*/
	class XPoint : public Point {
	public:
		friend class XList;
		
		// MARK: - Types
		
		enum Role {
			target = 0,
			blade
		};
		
			///Unique pointer
		using Unique = std::unique_ptr<XPoint>;
			///Shared pointer
		using Shared = std::shared_ptr<XPoint>;
			///Optional
		using Option = std::optional<XPoint>;
		
		/*!
			Constructor
			@param source The intersection point
			@param oPos The relationship of the point to the target
			@param bPos The relationship of the point to the blade
		*/
		XPoint(const Point& source = Point(), Position oPos = Point::undefined,
				Position bPos = Point::undefined);
		
		/*!
			Clone method
			@return A duplicate of this target
		*/
		Point* clonePtr() const { return new XPoint(*this); }
		
		/*!
			Assignment operator
		 	@param source The object to copy
			@return A reference to this
		*/
		XPoint& operator= (const Point& source);
		
			//Data access methods
		/*!
			Get relationship of the intersecting point to the specified part
			@param role The object role
			@return The relative point position
		*/
		auto getInfo(Role role) const { return m_info[role]; }
		/*!
			Get the position of the intersection relative to the specified part
			@param role The object role
			@return The position of the intersection relative to the specified part
		*/
		auto getPos(Role role) const { return m_info[role].pos; }
		/*!
			Get the vertex index of an intersecting object
			@param role The object role
			@return The vertex index of an intersecting object
		*/
		vertOption getVertex(Role role) const { return m_info[role].vertexIndex; }
		/*!
			Get the part index of an intersecting object
			@param role The object role
			@return The part index of an intersecting object
		*/
		partOption getPart(Role role) const { return m_info[role].partIndex; }

		/*!
			Set the target intersection specs
			@param role The target part
			@param info The target intersection specs
		*/
		void setInfo(Role role, const XInfo& info) { m_info[role] = info; }
		/*!
			Set the position of the intersection relative to the target
			@param role The object role
			@param pos The position of the intersection relative to the target
		*/
		void setPos(Role role, Position pos) { m_info[role].pos = pos; }
		/*!
			Set the ID of the intersecting target segment
			@param role The object role
			@param vertexIndex The vertex index of an intersecting object
		*/
		void setVertex(Role role, vertOption vertexIndex) { m_info[role].vertexIndex = vertexIndex; }
		/*!
			Set the ID of the intersecting blade object
			@param role The object role
			@param partIndex The part index of an intersecting object
		*/
		void setPart(Role role, partOption partIndex) { m_info[role].partIndex = partIndex; }
		/*!
			Swap the blade and target intercepts
		*/
		void swapIntercept();
		
	private:
		std::array<XInfo, 2> m_info;
	};
	
}

#endif	//ACTIVE_GEOMETRY_X_POINT
