/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/3D/Vertex.h"

#include "Active/Utility/MathFunctions.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Matrix4x4.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Geometry/Vector4.h"

#include <cmath>

using namespace active;
using namespace active::primitive;
using namespace active::math;

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Vertex::Vertex() {
	x = y = z = 0.0;
} //Vertex::Vertex


/*--------------------------------------------------------------------
	Constructor

	x: X coordinate
	y: Y coordinate
	z: Z coordinate
  --------------------------------------------------------------------*/
Vertex::Vertex(const double& x, const double& y, const double& z) {
	this->x = x;
	this->y = y;
	this->z = z;
} //Vertex::Vertex


/*--------------------------------------------------------------------
	Constructor
 
	source2D: A 2D point, i.e. only the x/y coordinates are relevant
	z: z coordinate
  --------------------------------------------------------------------*/
Vertex::Vertex(const Vertex& source2D, double z) {
	x = source2D.x;
	y = source2D.y;
	this->z = z;
} //Vertex::Vertex
		
// MARK: - Operators
		
/*--------------------------------------------------------------------
	Less-than operator
 
	ref: The point to compare with this
 
	return: True if this is less than ref
  --------------------------------------------------------------------*/
bool Vertex::operator< (const Vertex& ref) const {
	if (!isEqual(x, ref.x))
		return x < ref.x;
	if (!isEqual(y, ref.y))
		return y < ref.y;
	return isLess(z, ref.z);
} //Vertex::operator<


/*--------------------------------------------------------------------
	Assignment operator

	source: The point to assign

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator= (const Vertex& source) {
	if (&source != this) {
		x = source.x;
		y = source.y;
		z = source.z;
	}
	return *this;
} //Vertex::operator=


/*--------------------------------------------------------------------
	Addition operator

	offset: The offset to add

	return: The sum of this and the specified points
  --------------------------------------------------------------------*/
Vertex Vertex::operator+ (const Vertex& offset) const {
	return Vertex(*this) += offset;
} //Vertex::operator+


/*--------------------------------------------------------------------
	Addition and assignment operator

	offset: The offset to add

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator+= (const Vertex& offset) {
	x += offset.x;
	y += offset.y;
	z += offset.z;
	return *this;
} //Vertex::operator+=


/*--------------------------------------------------------------------
	Subtraction operator

	offset: The offset to subtract

	return: The result of this less the specified point
  --------------------------------------------------------------------*/
Vertex Vertex::operator- (const Vertex& offset) const {
	return Vertex(*this) -= offset;
} //Vertex::operator-


/*--------------------------------------------------------------------
	Subtraction and assignment operator

	offset: The offset to subtract

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator-= (const Vertex& offset) {
	x -= offset.x;
	y -= offset.y;
	z -= offset.z;
	return *this;
} //Vertex::operator-=


/*--------------------------------------------------------------------
	Multiplication operator

	mult: The factor to multiply the point by

	return: The result of the multiplication of this and the
				specified factor
  --------------------------------------------------------------------*/
Vertex Vertex::operator* (const double& mult) const {
	return Vertex(*this) *= mult;
} //Vertex::operator*


/*--------------------------------------------------------------------
	Multiplication operator
 
	mult: The x/y/z factors to multiply the point by
 
	return: The result of the multiplication of this and the specified factor
  --------------------------------------------------------------------*/
Vertex Vertex::operator* (const Vertex& mult) const {
	return Vertex(*this) *= mult;
} //Vertex::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply the point by
	
	return: The result of the multiplication
  --------------------------------------------------------------------*/
Vertex Vertex::operator* (const geometry::Matrix3x3& matrix) const {
	return geometry::Vector3(*this) * matrix;
} //Vertex::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply the point by
	
	return: The result of the multiplication
  --------------------------------------------------------------------*/
Vertex Vertex::operator* (const geometry::Matrix4x4& matrix) const {
	return geometry::Vector4(*this) * matrix;
} //Vertex::operator*


/*--------------------------------------------------------------------
	Multiplication and assignment operator

	mult: The factor to multiply the point by

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator*= (const double& mult) {
	x *= mult;
	y *= mult;
	z *= mult;
	return *this;
} //Vertex::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
 
	mult: The x/y/z factors to multiply the point by
 
	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator*= (const Vertex& mult) {
	x *= mult.x;
	y *= mult.y;
	z *= mult.z;
	return *this;
} //Vertex::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply the point by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator*= (const geometry::Matrix3x3& matrix) {
	return (*this = geometry::Vector3(*this) * matrix);
} //Vertex::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply the point by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator*= (const geometry::Matrix4x4& matrix) {
	return (*this = geometry::Vector4(*this) * matrix);
} //Vertex::operator*=


/*--------------------------------------------------------------------
	Division operator

	mult: The factor to divide the point by

	return: The result of the division of this by the specified factor
  --------------------------------------------------------------------*/
Vertex Vertex::operator/ (const double& mult) const {
	return Vertex(*this) /= mult;
} //Vertex::operator/


/*--------------------------------------------------------------------
	Division and assignment operator

	mult: The factor to divide the point by

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::operator/= (const double& mult) {
	x /= mult;
	y /= mult;
	z /= mult;
	return *this;
} //Vertex::operator/=
		
// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if two points are equal in 2D

	ref: The point to compare with this

	return: True if the points are identical
  --------------------------------------------------------------------*/
bool Vertex::isEqual2D(const Vertex& ref, double prec) const {
	return (isEqual(x, ref.x, prec) && isEqual(y, ref.y, prec));
} //Vertex::isEqual2D


/*--------------------------------------------------------------------
	Determine if two points are equal

	ref: The point to compare with this

	return: True if the points are identical
  --------------------------------------------------------------------*/
bool Vertex::isEqual3D(const Vertex& ref, double prec) const {
	return (isZero(lengthFrom3D(ref), prec));
} //Vertex::isEqual3D


/*--------------------------------------------------------------------
	Calculates the 2D distant from this to a specified point

	ref: A point to calculate the distance to

	return: The 2D distance between the points
  --------------------------------------------------------------------*/
double Vertex::lengthFrom2D(const Vertex& ref) const {
	return sqrt(pow((x - ref.x), 2.0) + pow((y - ref.y), 2.0));
} //Vertex::lengthFrom2D


/*--------------------------------------------------------------------
	Calculates the distant from a specified point to this in 3D

	ref: A point to calculate the distance from

	return: The distance from the point to this
  --------------------------------------------------------------------*/
double Vertex::lengthFrom3D(const Vertex& ref) const {
	return sqrt(pow((x - ref.x), 2.0) + pow((y - ref.y), 2.0) + pow((z - ref.z), 2.0));
} //Vertex::lengthTo3D


/*--------------------------------------------------------------------
	Return the point with coordinates rounded to the specified level of precision
 
	return: The point with rounded coordinates (z set to 0.0)
  --------------------------------------------------------------------*/
Vertex Vertex::rounded2D(double prec) const {
	return Vertex(round(x, prec), round(y, prec), 0.0);
} //Vertex::rounded2D


/*--------------------------------------------------------------------
	Return the point with coordinates rounded to the specified level of precision
 
	return: The point with rounded coordinates
  --------------------------------------------------------------------*/
Vertex Vertex::rounded3D(double prec) const {
	return Vertex(round(x, prec), round(y, prec), round(z, prec));
} //Vertex::rounded3D


/*--------------------------------------------------------------------
	Calculates the azimuth angle from this to the
				specified point

	ref: A point to calculate the angle to

	return: The azimuth angle from this to the specified point (0.0 to 2.pi)
  --------------------------------------------------------------------*/
double Vertex::azimuthAngleTo(const Vertex& ref) const {
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
} //Vertex::azimuthAngleTo


/*--------------------------------------------------------------------
	Calculates the altitude angle from this to the
				specified point

	ref: A point to calculate the angle to

	return: The altitude angle from this to the specified point (-pi / 2 to pi / 2)
  --------------------------------------------------------------------*/
double Vertex::altitudeAngleTo(const Vertex& ref) const {
	double dx = lengthFrom2D(ref),
			dy = ref.z - z;
	if (isZero(dx)) {
		if (isZero(dy))
			return 0.0;
		return (isGreaterZero(dy)) ? pi / 2.0 : -pi / 2.0;
	}
	return atan(dy / dx);
} //Vertex::azimuthAngleTo
		
// MARK: - Functions (mutating)
		
/*--------------------------------------------------------------------
	Offset a point in the x/y plane by the specified
				distance and angle

	len: The distance to move the point
	angle		The angle on which the point will move

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::movePolar(double len, double angle) {
	x += len * cos(angle);
	y += len * sin(angle);
	return *this;
} //Vertex::movePolar


/*--------------------------------------------------------------------
	Offset a point in the x/y/z plane by the specified distance and
	azimuth/altitude angle

	len: The distance to move the point
	azim: The azimuth angle on which the point will move
	alt: The altitude angle on which the point will move

	return: A reference to this
  --------------------------------------------------------------------*/
Vertex& Vertex::movePolar(double len, double azim, double alt) {
	double dist = len * fabs(cos(alt));
	x += dist * cos(azim);
	y += dist * sin(azim);
	z += len * sin(alt);
	return *this;
} //Vertex::movePolar
