/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Point.h"

#include "Active/Utility/MathFunctions.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Matrix4x4.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Geometry/Vector4.h"

#include <cmath>

using namespace active::geometry;
using namespace active::math;

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Point::Point() {
	x = y = z = 0.0;
} //Point::Point


/*--------------------------------------------------------------------
	Constructor

	x: X coordinate
	y: Y coordinate
	z: Z coordinate
  --------------------------------------------------------------------*/
Point::Point(double x, double y, double z) {
	this->x = x;
	this->y = y;
	this->z = z;
} //Point::Point


/*--------------------------------------------------------------------
	Constructor
 
	source2D: A 2D point, i.e. only the x/y coordinates are relevant
	z: z coordinate
  --------------------------------------------------------------------*/
Point::Point(const Point& source2D, double z) {
	x = source2D.x;
	y = source2D.y;
	this->z = z;
} //Point::Point
		
// MARK: - Operators
		
/*--------------------------------------------------------------------
	Less-than operator
 
	ref: The point to compare with this
 
	return: True if this is less than ref
  --------------------------------------------------------------------*/
bool Point::operator< (const Point& ref) const {
	if (!isEqual(x, ref.x))
		return x < ref.x;
	if (!isEqual(y, ref.y))
		return y < ref.y;
	return isLess(z, ref.z);
} //Point::operator<


/*--------------------------------------------------------------------
	Assignment operator

	source: The point to assign

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator= (const Point& source) {
	if (&source != this) {
		x = source.x;
		y = source.y;
		z = source.z;
	}
	return *this;
} //Point::operator=


/*--------------------------------------------------------------------
	Addition operator

	offset: The offset to add

	return: The sum of this and the specified points
  --------------------------------------------------------------------*/
Point Point::operator+ (const Point& offset) const {
	return Point(*this) += offset;
} //Point::operator+


/*--------------------------------------------------------------------
	Addition and assignment operator

	offset: The offset to add

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator+= (const Point& offset) {
	x += offset.x;
	y += offset.y;
	z += offset.z;
	return *this;
} //Point::operator+=


/*--------------------------------------------------------------------
	Subtraction operator

	offset: The offset to subtract

	return: The result of this less the specified point
  --------------------------------------------------------------------*/
Point Point::operator- (const Point& offset) const {
	return Point(*this) -= offset;
} //Point::operator-


/*--------------------------------------------------------------------
	Subtraction and assignment operator

	offset: The offset to subtract

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator-= (const Point& offset) {
	x -= offset.x;
	y -= offset.y;
	z -= offset.z;
	return *this;
} //Point::operator-=


/*--------------------------------------------------------------------
	Multiplication operator

	mult: The factor to multiply the point by

	return: The result of the multiplication of this and the
				specified factor
  --------------------------------------------------------------------*/
Point Point::operator* (double mult) const {
	return Point(*this) *= mult;
} //Point::operator*


/*--------------------------------------------------------------------
	Multiplication operator
 
	mult: The x/y/z factors to multiply the point by
 
	return: The result of the multiplication of this and the specified factor
  --------------------------------------------------------------------*/
Point Point::operator* (const Point& mult) const {
	return Point(*this) *= mult;
} //Point::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply the point by
	
	return: The result of the multiplication
  --------------------------------------------------------------------*/
Point Point::operator* (const Matrix3x3& matrix) const {
	return Vector3(*this) * matrix;
} //Point::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply the point by
	
	return: The result of the multiplication
  --------------------------------------------------------------------*/
Point Point::operator* (const Matrix4x4& matrix) const {
	return Vector4(*this) * matrix;
} //Point::operator*


/*--------------------------------------------------------------------
	Multiplication and assignment operator

	mult: The factor to multiply the point by

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator*= (double mult) {
	x *= mult;
	y *= mult;
	z *= mult;
	return *this;
} //Point::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
 
	mult: The x/y/z factors to multiply the point by
 
	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator*= (const Point& mult) {
	x *= mult.x;
	y *= mult.y;
	z *= mult.z;
	return *this;
} //Point::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply the point by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator*= (const Matrix3x3& matrix) {
	return (*this = Vector3(*this) * matrix);
} //Point::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply the point by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator*= (const Matrix4x4& matrix) {
	return (*this = Vector4(*this) * matrix);
} //Point::operator*=


/*--------------------------------------------------------------------
	Division operator

	mult: The factor to divide the point by

	return: The result of the division of this by the specified factor
  --------------------------------------------------------------------*/
Point Point::operator/ (double mult) const {
	return Point(*this) /= mult;
} //Point::operator/


/*--------------------------------------------------------------------
	Division and assignment operator

	mult: The factor to divide the point by

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::operator/= (double mult) {
	x /= mult;
	y /= mult;
	z /= mult;
	return *this;
} //Point::operator/=
		
// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if two points are equal in 2D

	ref: The point to compare with this

	return: True if the points are identical
  --------------------------------------------------------------------*/
bool Point::isEqual2D(const Point& ref, double prec) const {
	return (isEqual(x, ref.x, prec) && isEqual(y, ref.y, prec));
} //Point::isEqual2D


/*--------------------------------------------------------------------
	Determine if two points are equal

	ref: The point to compare with this

	return: True if the points are identical
  --------------------------------------------------------------------*/
bool Point::isEqual3D(const Point& ref, double prec) const {
	return (isZero(lengthFrom3D(ref), prec));
} //Point::isEqual3D


/*--------------------------------------------------------------------
	Calculates the 2D distant from this to a specified point

	ref: A point to calculate the distance to

	return: The 2D distance between the points
  --------------------------------------------------------------------*/
double Point::lengthFrom2D(const Point& ref) const {
	return sqrt(pow((x - ref.x), 2.0) + pow((y - ref.y), 2.0));
} //Point::lengthFrom2D


/*--------------------------------------------------------------------
	Calculates the distant from a specified point to this in 3D

	ref: A point to calculate the distance from

	return: The distance from the point to this
  --------------------------------------------------------------------*/
double Point::lengthFrom3D(const Point& ref) const {
	return sqrt(pow((x - ref.x), 2.0) + pow((y - ref.y), 2.0) + pow((z - ref.z), 2.0));
} //Point::lengthTo3D


/*--------------------------------------------------------------------
	Return the point with coordinates rounded to the specified level of precision
 
	return: The point with rounded coordinates (z set to 0.0)
  --------------------------------------------------------------------*/
Point Point::rounded2D(double prec) const {
	return Point(round(x, prec), round(y, prec), 0.0);
} //Point::rounded2D


/*--------------------------------------------------------------------
	Return the point with coordinates rounded to the specified level of precision
 
	return: The point with rounded coordinates
  --------------------------------------------------------------------*/
Point Point::rounded3D(double prec) const {
	return Point(round(x, prec), round(y, prec), round(z, prec));
} //Point::rounded3D


/*--------------------------------------------------------------------
	Calculates the azimuth angle from this to the
				specified point

	ref: A point to calculate the angle to

	return: The azimuth angle from this to the specified point (0.0 to 2.pi)
  --------------------------------------------------------------------*/
double Point::azimuthAngleTo(const Point& ref) const {
	double dx = ref.x - x, dy = ref.y - y;
	if (isZero(dx)) {
		if (isZero(dy))
			return 0.0;
		return (isGreaterZero(dy)) ? pi * 0.5 : pi * 1.5;
	}
	double azim = atan(dy / dx);
	if (dx < 0.0)
		azim += pi;
	else if (azim < 0.0)
		azim += (2.0 * pi);
	return azim;
} //Point::azimuthAngleTo


/*--------------------------------------------------------------------
	Calculates the altitude angle from this to the
				specified point

	ref: A point to calculate the angle to

	return: The altitude angle from this to the specified point (-pi / 2 to pi / 2)
  --------------------------------------------------------------------*/
double Point::altitudeAngleTo(const Point& ref) const {
	double dx = lengthFrom2D(ref),
			dy = ref.z - z;
	if (isZero(dx)) {
		if (isZero(dy))
			return 0.0;
		return (isGreaterZero(dy)) ? pi / 2.0 : -pi / 2.0;
	}
	return atan(dy / dx);
} //Point::azimuthAngleTo
		
// MARK: - Functions (mutating)
		
/*--------------------------------------------------------------------
	Offset a point in the x/y plane by the specified
				distance and angle

	len: The distance to move the point
	angle		The angle on which the point will move

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::movePolar(double len, double angle) {
	x += len * cos(angle);
	y += len * sin(angle);
	return *this;
} //Point::movePolar


/*--------------------------------------------------------------------
	Offset a point in the x/y/z plane by the specified distance and
	azimuth/altitude angle

	len: The distance to move the point
	azim: The azimuth angle on which the point will move
	alt: The altitude angle on which the point will move

	return: A reference to this
  --------------------------------------------------------------------*/
Point& Point::movePolar(double len, double azim, double alt) {
	double dist = len * fabs(cos(alt));
	x += dist * cos(azim);
	y += dist * sin(azim);
	z += len * sin(alt);
	return *this;
} //Point::movePolar
