/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_ARC
#define ACTIVE_GEOMETRY_ARC

#include "Active/Geometry/Box.h"
#include "Active/Geometry/Plane.h"
#include "Active/Geometry/Point.h"
#include "Active/Geometry/PolyPoint.h"
#include "Active/Geometry/Vector3.h"

#include <memory>

namespace active::geometry {
	
	class Leveller;
	class Line;
	class Matrix3x3;
	class XList;

	/// Class to represent an arc
	class Arc : public utility::Cloner {
	public:

		// MARK: - Types
		
		using enum Position;
		
			///Unique pointer
		using Unique = std::unique_ptr<Arc>;
			///Shared pointer
		using Shared = std::shared_ptr<Arc>;
			///Optional
		using Option = std::optional<Arc>;

		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		Arc();
		/*!
			Constructor
			@param centre Centre of the arc
			@param start Start angle of the arc sweep
			@param sweep Arc sweep angle
			@param norm Normal to the arc plane
		*/
		Arc(const Point& centre, double radius, double sweep = 2.0 * math::pi, double start = 0, const Vector3& norm = Vector3(0, 0, 1));
		/*!
			Constructor
			@param centre Centre of the arc
			@param pt1 First point on the arc
			@param pt2 Second point on the arc
		 	@param isClockwise True if the arc travels in a clockwise direction from pt1 to pt2
		*/
		Arc(const Point& centre, const Point& pt1, const Point& pt2, bool isClockwise);
		/*!
			Constructor
			@param pt1 First point on the arc (Note: points must be consecutive)
			@param pt2 Second point on the arc
			@param pt3 Third point on the arc
		*/
		Arc(const Point& pt1, const Point& pt2, const Point& pt3);
		/*!
			Constructor
			@param origin Origin point of the arc
			@param end End point of the arc
		*/
		Arc(const Point& origin, const PolyPoint& end);
		
		/*!
			Clone method
			@return A clone of this object
		*/
		Arc* clonePtr() const { return new Arc(*this); }

		// MARK: - Variables
		
			//The arc centre
		Point centre;
			//The normal of the plane the arc lies along
		Vector3 normal;
			//The arc radius
		double radius;
			//The arc start angle
		double startAngle;
			//The arc sweep angle
		double sweep;

		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref The line to compare with this
			@return True if the lines are identical
		*/
		bool operator== (const Arc& ref) const { return isEqual3D(ref); }
		/*!
			Inequality operator
			@param ref The line to compare with this
			@return True if the lines are different
		*/
		bool operator!= (const Arc& ref) const { return !isEqual3D(ref); }
		/*!
			Assignment operator
			@param source The arc to be assigned to this
			@return A reference to this arc
		*/
		Arc& operator= (const Arc& source);
		/*!
			Addition operator
			@param offset The amount to add to the arc
			@return An offset arc
		*/
		Arc operator+ (const Point& offset) const;
		/*!
			Addition with assignment operator
			@param offset The amount to add to the arc
			@return A reference to this
		*/
		Arc& operator+= (const Point& offset);
		/*!
			Subtraction operator
			@param offset The amount to subtract from the arc
			@return An offset arc
		*/
		Arc operator- (const Point& offset) const;
		/*!
			Subtraction with assignment operator
			@param offset The amount to subtract from the arc
			@return A reference to this
		*/
		Arc& operator-= (const Point& offset);
		/*!
			Multiplication operator
			@param scale The amount to multiply the arc by
			@return A multiplied arc
		*/
		Arc operator* (double scale) const;
		/*!
			Multiplication with assignment operator
			@param scale The amount to multiply the arc by
			@return A reference to this
		*/
		Arc& operator*= (double scale);
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the arc by
			@return A multiplied arc
		*/
		Arc operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication with assignment operator
			@param matrix The matrix to multiply the arc by
			@return A reference to this
		*/
		Arc& operator*= (const Matrix3x3& matrix);

		// MARK: - Functions (const)
		
		/*!
			Determine if the arc is valid
			@return True if the arc is valid
		*/
		double isValid(double prec = math::eps) const;
		/*!
			Determine if two arcs are equal in 2D
			@param ref The arc to compare with this
			@param prec The required precision
			@return True if the arcs are identical
		*/
		bool isEqual2D(const Arc& ref, double prec = math::eps) const;
		/*!
			Determine if two arcs are equal
			@param ref The arc to compare with this
			@param prec The required precision
			@return True if the arcs are identical
		*/
		bool isEqual3D(const Arc& ref, double prec = math::eps) const;
		/*!
			Get the origin point of the arc
			@return The origin point
		*/
		Point getOrigin() const;
		/*!
			Get the end point of the arc
			@return The end point
		*/
		PolyPoint getEnd() const;
		/*!
			Get the midpoint of the arc
			@return The midpoint of the arc
		*/
		Point midpoint() const;
		/*!
			Get the arc end angle
			@return The arc end angle
		*/
		double getEndAngle() const { return startAngle + sweep; }
		/*!
			Get the plane of the arc
			@return The plane of the arc
		*/
		Plane getPlane() const;
		/*!
			Get the bounds of the arc
			@return The bounds of the arc
		*/
		Box::Unique bounds() const;
		/*!
			Get the length of the arc in 2D
			@return The 2D arc length
		*/
		double length2D() const;
		/*!
			Get the length of the arc
			@return The length of the arc
		*/
		double length3D() const;
		/*!
			Get the arc area
		  	@param isArcOnly True if only the area enclosed by the arc is included (not the wedge to the centre)
		 	@param isResultSigned True if the result should be signed (reflecting the arc angle)
			@return  The arc area
		*/
		double getArea(bool isArcOnly = true, bool isResultSigned = false) const;
		/*!
			Check if two arcs are colinear
			@param ref The reference arc
			@return True if the arcs are colinear
		*/
		bool isColinearTo2D(const Arc& ref, double prec = math::eps) const;
		/*!
			Check if two arcs are colinear
			@param ref The reference arc
			@return True if the arcs are colinear
		*/
		bool isColinearTo3D(const Arc& ref, double prec = math::eps) const;
		/*!
			Check if two arcs are parallel
			@param ref The reference arc
			@return True if they are parallel
		*/
		bool isParallelTo2D(const Arc& ref, double prec = math::eps) const;
		/*!
			Check if two arcs are concentric
			@param ref The reference arc
			@return True if the arcs are concentric
		*/
		bool isParallelTo3D(const Arc& ref, double prec = math::eps) const;
		/*!
			Get the closest point along the arc to a given point in 2D
			@param ref The reference point
			@return The closest point along the arc to the reference point
		*/
		Point closestPointAlong2D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point along the arc to a given point
			@param ref The reference point
			@return The closest point along the line to the reference point
		*/
		Point closestPointAlong3D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point in the arc to a given point in 2D
			@param ref The reference point
			@return The closest point in the arc to the reference point
		*/
		Point closestPointTo2D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point in the arc to a given point
			@param ref The reference point
			@return The closest point in the arc to the reference point
		*/
		Point closestPointTo3D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the intersection point of an arc and plane in 3D
			@param ref The reference plane
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
 		vertex_index intersectionWith(const Plane& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point of an arc and line in 2D
			@param ref The reference line
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith2D(const Line& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point of two arcs in 2D
			@param ref The reference arc
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith2D(const Arc& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point of an arc and line in 3D
			@param ref The reference line
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
 		vertex_index intersectionWith3D(const Line& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point of two arcs in 3D
			@param ref The reference arc
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith3D(const Arc& ref, XList& inter, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the arc in 2D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the arc in 3D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf3D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if a point is enclosed by an arc in 2D
			@param ref The reference point
			@return True if the arcarc encloses the point
		*/
		bool encloses2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if a point is enclosed by an arc
			@param ref The reference point
			@return True if the arc encloses the point
		*/
		bool encloses3D(const Point& ref, double prec = math::eps) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the arc end angle
			@param end The arc end angle
		*/
		void setEndAngle(double end) { sweep = end - startAngle; }
		/*!
			Move the arc by the specified distance and azimuth/altitude angles
			@param len The distance to move the arc
			@param azim The azimuth angle to move the arc along
			@param alt The altitude angle to move the arc along
		*/
		void movePolar(double len, double azim, double alt);
		/*!
			Move the arc by the specified distance and angle
			@param len The distance to move the arc
			@param angle The angle to move the arc along
		*/
		void movePolar(double len, double angle);
		/*!
			Adjust the radius by the specified amount
			@param inc The expansion amount
		*/
		void expand(double inc) { radius += inc; }
		/*!
			Spin the arc about its axis by the specified angle
			@param rotAngle The rotation angle
		*/
		void spin(double rotAngle) { startAngle += rotAngle; }
		/*!
			Flip the arc
		*/
		void flip();
		
	protected:
		/*!
			Initialise a Leveller to transform the arc to flat plane
			@param level The Leveller to be initialised
			@param prec The required precision
		*/
		void initLevel(Leveller& level, double prec = math::eps) const;
		/*!
			Create an intersection point
			@param pt The intersection point
			@param arc The source arc
			@param ref The reference arc
			@param inter The intersection list to populate
			@param prec The required precision
			@return True if a new intersection was formed
		*/
		bool createIntersect(const Point& pt, const Arc& arc, const Arc& ref,
				XList& inter, double prec = math::eps) const;
		/*!
			Create an intersection point
			@param pt The intersection point
			@param arc The source arc
			@param ref The reference line
			@param inter The intersection list to populate
			@param prec The required precision
			@return True if a new intersection was formed
		*/
		bool createIntersect(const Point& pt, const Arc& arc, const Line& ref,
				XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point between a co-planar horizontal line and arc
			@param ref The reference line
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWithLevel(const Line& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point between two co-planar horizontal arcs
			@param ref The reference arc
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWithLevel(const Arc& ref, XList& inter, double prec = math::eps) const;
	};
	
}

#endif	//ACTIVE_GEOMETRY_ARC
