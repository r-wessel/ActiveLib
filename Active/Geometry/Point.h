/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_POINT
#define ACTIVE_GEOMETRY_POINT

#include "Active/Geometry/Position.h"
#include "Active/Utility/Cloner.h"
#include "Active/Utility/MathFunctions.h"

#include <memory>
#include <optional>

namespace active::geometry {
	
	class Matrix3x3;
	class Matrix4x4;

		///Index of a vertex, e.g. within a polygon
	using vertex_index = int32_t;
		///Optional vertex index (for an undefined or missing vertex index)
	using vertOption = std::optional<vertex_index>;
		///Index of a part, e.g. a hole within a polygon
	using part_index = int32_t;
		///Optional part index (for an undefined or missing part index)
	using partOption = std::optional<part_index>;
	
	/*!
		A class to represent a point in 3-dimensional space, i.e. with x, y, z coordinates
		
		Note that this class serves for both 2D and 3D contexts (rather than having a separate class for each). Functionality that is specific
		to either context is clearly labelled, e.g. 'isEqual2D' vs 'isEqual3D'.
	*/
	class Point : public utility::Cloner {
	public:
		
		// MARK: - Types
		
		using enum Position;

			///Unique pointer
		using Unique = std::unique_ptr<Point>;
			///Shared pointer
		using Shared = std::shared_ptr<Point>;
			///Optional
		using Option = std::optional<Point>;
		
		// MARK: Constructors
			
		/*!
			Constructor
		*/
		Point();
		/*!
			Constructor
			@param x X coordinate
			@param y Y coordinate
			@param z Z coordinate
		*/
		Point(const double& x, const double& y, const double& z = 0.0);
		/*!
			Constructor
			@param source2D A 2D point, i.e. only the x/y coordinates are relevant
			@param z z coordinate
		*/
		Point(const Point& source2D, double z);
		/*!
			Destructor
		*/
		virtual ~Point() = default;
		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Point* clonePtr() const { return new Point(*this); }
		
		// MARK: Public variables
			
			///X coordinate
		double x;
			///Y coordinate
		double y;
			///Z coordinate
		double z;
		
		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref The point to compare with this
			@return True if the points are identical
		*/
		virtual bool operator== (const Point& ref) const { return isEqual3D(ref); }
		/*!
			Inequality operator
			@param ref The point to compare with this
			@return True if the points are different
		*/
		virtual bool operator!= (const Point& ref) const { return !isEqual3D(ref); }
		/*!
			Less-than operator
			@param ref The point to compare with this
			@return True if this is less than ref
		*/
		virtual bool operator< (const Point& ref) const;
		/*!
			Assignment operator
			@param source The point to assign
			@return A reference to this
		*/
		Point& operator= (const Point& source);
		/*!
			Addition operator
			@param offset The point to add
			@return The sum of this and the specified points
		*/
		Point operator+ (const Point& offset) const;
		/*!
			Addition and assignment operator
			@param offset The point to add
			@return A reference to this
		*/
		virtual Point& operator+= (const Point& offset);
		/*!
			Substraction operator
			@param offset The point to substract
			@return The result of this less the specified point
		*/
		Point operator- (const Point& offset) const;
		/*!
			Substraction and assignment operator
			@param offset The point to substract
			@return A reference to this
		*/
		virtual Point& operator-= (const Point& offset);
		/*!
			Multiplication operator
			@param mult The factor to multiply the point by
			@return The result of the multiplication of this and the specified factor
		*/
		Point operator* (const double& mult) const;
		/*!
			Multiplication operator
			@param mult The x/y/z factors to multiply the point by
			@return The result of the multiplication of this and the specified factor
		*/
		Point operator* (const Point& mult) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the point by
			@return The result of the multiplication
		*/
		Point operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the point by
			@return The result of the multiplication
		*/
		Point operator* (const Matrix4x4& matrix) const;
		/*!
			Multiplication and assignment operator
			@param mult The factor to multiply the point by
			@return A reference to this
		*/
		virtual Point& operator*= (const double& mult);
		/*!
			Multiplication and assignment operator
			@param mult The x/y/z factors to multiply the point by
			@return A reference to this
		*/
		virtual Point& operator*= (const Point& mult);
		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply the point by
			@return A reference to this
		*/
		virtual Point& operator*= (const Matrix3x3& matrix);
		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply the point by
			@return A reference to this
		*/
		virtual Point& operator*= (const Matrix4x4& matrix);
		/*!
			Division operator
			@param mult The factor to divide the point by
			@return The result of the division of this by the specified factor
		*/
		Point operator/ (const double& mult) const;
		/*!
			Division and assignment operator
			@param mult The factor to divide the point by
			@return A reference to this
		*/
		virtual Point& operator/= (const double& mult);
		
		// MARK: - Functions (const)
		
		/*!
			Determine if two points are equal in 2D
			@param ref The point to compare with this
			@param prec Precision
			@return True if the points are identical
		*/
		virtual bool isEqual2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if two points are equal
			@param ref The point to compare with this
			@param prec Precision
			@return True if the points are identical
		*/
		virtual bool isEqual3D(const Point& ref, double prec = math::eps) const;
		/*!
			Calculates the 2D distance from this to a specified point
			@param ref The point to compare with this
			@return The 2D distance between the points
		*/
		virtual double lengthFrom2D(const Point& ref) const;
		/*!
			Calculates the distant from a specified point to this in 3D
			@param ref A point to calculate the distance from
			@return The distance from the point to this
		*/
		virtual double lengthFrom3D(const Point& ref) const;
		/*!
			Return the point with coordinates rounded to the specified level of precision
			@return The point with rounded coordinates (z set to 0.0)
		*/
		Point rounded2D(double prec = math::eps) const;
		/*!
			Return the point with coordinates rounded to the specified level of precision
			@return The point with rounded coordinates
		*/
		Point rounded3D(double prec = math::eps) const;
		/*!
			Calculates the azimuth angle from this to the specified point
			@param ref A point to calculate the angle to
			@return The azimuth angle from this to the specified point (0 to 2.pi)
		*/
		double azimuthAngleTo(const Point& ref) const;
		/*!
			Calculates the altitude angle from this to the specified point
			@param ref A point to calculate the angle to
			@return The altitude angle from this to the specified point (-pi / 2 to pi / 2)
		*/
		double altitudeAngleTo(const Point& ref) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Offset a point in the x/y plane by the specified distance and angle
			@param len The distance to move the point
			@param angle The angle on which the point will move
			@return A reference to this
		*/
		Point& movePolar(double len, double angle);
		/*!
			Offset a point in the x/y/z plane by the specified distance and azimuth/altitude angle
			@param len The distance to move the point
			@param azim The azimuth angle on which the point will move
			@param alt The altitude angle on which the point will move
			@return A reference to this
		*/
		Point& movePolar(double len, double azim, double alt);
	};

}

#endif	//ACTIVE_GEOMETRY_POINT
