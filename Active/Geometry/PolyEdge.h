/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_POLY_EDGE
#define ACTIVE_GEOMETRY_POLY_EDGE

#include "Active/Geometry/PolyPoint.h"
#include "Active/Geometry/Rotation.h"
#include "Active/Geometry/Vector3.h"

#include <memory>

namespace active::geometry {
	
	class Arc;
	class Matrix3x3;
	class Plane;
	class XList;

	/*!
		Class to represent an edge (curved or straight)
	*/
	class PolyEdge {
	public:
		
		// MARK: - Types
		
		using enum Rotation;
			///Unique pointer
		using Unique = std::unique_ptr<PolyEdge>;
			///Shared pointer
		using Shared = std::shared_ptr<PolyEdge>;
			///Optional
		using Option = std::optional<PolyEdge>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		PolyEdge() {}
		/*!
			Constructor
			@param origin Origin point of the edge
			@param end End point of the edge
		*/
		PolyEdge(const Point& origin, const PolyPoint& end);
		/*!
			Constructor
			@param origin Origin point of the edge
			@param end End point of the edge
			@param radius The edge radius (sign indicates arc centre side (+ve = right, -ve = left, 0 = linear)
			@param rotation The rotation direction of the edge
			@param prec The required precision
		*/
		PolyEdge(const Point& origin, const Point& end, double radius, Rotation rotation = anticlockwise, double prec = math::eps);
		/*!
			Constructor
			@param arc An arc describing the poly-edge
		*/
		PolyEdge(const Arc& arc);
		
		// MARK: - Variables
		
			///The edge origin
		Point origin;
			///The edge end
		PolyPoint end;
			///The normal to the plane the edge lies in
		Vector3 normal = Vector3(0.0, 0.0, 1.0);	///The normal to the plane the edge is aligned to (vertical by default)

		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref The edge to compare with this
			@return True if the edges are identical
		*/
		bool operator== (const PolyEdge& ref) const { return isEqual3D(ref); }
		/*!
			Inequality operator
			@param ref The edge to compare with this
			@return True if the edges are different
		*/
		bool operator!= (const PolyEdge& ref) const { return !isEqual3D(ref); }
		/*!
			Assignment operator
			@param source The edge to be assigned to this
			@return A reference to this edge
		*/
		PolyEdge& operator= (const PolyEdge& source);
		/*!
			Addition operator
			@param offset The amount to add to the edge
			@return An offset edge
		*/
		PolyEdge operator+ (const Point& offset) const;
		/*!
			Addition with assignment operator
			@param offset The amount to add to the edge
			@return A reference to this
		*/
		PolyEdge& operator+= (const Point& offset);
		/*!
			Subtraction operator
			@param offset The amount to subtract from the edge
			@return An offset edge
		*/
		PolyEdge operator- (const Point& offset) const;
		/*!
			Subtraction with assignment operator
			@param offset The amount to subtract from the edge
			@return A reference to this
		*/
		PolyEdge& operator-= (const Point& offset);
		/*!
			Multiplication operator
			@param scale The amount to multiply the edge by
			@return A multiplied edge
		*/
		PolyEdge operator* (double scale) const;
		/*!
			Multiplication with assignment operator
			@param scale The amount to multiply the edge by
			@return A reference to this
		*/
		PolyEdge& operator*= (double scale);
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the edge by
			@return A multiplied edge
		*/
		PolyEdge operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication with assignment operator
			@param matrix The matrix to multiply the edge by
			@return A reference to this
		*/
		PolyEdge& operator*= (const Matrix3x3& matrix);
		
		// MARK: - Functions (const)

		/*!
			Determine if the edge is an arc
			@return True if the edge is an arc
		*/
		bool isArc(double prec = math::eps) const { return end.isArc(prec); }
		/*!
			Determine if two edges are equal in 2D
			@param ref The edge to compare with this
			@param prec The required precision
			@return True if the edges are identical
		*/
		bool isEqual2D(const PolyEdge& ref, double prec = math::eps) const;
		/*!
			Determine if two edges are equal
			@param ref The edge to compare with this
			@param prec The required precision
			@return True if the edges are identical
		*/
		bool isEqual3D(const PolyEdge& ref, double prec = math::eps) const;
		/*!
			Check if two edges are colinear
			@param ref The reference edge
			@param prec The required precision
			@return True if the edges are colinear
		*/
		bool isColinearTo2D(const PolyEdge& ref, double prec = math::eps) const;
		/*!
			Check if two edges are colinear
			@param ref The reference edge
			@return True if the edges are colinear
		*/
		bool isColinearTo3D(const PolyEdge& ref, double prec = math::eps) const;
		/*!
			Check if two edges are parallel
			@param ref The reference edge
			@param prec The required precision
			@return True if they are parallel
		*/
		bool isParallelTo2D(const PolyEdge& ref, double prec = math::eps) const;
		/*!
			Check if two edges are parallel
			@param ref The reference edge
			@return True if the edges are parallel
		*/
		bool isParallelTo3D(const PolyEdge& ref, double prec = math::eps) const;
		/*!
			Check if two edges are tangential (i.e. the end tangent of the reference edge matches the start tangent of this edge)
			@param ref The reference edge
			@param prec The coordinate precision
			@param anglePrec The angle precision
			@return True if the edges are tangential*/
		bool isTangentialTo2D(const PolyEdge& ref, double prec = math::eps, double anglePrec = math::epsAngle) const;
		/*!
			Get the radius of the edge
		 	@param isSigned True if the radius should be signed for the side of arc centres (left = -ve, right = +ve)
			@return The radius of the edge (0 = straight edge)
		*/
		double getRadius(bool isSigned = false) const;
		/*!
			Get the edge sweep angle
			@return The edge sweep angle (0.0 = straight line)
		*/
		double getSweep() const { return end.sweep; }
		/*!
			Get the azimuth angle of the edge
			@return The azimuth angle
		*/
		double azimuthAngle() const;
		/*!
			Get the altitude angle of the edge
			@return The altitude angle
		*/
		double altitudeAngle() const;
		/*!
			Get the tangent angle at the edge start
			@return The start tangent angle
		*/
		double startTangent() const;
		/*!
			Get the tangent angle at the edge end
			@return The end tangent angle
		*/
		double endTangent() const;
		/*!
			Get the tangent angle at the specified point
		 	@param ref The reference point
			@return The tangent angle at the specified point
		*/
		double getTangentAt(const Point& ref) const;
		/*!
			Get the center of the edge
			@return The edge centre (centre of arc or midpoint of straight line)
		*/
		Point centre() const;
		/*!
			Get the midpoint of the edge
			@return The midpoint of the edge
		*/
		Point midpoint() const;
		/*! Get the edge area (line edges are always zero)
		 	@param isResultSigned True if the result should be signed (reflecting the arc angle)
			@return  The edge area
		*/
		double getArea(bool isResultSigned = false) const;
		/*!
			Get the length of the edge in 2D
			@return The 2D edge length*/
		double length2D() const;
		/*!
			Get the length of the edge
			@return The length of the edge
		*/
		double length3D() const;
		/*!
			Get the edge as an arc
			@param prec The required precision
			@return The edge as an arc (nullopt if the edge is not an arc)
		*/
		std::optional<Arc> asArc(double prec = math::eps) const;
		/*!
			Get the closest point in the edge to a given point in 2D
			@param ref The reference point
			@return The closest point in the edge to the reference point
		*/
		Point closestPointTo2D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point in the edge to a given point
			@param ref The reference point
			@return The closest point in the edge to the reference point
		*/
		Point closestPointTo3D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point along the edge to a given point in 2D
			@param ref The reference point
			@return The closest point along the edge to the reference point
		*/
		Point closestPointAlong2D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point along the edge to a given point
			@param ref The reference point
			@return The closest point along the edge to the reference point
		*/
		Point closestPointAlong3D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the intersection point of two edges in 2D
			@param ref The reference edge
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith2D(const PolyEdge& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point of two edges in 3D
			@param ref The reference edge
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith3D(const PolyEdge& ref, XList& inter, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the edge in 2D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the edge in 3D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf3D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if a point is enclosed by a edge in 2D
			@param ref The reference point
			@param prec The required precision
			@return True if the edge encloses the point
		*/
		bool encloses2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if a point is enclosed by a edge
			@param ref The reference point
			@return True if the edge encloses the point
		*/
		bool encloses3D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if this edge is overlapped by a reference edge (a touching vertex is not an overlap)
			@param ref The reference edge
			@param prec The required precision
			@return True if the edge overlaps the reference edge
		*/
		bool overlaps2D(const PolyEdge& ref, double prec = math::eps) const;
		
		// MARK: - Functions (mutating)

		/*!
			Set the edge radius
			@param radius The edge radius (sign indicates arc centre side (+ve = right, -ve = left, 0 = linear)
			@param rotation The rotation direction of the edge (undefined = use existing, default = anticlockwise)
			@param prec The required precision
		*/
		void setRadius(double radius, std::optional<Rotation> rotation = std::nullopt, double prec = math::eps);
		/*!
			Set the edge sweep angle
			@param sweep The edge sweep angle (0.0 = straight line)
		*/
		void setSweep(double sweep) { end.sweep = sweep; }
		/*!
			Stretch the origin point for this edge to the specified point
			@param pt The point to stretch to
			@param canInvert True if the direction can be inverted
		*/
		void stretchOrigin(const Point& pt, bool canInvert = true, double prec = math::eps);
		/*!
			Stretch the end point for this edge to the specified point
			@param pt The point to stretch to
		*/
		void stretchEnd(const Point& pt, double prec = math::eps);
		/*!
			Set the level (z coordinate) of the edge vertices
			@param z The required level
		*/
		void setBaseLevel(double z = 0.0);
		/*!
			Offset the edge by the specified amount
			@param shift The amount to offset the edge by
		*/
		void offset(double shift);
		/*!
			Extend the edge length by the specified amount
			It is also possible to shorten the edge or to set *len* to negative values.
			If the edge is an arc, it cannot be extended further than to a full circle,
			both to the positive and to the negative direction.
			@param len The amount to extend the edge by
 			@param byEnd True to extend the edge by the endpoint
		*/
		void extend(double len, bool byEnd = true);
		/*!
			Extend the edge to the point nearest te specified point
			@param ref The point to extend to
 			@param byEnd True to extend the edge by the endpoint
		*/
		void extend(const Point& ref, bool byEnd = true);
		/*!
			Split the edge at the specified point
			@param pos The point to split the edge at
		 	@param offcut The offcut created by the split
		 	@param keepOrig True to maintain the edge origin
		*/
		void split(const Point& pos, PolyEdge& offcut, bool keepOrig = true);
		/*!
			Move the edge by the specified distance and azimuth/altitude angles
			@param len The distance to move the edge
			@param azim The azimuth angle to move the edge along
			@param alt The altitude angle to move the edge along
		*/
		void movePolar(double len, double azim, double alt);
		/*!
			Move the edge by the specified distance and angle
			@param len The distance to move the edge
			@param angle The angle to move the edge along
		*/
		void movePolar(double len, double angle);
		/*!
			Flip the edge
		*/
		PolyEdge& flip();
	};
	
}

#endif	//ACTIVE_GEOMETRY_POLY_EDGE
