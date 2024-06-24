/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_LINE
#define ACTIVE_GEOMETRY_LINE


#include <memory>

#include "Active/Geometry/XPoint.h"
#include "Active/Utility/MathFunctions.h"

namespace active::geometry {
	
	class Matrix3x3;
	class XList;

	/// Class to represent a line
	class Line {
	public:
		
		// MARK: - Types

			///Unique pointer
		using Unique = std::unique_ptr<Line>;
			///Shared pointer
		using Shared = std::shared_ptr<Line>;
			///Optional
		using Option = std::optional<Line>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param origin Origin point of the line
			@param end End point of the line
		*/
		Line(const Point& origin, const Point& end) : origin(origin), end(end) {}
		/*!
			Constructor
			@param origin Origin point of the line
			@param rad The radial offset to the end of the line
			@param azim The azimuth angle of the line
		*/
		Line(const Point& origin, double rad, double azim) : origin(origin), end(origin) { end.movePolar(rad, azim); }
		
		// MARK: - Variables
		
			///The line origin
		Point origin;
			///The line end
		Point end;

		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref The line to compare with this
			@return True if the lines are identical
		*/
		bool operator== (const Line& ref) const { return isEqual3D(ref); }
		/*!
			Inequality operator
			@param ref The line to compare with this
			@return True if the lines are different
		*/
		bool operator!= (const Line& ref) const { return !isEqual3D(ref); }
		/*!
			Addition operator
			@param offset The amount to add to the line
			@return An offset line
		*/
		Line operator+ (const Point& offset) const;
		/*!
			Addition with assignment operator
			@param offset The amount to add to the line
			@return A reference to this
		*/
		Line& operator+= (const Point& offset);
		/*!
			Subtraction operator
			@param offset The amount to subtract from the line
			@return An offset line
		*/
		Line operator- (const Point& offset) const;
		/*!
			Subtraction with assignment operator
			@param offset The amount to subtract from the line
			@return A reference to this
		*/
		Line& operator-= (const Point& offset);
		/*!
			Multiplication operator
			@param scale The amount to multiply the line by
			@return A multiplied line
		*/
		Line operator* (double scale) const;
		/*!
			Multiplication with assignment operator
			@param scale The amount to multiply the line by
			@return A reference to this
		*/
		Line& operator*= (double scale);
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the line by
			@return A multiplied line
		*/
		Line operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication with assignment operator
			@param matrix The matrix to multiply the line by
			@return A reference to this
		*/
		Line& operator*= (const Matrix3x3& matrix);
		
		// MARK: - Functions (const)
		
		/*!
			Determine if two lines are equal in 2D
			@param ref The line to compare with this
			@param prec The required precision
			@return True if the lines are identical
		*/
		bool isEqual2D(const Line& ref, double prec = math::eps) const;
		/*!
			Determine if two lines are equal
			@param ref The line to compare with this
			@param prec The required precision
			@return True if the lines are identical
		*/
		bool isEqual3D(const Line& ref, double prec = math::eps) const;
		/*!
			Get the length of the line in 2D
			@return The 2D line length*/
		double length2D() const;
		/*!
			Get the length of the line
			@return The length of the line
		*/
		double length3D() const;
		/*!
			Get the azimuth angle of the line
			@return The azimuth angle
		*/
		double azimuthAngle() const;
		/*!
			Get the altitude angle of the line
			@return The altitude angle
		*/
		double altitudeAngle() const;
		/*!
			Get the angle between two lines
			@param ref The reference line
			@return The angle between the lines
		*/
		double angleTo(const Line& ref) const;
		/*!
			Get the midpoint of the line
			@return The midpoint of the line
		*/
		Point midpoint() const;
		/*!
			Get the height of the line at a specified point
			@param ref The point at which to calculate the height
			@return The height at the specified point
		*/
		double heightAt(const Point& ref) const;
		/*!
			Check if two lines are colinear
			@param ref The reference line
			@return True if the lines are colinear
		*/
		bool isColinearTo2D(const Line& ref, double prec = math::eps) const;
		/*!
			Check if two lines are colinear
			@param ref The reference line
			@return True if the lines are colinear
		*/
		bool isColinearTo3D(const Line& ref, double prec = math::eps) const;
		/*!
			Check if two lines are parallel
			@param ref The reference line
			@return True if they are parallel
		*/
		bool isParallelTo2D(const Line& ref, double prec = math::eps) const;
		/*!
			Check if two lines are parallel
			@param ref The reference line
			@return True if the lines are parallel
		*/
		bool isParallelTo3D(const Line& ref, double prec = math::eps) const;
		/*!
			Get the closest point in the line to a given point in 2D
			@param ref The reference point
			@return The closest point in the line to the reference point
		*/
		Point closestPointTo2D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point in the line to a given point
			@param ref The reference point
			@return The closest point in the line to the reference point
		*/
		Point closestPointTo3D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point along the line to a given point in 2D
			@param ref The reference point
			@return The closest point along the line to the reference point
		*/
		Point closestPointAlong2D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the closest point along the line to a given point
			@param ref The reference point
			@return The closest point along the line to the reference point
		*/
		Point closestPointAlong3D(const Point& ref, double prec = math::eps) const;
		/*!
			Get the intersection point of two lines in 2D (NB: either along or projected beyond the line extents)
			@param ref The reference line
			@param prec The required precision
			@return The intersection point (nullopt if no intersection is possible)
		*/
		XPoint::Option intersectionWith2D(const Line& ref , double prec = math::eps) const;
		/*!
			Get the intersection point of two lines in 2D
			@param ref The reference line
			@param inter The intersection list to populate
			@param prec The required precision
			@return The number of intersections calculated
		*/
		vertex_index intersectionWith2D(const Line& ref, XList& inter, double prec = math::eps) const;
		/*!
			Get the intersection point of two lines in 3D
			ref: The reference line
			inter: The intersection list to populate
			prec: The required precision
			return: The number of intersections calculated
		*/
		vertex_index intersectionWith3D(const Line& ref, XList& inter, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the line in 2D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the line in 3D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf3D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if a point is enclosed by a line in 2D
			@param ref The reference point
			@return True if the lineline encloses the point
		*/
		bool encloses2D(const Point& ref, double prec) const;
		/*!
			Determine if a point is enclosed by a line
			@param ref The reference point
			@return True if the lineline encloses the point
		*/
		bool encloses3D(const Point& ref, double prec) const;
		/*!
			Get the angle between two lines
			@param ref The reference line
			@return The angle between the lines
		*/
		double angleTo2D(const Line& ref) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Extend the line length by the specified amount
			@param len The amount to extend the line by
		*/
		void extend(double len);
		/*!
			Move the line by the specified distance and azimuth/altitude angles
			@param len The distance to move the line
			@param azim The azimuth angle to move the line along
			@param alt The altitude angle to move the line along
		*/
		void movePolar(double len, double azim, double alt);
		/*!
			Move the line by the specified distance and angle
			@param len The distance to move the line
			@param angle The angle to move the line along
		*/
		void movePolar(double len, double angle);
		/*!
			Flip the line
		*/
		void flip();
	};
	
}

#endif	//ACTIVE_GEOMETRY_LINE
