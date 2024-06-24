/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_LINEAR_EQUATION
#define ACTIVE_GEOMETRY_LINEAR_EQUATION

#include "Active/Geometry/Point.h"
#include "Active/Geometry/XPoint.h"

#include <memory>

namespace active::geometry {
	
	class Line;
	
	/// Class to represent a linear equation
	class LinEquation {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<LinEquation>;
			///Shared pointer
		using Shared = std::shared_ptr<LinEquation>;
			///Optional
		using Option = std::optional<LinEquation>;
		
		// MARK: - Factory functions
		
		/*!
			Create a new linear equation from a point (origin assumed to be 0,0,0)
			@param source The end of the line
			@return A linear equation (nullopt if input is invalid)
		*/
		static Option create(const Point& source);
		/*!
			Create a new linear equation from two points in a line
			@param start The start of the line
			@param end The end of the line
			@return A linear equation (nullopt if input is invalid)
		*/
		static Option create(const Point& start, const Point& end);
		/*!
			Create a new linear equation from a line
			@param source A line
			@return A linear equation (nullopt if input is invalid)
		*/
		static Option create(const Line& source);
		
		// MARK: - Constructors
		
		/*!
			Constructor 
		*/
		LinEquation();
		/*!
			Constructor
			@param angle Angle of the line
			@param source A point along the line
		*/
		LinEquation(double angle, const Point& source = Point());
		/*!
			Copy constructor
			@param source The object to copy
		*/
		LinEquation(const LinEquation& source);
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		LinEquation& operator= (const LinEquation& source);
		/*!
			Equality operator
			@param ref The object to compare
			@return True if ref is identical to this
		*/
		bool operator== (const LinEquation& ref) const;
		/*!
			Test to determine if a point lies within this equation
			@param ref A point to test
			@return True if the point lies along the line
		*/
		bool operator== (const Point& ref) const;
		/*!
			Inequality operator
			@param ref The object to compare
			@return True if ref is not identical to this
		*/
		bool operator!= (const LinEquation& ref) const { return !(*this == ref); }
		/*!
			Less-than operator
			@param ref The object to compare
			@return True if the angle of this equation is less than ref
		*/
		bool operator< (const LinEquation& ref) const;
		
		// MARK: - Functions (const)
		
		/*!
			Test if the equation is parallel to the x axis
		 	@param prec The required precision
			@return True the equation is parallel to the x axis
		*/
		bool isXAxis(double prec = math::eps) const { return math::isZero(m_a, prec); }
		/*!
			Test if the equation is parallel to the y axis
		 	@param prec The required precision
			@return True the equation is parallel to the y axis
		*/
		bool isYAxis(double prec = math::eps) const { return math::isZero(m_b, prec); }
		/*!
			Test two equations to determine if they are perpendicular
			@param ref The reference line
			@param prec The required precision
			@return True if the equations are perpendicular
		*/
		bool isPerpendicularTo(const LinEquation& ref, double prec = math::eps) const;
		/*!
			Test two equations to determine if they are parallel
			@param ref The reference line
			@param prec The required precision
			@return True if the equations are parallel
		*/
		bool isParallelTo(const LinEquation& ref, double prec = math::eps) const;
		/*!
			Calculate the angle of this equation
			@return The equation's angle (0 to 2*pi)
		*/
		double azimuthAngle() const;
		/*!
			Create a linear equation opposite to this
			@return An unique_ptr to the opposite equation
		*/
		Option getFlipped() const;
		/*!
			Create a linear equation perpendicular to this which passes through the specified point
			@param ref The point through which the new equation must pass
			@return An unique_ptr to the perpendicular equation
		*/
		Option getPerpendicular(const Point& ref) const;
		/*!
			Create a linear equation parallel to this which passes through the specified point
			@param ref The point through which the new equation must pass
			@return An unique_ptr to the parallel equation
		*/
		Option getParallel(const Point& ref) const;
		/*!
			Calculate the angle between two linear equations
			@param ref The reference line
			@return The angle between the two lines
		*/
		double angleTo(const LinEquation& ref) const;
		/*!
			Calculate the intersection point of two equations
			@param ref The intersecting line
			@return An unique_ptr for the intersection point (or 0 if parallel)
		*/
		XPoint::Option intersectionWith(const LinEquation& ref) const;
		/*!
			Determine the relationship of a point to the equation
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point (along, left, right)
		*/
		Position positionOf(const Point& ref, double prec = math::eps) const;
		/*!
			Calculate the x coordinate at the specifed y coordinate
		 	@param y The y coordinate
			@param x The x coordinate (n/a if the equation is parallel to the x axis)
		 	@return False if the equation is parallel to the x axis
		*/
		bool xAtY(double y, double& x) const;
		/*!
			Calculate the y coordinate at the specifed x coordinate
		 	@param x The x coordinate
			@param y The y coordinate (n/a if the equation is parallel to the y axis)
		 	@return False if the equation is parallel to the y axis
		*/
		bool yAtX(double x, double& y) const;
		/*!
			Calculate the closest point along this to a another specified point
			@param ref The reference point
			@return The closest point
		*/
		Point closestPointTo(const Point& ref) const;
		/*!
			Calculate the minimum distance from a point to this
			@param ref The reference point
			@return The minimum distance
		*/
		double lengthTo(const Point& ref) const;
			
	private:
		
		/*!
			Calculate the equation based on a slope and origin
			@param start The origin for the equation
			@param dx Slope delta x
			@param dy Slope delta y
		*/
		void calculate(const Point& start = Point(), double dx = 1.0, double dy = 0.0);
		/*!
			Get an equation from the specified points
			@param start The start of the line
			@param end The end of the line
			@return A linear equation (nullopt if input is invalid)
		*/
		static Option getEquation(const Point& start, const Point& end);
		
		double m_a;
		double m_b;
		double m_c;
	};

}

#endif	//ACTIVE_GEOMETRY_LINEAR_EQUATION
