/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_PLANE
#define ACTIVE_GEOMETRY_PLANE

#include "Active/Geometry/Point.h"
#include "Active/Geometry/Line.h"
#include "Active/Geometry/XPoint.h"
#include "Active/Geometry/Vector3.h"

namespace active::geometry {
	
	class Box;
	class Matrix3x3;
	class Matrix4x4;
	
	/// Class to represent a plane
	class Plane {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Plane>;
			///Shared pointer
		using Shared = std::shared_ptr<Plane>;
			///Optional
		using Option = std::optional<Plane>;
		
		// MARK: - Factory functions
		
		/*!
			Create a new plane from a normal and offset from the origin
			@param offset Distance along normal from the origin to the plane
			@param norm Normal to the plane
			@return An unique_ptr to a plane, or 0 if invalid
		*/
		static Option create(double offset, const Vector3& norm);
		/*!
			Create a new plane from a point and normal
			@param point A point on the plane
			@param norm Normal to the plane
			@return An unique_ptr to a plane, or 0 if invalid
		*/
		static Option create(const Point& point, const Vector3& norm);
		/*!
			Create a plane from three points
			@param p1 First point on the plane
			@param p2 Second point on the plane
			@param p3 Third point on the plane
			@return An unique_ptr to a plane, or 0 if invalid
		*/
		static Option create(const Point& p1, const Point& p2, const Point& p3);
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		Plane();
		/*!
			Constructor
		 	@param norm The plane normal
		*/
		Plane(const Vector3& norm);
		
		// MARK: - Operators
		
		/*!
			Addition operator
			@param offset The amount to offset this by
			@return The result of the addition
		*/
		virtual Plane operator+ (const Point& offset);
		/*!
			Addition and assignment operator
			@param offset The amount to offset this by
			@return A reference to this
		*/
		virtual Plane& operator+= (const Point& offset);
		/*!
			Multiplication operator
			@param mult The factor to multiply this by
			@return The result of the multiplication of this and the specified factor
		*/
		virtual Plane operator* (const double& mult) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply this by
			@return The result of the multiplication
		*/
		virtual Plane operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply this by
			@return The result of the multiplication
		*/
		virtual Plane operator* (const Matrix4x4& matrix) const;
		/*!
			Multiplication and assignment operator
			@param mult The factor to multiply this by
			@return A reference to this
		*/
		virtual Plane& operator*= (const double& mult);
		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply this by
			@return A reference to this
		*/
		virtual Plane& operator*= (const Matrix3x3& matrix);
		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply this by
			@return A reference to this
		*/
		virtual Plane& operator*= (const Matrix4x4& matrix);
		
		// MARK: - Functions (const)
		
		/*!
			Get the normal to the plane
			@return The normal to the plane
		*/
		const Vector3& getNormal() const;
		/*!
			Get the offset from the origin to the plane along the normal
			@return The offset from the origin to the plane along the normal
		*/
		double getOffset() const;
		/*!
			Get the position of a point with respect to the plane
			@param ref The reference point
			@param prec The required precision
			@return The relative point position
		*/
		Position positionOf(const Point& ref, double prec = math::eps) const;
		/*!
			Get the point on the plane closest to the reference point
			@param ref The reference point
			@return The closest point (perpendicular to the plane)
		*/
		Point closestPointTo(const Point& ref) const;
		/*!
			Get the minimum length from a point to the plane
			@param ref The reference point
			@return The minimum distance between the point and this plane
		*/
		double lengthTo(const Point& ref) const;
		/*!
			Get the height of a point projected vertically onto the plane
			@param ref The reference point
			@return The height of the point projected vertically onto the plane
		*/
		double heightAt(const Point& ref, double prec = math::eps) const;
		/*!
			Get the point of intersection between a line and the plane
			@param ref The reference line
			@param prec The required precision
			@return The point of intersection between the line and plane
		*/
		XPoint::Option intersectionWith(const Line& ref, double prec = math::eps) const;
		/*!
			Get the vector describing the intersection of two planes
			@param ref The reference plane
			@param prec The required precision
			@return The line along which the planes intersect (or 0 if they are parallel)
		*/
		Line::Option intersectionWith(const Plane& ref, double prec = math::eps) const;
		/*!
			Get the point at the intersection of three planes
			@param ref1 The first plane
			@param ref2 The second plane
			@return The point at which the planes intersect
		*/
		Point::Option intersectionWith(const Plane& ref1, const Plane& ref2) const;
		/*!
			Determine whether this plane is parallel to another
			@param ref The plane to compare
			@param prec The required precision
			@return True if the planes are parallel
		*/
		bool isParallelTo(const Plane& ref, double prec = math::eps) const;
		/*!
			Determine whether this plane cuts through the specified volume
			@param ref The volume to compare
			@param prec The required precision
			@return True if the plane cuts through the specified volume
		*/
		bool cutsThrough(const Box& ref, double prec = math::eps) const;
	
		// MARK: - Functions (mutating)
		
		/*!
			Set the normal to the plane
			@param vect The normal to the plane
			@return True if the normal was applied (false = normal invalid)
		*/
		bool setNormal(const Vector3& vect);
		/*!
			Set the offset to the plane
			@param offset The offset to the plane
		*/
		void setOffset(double offset) { m_offset = offset; }
		
	private:
		/*!
			Constructor
			@param offset Distance along normal from the origin to the plane
			@param norm Normal to the plane
		*/
		Plane(double offset, const Vector3& norm);
		/*!
			Constructor
			@param point A point on the plane
			@param norm Normal to the plane
		*/
		Plane(const Point& point, const Vector3& norm);
		/*!
			Constructor
			@param p1 First point on the plane
			@param p2 Second point on the plane
			@param p3 Third point on the plane
		*/
		Plane(const Point& p1, const Point& p2, const Point& p3);

		double m_offset;
		Vector3 m_normal;
	};

}

#endif	//ACTIVE_GEOMETRY_PLANE
