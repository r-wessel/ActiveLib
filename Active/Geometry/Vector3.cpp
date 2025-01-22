/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Vector3.h"

#include "Active/Geometry/Line.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Matrix4x4.h"
#include "Active/Geometry/Vector4.h"
#include "Active/Primitives/3D/Vertex.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Vector3::Vector3() {
	for (auto i = 0; i < 3; ++i)
		m_vector[i] = 0;
} //Vector3::Vector3


/*--------------------------------------------------------------------
	Constructor
	
	x: X coefficient
	y: Y coefficient
	z: Z coefficient
  --------------------------------------------------------------------*/
Vector3::Vector3(double x, double y, double z) {
	m_vector[0] = x;
	m_vector[1] = y;
	m_vector[2] = z;
} //Vector3::Vector3


/*--------------------------------------------------------------------
	Constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector3::Vector3(const Point& source) {
	*this = source;
} //Vector3::Vector3


/*--------------------------------------------------------------------
	Constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector3::Vector3(const Line& source) {
	*this = source;
} //Vector3::Vector3


/*--------------------------------------------------------------------
	Copy constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector3::Vector3(const Vector3& source) {
	*this = source;
} //Vector3::Vector3

// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment operator
	
	source: The point to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector3& Vector3::operator=(const Point& source) {
	m_vector[0] = source.x;
	m_vector[1] = source.y;
	m_vector[2] = source.z;
	return *this;
} //Vector3::operator=


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The vertex to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector3& Vector3::operator=(const primitive::Vertex& source) {
	m_vector[0] = source.x;
	m_vector[1] = source.y;
	m_vector[2] = source.z;
	return *this;
} //Vector3::operator=



/*--------------------------------------------------------------------
	Assignment operator
	
	source: The line to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector3& Vector3::operator=(const Line& source) {
	*this = source.end - source.origin;
	return *this;
} //Vector3::operator=


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The vector to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector3& Vector3::operator=(const Vector3& source) {
	if (this != &source) {
		m_vector[0] = source.m_vector[0];
		m_vector[1] = source.m_vector[1];
		m_vector[2] = source.m_vector[2];
	}
	return *this;
} //Vector3::operator=


/*--------------------------------------------------------------------
	Addition operator
	
	vect: The vector to add
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector3 Vector3::operator+(const Vector3& vect) const {
	return Vector3(*this) += vect;
} //Vector3::operator+


/*--------------------------------------------------------------------
	Addition and assignment operator
	
	vect: The vector to add
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector3& Vector3::operator+=(const Vector3& vect) {
	m_vector[0] += vect.m_vector[0];
	m_vector[1] += vect.m_vector[1];
	m_vector[2] += vect.m_vector[2];
	return *this;
} //Vector3::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	vect: The vector to subtract
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector3 Vector3::operator-(const Vector3& vect) const {
	return Vector3(*this) -= vect;
} //Vector3::operator-


/*--------------------------------------------------------------------
	Subtraction and assignment operator
	
	vect: The vector to subtract
	return: A reference to this
  --------------------------------------------------------------------*/
Vector3& Vector3::operator-=(const Vector3& vect) {
	m_vector[0] -= vect.m_vector[0];
	m_vector[1] -= vect.m_vector[1];
	m_vector[2] -= vect.m_vector[2];
	return *this;
} //Vector3::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	vect: The vector to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector3 Vector3::operator*(double scale) const {
	return Vector3(*this) *= scale;
} //Vector3::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector3 Vector3::operator*(const Matrix3x3& matrix) const {
	return Vector3(*this) *= matrix;
} //Vector3::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector3 Vector3::operator*(const Matrix4x4& matrix) const {
	return Vector3{Point{Vector4(*this) *= matrix}};
} //Vector3::operator*


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	vect: The vector to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector3& Vector3::operator*=(double scale) {
	m_vector[0] *= scale;
	m_vector[1] *= scale;
	m_vector[2] *= scale;
	return *this;
} //Vector3::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply by
	
	return: A reference to this
--------------------------------------------------------------------*/
Vector3& Vector3::operator*=(const Matrix3x3& matrix) {
	Vector3 result;
	for (auto row = 0; row < 3; ++row) {
		for (auto col = 0; col < 3; ++col)
			result.m_vector[row] += m_vector[col] * matrix(row, col);
	}
	*this = result;
	return *this;
} //Vector3::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply by
	
	return: A reference to this
--------------------------------------------------------------------*/
Vector3& Vector3::operator*=(const Matrix4x4& matrix) {
	return (*this = Point{(Vector4(*this) *= matrix)});
} //Vector3::operator*=


/*--------------------------------------------------------------------
	Conversion operator
	
	return: A point derived from this vector
  --------------------------------------------------------------------*/
Vector3::operator Point() const {
	return Point(m_vector[0], m_vector[1], m_vector[2]);
} //Vector3::operator Point


/*--------------------------------------------------------------------
	Conversion operator
	
	return: A vertex derived from this vector
  --------------------------------------------------------------------*/
Vector3::operator primitive::Vertex() const {
	return primitive::Vertex(m_vector[0], m_vector[1], m_vector[2]);
} //Vector3::operator Vertex

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine of a vector is parallel to this
	
	ref: A vector
	prec: The precision of the test
	
	return: True if the vector is parallel to this
  --------------------------------------------------------------------*/
bool Vector3::isParallelTo(const Vector3& ref, double prec) const {
	return (isEmpty(prec) || ref.isEmpty(prec)) ? false : vectorProduct(ref).isEmpty(prec);
} //Vector3::isParallelTo


/*--------------------------------------------------------------------
	Determine if a vector is perpendicular to this
	
	ref: A vector
	prec: The precision of the test
	
	return: True if the vector is perpendicular to this
  --------------------------------------------------------------------*/
bool Vector3::isPerpendicularTo(const Vector3& ref, double prec) const {
	return (isEmpty(prec) || ref.isEmpty(prec)) ? false : isZero(dotProduct(ref), prec);
} //Vector3::isPerpendicularTo


/*--------------------------------------------------------------------
	Determine if a vector has the same sense as this
	
	ref: A vector
	prec: The precision of the test
	
	return: True if the vector has the same sense as this
  --------------------------------------------------------------------*/
bool Vector3::isSameSense(const Vector3& ref, double prec) const {
	for (auto i = 0; i < 3; ++i) {
		if (isLessZero(m_vector[i] * ref.m_vector[i], prec))
			return false;
	}
	return true;
} //Vector3::isSameSense


/*--------------------------------------------------------------------
	Determine if the vector is empty (all null values)
	
	prec: The precision of the test
	
	return: True if the vector is empty
  --------------------------------------------------------------------*/
bool Vector3::isEmpty(double prec) const {
	return (isZero(m_vector[0], prec) && isZero(m_vector[1], prec) && isZero(m_vector[2], prec));
} //Vector3::isEmpty


/*--------------------------------------------------------------------
	Get the magnitude of the vector
	
	return: The magnitude of the vector
  --------------------------------------------------------------------*/
double Vector3::magnitude() const {
	return sqrt((m_vector[0] * m_vector[0]) + (m_vector[1] * m_vector[1]) +
			(m_vector[2] * m_vector[2]));
} //Vector3::magnitude


/*--------------------------------------------------------------------
	Get the dot product of two vectors
	
	ref: A vector
	
	return: The dot product
  --------------------------------------------------------------------*/
double Vector3::dotProduct(const Vector3& ref) const {
	return (m_vector[0] * ref.m_vector[0]) + (m_vector[1] * ref.m_vector[1]) +
			(m_vector[2] * ref.m_vector[2]);
} //Vector3::dotProduct


/*--------------------------------------------------------------------
	Get the vectorial product of two vectors (a perpendicular vector)
	
	ref: A vector
	
	return: The vectorial product
  --------------------------------------------------------------------*/
Vector3 Vector3::vectorProduct(const Vector3& ref) const {
	return Vector3((m_vector[1] * ref.m_vector[2]) - (m_vector[2] * ref.m_vector[1]),
			(m_vector[2] * ref.m_vector[0]) - (m_vector[0] * ref.m_vector[2]),
			(m_vector[0] * ref.m_vector[1]) - (m_vector[1] * ref.m_vector[0]));
} //Vector3::vectorProduct


/*--------------------------------------------------------------------
	Get the normalised vector
	
	return: The normalised vector
  --------------------------------------------------------------------*/
Vector3 Vector3::normalised() const {
	Vector3 result(*this);
	double mag = result.magnitude();
	if (mag > 0) {
		result.m_vector[0] /= mag;
		result.m_vector[1] /= mag;
		result.m_vector[2] /= mag;
	}
	return result;
} //Vector3::normalised


/*--------------------------------------------------------------------
	Get the azimuth angle of the vector
	
	return: The azimuth angle
  --------------------------------------------------------------------*/
double Vector3::azimuthAngle() const {
	return Point().azimuthAngleTo(Point(*this));
} //Vector3::azimuthAngle


/*--------------------------------------------------------------------
	Get the altitude angle of the vector
	
	return: The altitude angle
  --------------------------------------------------------------------*/
double Vector3::altitudeAngle() const {
	return Point().altitudeAngleTo(Point(*this));
} //Vector3::altitudeAngle


/*--------------------------------------------------------------------
	Get the angle between a vector and this
	
	ref: A vector
	
	return: The angle between the vector and this
  --------------------------------------------------------------------*/
double Vector3::angleTo(const Vector3& ref) const {
	double mag1 = this->magnitude(), mag2 = ref.magnitude();
	return (isZero(mag1) || isZero(mag2)) ? 0 : acos(this->dotProduct(ref) / (mag1 * mag2));
} //Vector3::angleTo
