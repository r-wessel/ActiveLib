/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_POLY_POINT
#define ACTIVE_GEOMETRY_POLY_POINT

#include "Active/Geometry/Point.h"

namespace active::geometry {
	
	using vertex_id = uint32_t;

	class Line;
	
	/*!
		Overrides the Point class to support curved edges, primarily for Polygon vertices
		
		PolyPoint defines the sweep angle for an edge leading to this point (0.0 = straight edge). It also includes an ID, a number that can be
		used to uniquely identify a specific vertex within a polygon. This ID can ensure that operations modifying the order of vertices does not
		affect associations between vertices and other data.
	*/
	class PolyPoint : public Point {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<PolyPoint>;
			///Shared pointer
		using Shared = std::shared_ptr<PolyPoint>;
			///Optional
		using Option = std::optional<PolyPoint>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		PolyPoint() {}
		/*!
			Constructor
			@param x X coordinate
			@param y Y coordinate
			@param z Z coordinate
			@param angle The sweep angle to this position (0 = straight line)
		 	@param id The point ID
		*/
		PolyPoint(double x, double y, double z = 0.0, double angle = 0.0, vertex_id id = 0) :
				PolyPoint{Point{x, y, z}, angle, id}	{}
		/*!
			Constructor
			@param source The point position
			@param angle The sweep angle to this position (0 = straight line)
		 	@param id The point ID
		*/
		PolyPoint(const Point& source, double angle = 0.0, vertex_id id = 0);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		PolyPoint(const PolyPoint& source);
		/*!
			Destructor
		*/
		virtual ~PolyPoint() = default;
		
		/*!
			Clone method
			@return A clone of this object
		*/
		PolyPoint* clonePtr() const override	{ return new PolyPoint(*this); }
		
		// MARK: - Variables
		
			///The sweep angle of an edge leading to this point (0.0 = atraight line)
		double sweep = 0.0;
			///An optional unique ID for the vertex, e.g. for maintaining links to a specific vertex within a polygon
		vertex_id id = 0;
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The point to assign
			@return A reference to this
		*/
		PolyPoint& operator= (const PolyPoint& source);
		/*!
			Assignment operator
			@param source The point to assign
			@return A reference to this
		*/
		PolyPoint& operator= (const Point& source) { Point::operator= (source); return *this; }
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the vertex describes an arc
			@return True if an arc is described
		*/
		bool isArc(double prec = math::eps) const { return !math::isZero(sweep, prec); }
		/*!
			Calculates the distant from a specified point to this in 2D
			@param ref A point to calculate the distance from
			@return The distance from the point to this
		*/
		double lengthFrom2D(const Point& ref) const override;
		
		// MARK: - Functions (mutating)
		/*!
			Set the sweep angle to this point from the specified line
			@param ref The reference line (centre to sweep start)
		*/
		void setSweep(const Line& ref);
		/*!
			Set the sweep angle for an arc starting parallel to a line
			@param ref The reference line
		*/
		void setSweepParallel(const Line& ref);
	};
	
}

#endif	//ACTIVE_GEOMETRY_POLY_POINT
