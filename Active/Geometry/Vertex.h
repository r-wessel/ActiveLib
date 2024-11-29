/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_VERTEX
#define ACTIVE_GEOMETRY_VERTEX

#include "Active/Geometry/Position.h"
#include "Active/Utility/Cloner.h"
#include "Active/Utility/MathFunctions.h"

#include <memory>
#include <optional>

namespace active::geometry {
	
	class Matrix3x3;
	class Matrix4x4;

	/*!
	 A class to represent a point in 3-dimensional space, i.e. with x, y, z coordinates
	 
	 Use this class in preference to Point in cases where memory overheads are important. Point has functions which means that
	 every allocated instance uses an extra 8 bytes.
	*/
	class Vertex {
	public:
		
		// MARK: - Types
		
		using enum Position;

			///Unique pointer
		using Unique = std::unique_ptr<Vertex>;
			///Shared pointer
		using Shared = std::shared_ptr<Vertex>;
			///Optional
		using Option = std::optional<Vertex>;
		
		// MARK: Constructors
			
		/*!
			Constructor
		*/
		Vertex();
		/*!
			Constructor
			@param x X coordinate
			@param y Y coordinate
			@param z Z coordinate
		*/
		Vertex(const double& x, const double& y, const double& z = 0.0);
		/*!
			Constructor
			@param source2D A 2D point, i.e. only the x/y coordinates are relevant
			@param z z coordinate
		*/
		Vertex(const Vertex& source2D, double z);
		
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
		bool operator== (const Vertex& ref) const { return isEqual3D(ref); }
		/*!
			Inequality operator
			@param ref The point to compare with this
			@return True if the points are different
		*/
		bool operator!= (const Vertex& ref) const { return !isEqual3D(ref); }
		/*!
			Less-than operator
			@param ref The point to compare with this
			@return True if this is less than ref
		*/
		bool operator< (const Vertex& ref) const;
		/*!
			Assignment operator
			@param source The point to assign
			@return A reference to this
		*/
		Vertex& operator= (const Vertex& source);
		/*!
			Addition operator
			@param offset The point to add
			@return The sum of this and the specified points
		*/
		Vertex operator+ (const Vertex& offset) const;
		/*!
			Addition and assignment operator
			@param offset The point to add
			@return A reference to this
		*/
		Vertex& operator+= (const Vertex& offset);
		/*!
			Substraction operator
			@param offset The point to substract
			@return The result of this less the specified point
		*/
		Vertex operator- (const Vertex& offset) const;
		/*!
			Substraction and assignment operator
			@param offset The point to substract
			@return A reference to this
		*/
		Vertex& operator-= (const Vertex& offset);
		/*!
			Multiplication operator
			@param mult The factor to multiply the point by
			@return The result of the multiplication of this and the specified factor
		*/
		Vertex operator* (const double& mult) const;
		/*!
			Multiplication operator
			@param mult The x/y/z factors to multiply the point by
			@return The result of the multiplication of this and the specified factor
		*/
		Vertex operator* (const Vertex& mult) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the point by
			@return The result of the multiplication
		*/
		Vertex operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply the point by
			@return The result of the multiplication
		*/
		Vertex operator* (const Matrix4x4& matrix) const;
		/*!
			Multiplication and assignment operator
			@param mult The factor to multiply the point by
			@return A reference to this
		*/
		Vertex& operator*= (const double& mult);
		/*!
			Multiplication and assignment operator
			@param mult The x/y/z factors to multiply the point by
			@return A reference to this
		*/
		Vertex& operator*= (const Vertex& mult);
		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply the point by
			@return A reference to this
		*/
		Vertex& operator*= (const Matrix3x3& matrix);
		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply the point by
			@return A reference to this
		*/
		Vertex& operator*= (const Matrix4x4& matrix);
		/*!
			Division operator
			@param mult The factor to divide the point by
			@return The result of the division of this by the specified factor
		*/
		Vertex operator/ (const double& mult) const;
		/*!
			Division and assignment operator
			@param mult The factor to divide the point by
			@return A reference to this
		*/
		Vertex& operator/= (const double& mult);
		
		// MARK: - Functions (const)
		
		/*!
			Determine if two points are equal in 2D
			@param ref The point to compare with this
			@param prec Precision
			@return True if the points are identical
		*/
		bool isEqual2D(const Vertex& ref, double prec = math::eps) const;
		/*!
			Determine if two points are equal
			@param ref The point to compare with this
			@param prec Precision
			@return True if the points are identical
		*/
		bool isEqual3D(const Vertex& ref, double prec = math::eps) const;
		/*!
			Calculates the 2D distance from this to a specified point
			@param ref The point to compare with this
			@return The 2D distance between the points
		*/
		double lengthFrom2D(const Vertex& ref) const;
		/*!
			Calculates the distant from a specified point to this in 3D
			@param ref A point to calculate the distance from
			@return The distance from the point to this
		*/
		double lengthFrom3D(const Vertex& ref) const;
		/*!
			Return the point with coordinates rounded to the specified level of precision
			@return The point with rounded coordinates (z set to 0.0)
		*/
		Vertex rounded2D(double prec = math::eps) const;
		/*!
			Return the point with coordinates rounded to the specified level of precision
			@return The point with rounded coordinates
		*/
		Vertex rounded3D(double prec = math::eps) const;
		/*!
			Calculates the azimuth angle from this to the specified point
			@param ref A point to calculate the angle to
			@return The azimuth angle from this to the specified point (0 to 2.pi)
		*/
		double azimuthAngleTo(const Vertex& ref) const;
		/*!
			Calculates the altitude angle from this to the specified point
			@param ref A point to calculate the angle to
			@return The altitude angle from this to the specified point (-pi / 2 to pi / 2)
		*/
		double altitudeAngleTo(const Vertex& ref) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Offset a point in the x/y plane by the specified distance and angle
			@param len The distance to move the point
			@param angle The angle on which the point will move
			@return A reference to this
		*/
		Vertex& movePolar(double len, double angle);
		/*!
			Offset a point in the x/y/z plane by the specified distance and azimuth/altitude angle
			@param len The distance to move the point
			@param azim The azimuth angle on which the point will move
			@param alt The altitude angle on which the point will move
			@return A reference to this
		*/
		Vertex& movePolar(double len, double azim, double alt);
	};

}

#endif	//ACTIVE_GEOMETRY_VERTEX
