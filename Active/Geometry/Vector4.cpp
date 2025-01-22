/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Vector4.h"

#include "Active/Geometry/Line.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Geometry/Matrix4x4.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Primitives/3D/Vertex.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Vector4::Vector4() {
	for (auto i = 0; i < 4; ++i)
		m_vector[i] = 0;
} //Vector4::Vector4


/*--------------------------------------------------------------------
	Constructor
	
	x: X coefficient
	y: Y coefficient
	z: Z coefficient
  --------------------------------------------------------------------*/
Vector4::Vector4(double x, double y, double z) {
	m_vector[0] = x;
	m_vector[1] = y;
	m_vector[2] = z;
} //Vector4::Vector4


/*--------------------------------------------------------------------
	Constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector4::Vector4(const Point& source) {
	*this = source;
} //Vector4::Vector4


/*--------------------------------------------------------------------
	Constructor
	
	source: The vertex to copy
  --------------------------------------------------------------------*/
Vector4::Vector4(const primitive::Vertex& source) {
	*this = source;
} //Vector4::Vector4


/*--------------------------------------------------------------------
	Constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector4::Vector4(const Line& source) {
	*this = source;
} //Vector4::Vector4


/*--------------------------------------------------------------------
	Copy constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector4::Vector4(const Vector3& source) {
	*this = source;
} //Vector4::Vector4


/*--------------------------------------------------------------------
	Copy constructor
	
	source: The object to copy
  --------------------------------------------------------------------*/
Vector4::Vector4(const Vector4& source) {
	*this = source;
} //Vector4::Vector4

// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment operator
	
	source: The point to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator=(const Point& source) {
	m_vector[0] = source.x;
	m_vector[1] = source.y;
	m_vector[2] = source.z;
	m_vector[3] = 1;
	return *this;
} //Vector4::operator=


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The vertex to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator=(const primitive::Vertex& source) {
	m_vector[0] = source.x;
	m_vector[1] = source.y;
	m_vector[2] = source.z;
	m_vector[3] = 1;
	return *this;
} //Vector4::operator=


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The line to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator=(const Line& source) {
	*this = source.end - source.origin;
	return *this;
} //Vector4::operator=


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The vector to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator=(const Vector3& source) {
	m_vector[0] = source[0];
	m_vector[1] = source[1];
	m_vector[2] = source[2];
	m_vector[3] = 1;
	return *this;
} //Vector4::operator=


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The vector to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator=(const Vector4& source) {
	if (this != &source) {
		m_vector[0] = source.m_vector[0];
		m_vector[1] = source.m_vector[1];
		m_vector[2] = source.m_vector[2];
		m_vector[3] = source.m_vector[3];
	}
	return *this;
} //Vector4::operator=


/*--------------------------------------------------------------------
	Addition operator
	
	vect: The vector to add
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector4 Vector4::operator+(const Vector4& vect) const {
	return Vector4(*this) += vect;
} //Vector4::operator+


/*--------------------------------------------------------------------
	Addition and assignment operator
	
	vect: The vector to add
	
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator+=(const Vector4& vect) {
	m_vector[0] += vect.m_vector[0];
	m_vector[1] += vect.m_vector[1];
	m_vector[2] += vect.m_vector[2];
	return *this;
} //Vector4::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	vect: The vector to subtract
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector4 Vector4::operator-(const Vector4& vect) const {
	return Vector4(*this) -= vect;
} //Vector4::operator-


/*--------------------------------------------------------------------
	Subtraction and assignment operator
	
	vect: The vector to subtract
	return: A reference to this
  --------------------------------------------------------------------*/
Vector4& Vector4::operator-=(const Vector4& vect) {
	m_vector[0] -= vect.m_vector[0];
	m_vector[1] -= vect.m_vector[1];
	m_vector[2] -= vect.m_vector[2];
	return *this;
} //Vector4::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	vect: The vector to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector4 Vector4::operator*(double scale) const {
	return Vector4(*this) *= scale;
} //Vector4::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector4 Vector4::operator*(const Matrix4x4& matrix) const {
	return Vector4(*this) *= matrix;
} //Vector4::operator*


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	vect: The vector to multiply by
	
	return: The resultant vector
  --------------------------------------------------------------------*/
Vector4& Vector4::operator*=(double scale) {
	m_vector[0] *= scale;
	m_vector[1] *= scale;
	m_vector[2] *= scale;
	m_vector[3] *= scale;
	return *this;
} //Vector4::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply by
	
	return: A reference to this
--------------------------------------------------------------------*/
Vector4& Vector4::operator*=(const Matrix4x4& matrix) {
	Vector4 result;
	for (auto row = 0; row < 4; ++row) {
		for (auto col = 0; col < 4; ++col)
			result.m_vector[row] += m_vector[col] * matrix(row, col);
	}
	*this = result;
	return *this;
} //Vector4::operator*=


/*--------------------------------------------------------------------
	Converion operator
	
	return: A point derived from this vector
  --------------------------------------------------------------------*/
Vector4::operator Point() const {
	return Point(m_vector[0], m_vector[1], m_vector[2]);
} //Vector4::operator Point


/*--------------------------------------------------------------------
	Converion operator
	
	return: A vertex derived from this vector
  --------------------------------------------------------------------*/
Vector4::operator primitive::Vertex() const {
	return primitive::Vertex(m_vector[0], m_vector[1], m_vector[2]);
} //Vector4::operator Vertex

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine of a vector is parallel to this
	
	ref: A vector
	prec: The precision of the test
	
	return: True if the vector is parallel to this
  --------------------------------------------------------------------*/
bool Vector4::isParallelTo(const Vector4& ref, double prec) const {
	return vectorProduct(ref).isEmpty(prec);
} //Vector4::isParallelTo


/*--------------------------------------------------------------------
	Determine if a vector is perpendicular to this
	
	ref: A vector
	prec: The precision of the test
	
	return: True if the vector is perpendicular to this
  --------------------------------------------------------------------*/
bool Vector4::isPerpendicularTo(const Vector4& ref, double prec) const {
	return isZero(dotProduct(ref), prec);
} //Vector4::isPerpendicularTo


/*--------------------------------------------------------------------
	Determine if a vector has the same sense as this
	
	ref: A vector
	prec: The precision of the test
	
	return: True if the vector has the same sense as this
  --------------------------------------------------------------------*/
bool Vector4::isSameSense(const Vector4& ref, double prec) const {
	for (auto i = 0; i < 4; ++i) {
		if (isLessZero(m_vector[i] * ref.m_vector[i], prec))
			return false;
	}
	return true;
} //Vector4::isSameSense


/*--------------------------------------------------------------------
	Determine if the vector is empty (all null values)
	
	prec: The precision of the test
	
	return: True if the vector is empty
  --------------------------------------------------------------------*/
bool Vector4::isEmpty(double prec) const {
	return (isZero(m_vector[0], prec) && isZero(m_vector[1], prec) &&
			isZero(m_vector[2], prec));
} //Vector4::isEmpty


/*--------------------------------------------------------------------
	Get the magnitude of the vector
	
	return: The magnitude of the vector
  --------------------------------------------------------------------*/
double Vector4::magnitude() const {
	return sqrt((m_vector[0] * m_vector[0]) + (m_vector[1] * m_vector[1]) +
			(m_vector[2] * m_vector[2]));
} //Vector4::magnitude


/*--------------------------------------------------------------------
	Get the dot product of two vectors
	
	ref: A vector
	
	return: The dot product
  --------------------------------------------------------------------*/
double Vector4::dotProduct(const Vector4& ref) const {
	return (m_vector[0] * ref.m_vector[0]) + (m_vector[1] * ref.m_vector[1]) +
			(m_vector[2] * ref.m_vector[2]);
} //Vector4::dotProduct


/*--------------------------------------------------------------------
	Get the vectorial product of two vectors (a perpendicular vector)
	
	ref: A vector
	
	return: The vectorial product
  --------------------------------------------------------------------*/
Vector4 Vector4::vectorProduct(const Vector4& ref) const {
	return Vector4((m_vector[1] * ref.m_vector[2]) - (m_vector[2] * ref.m_vector[1]),
			(m_vector[2] * ref.m_vector[0]) - (m_vector[0] * ref.m_vector[2]),
			(m_vector[0] * ref.m_vector[1]) - (m_vector[1] * ref.m_vector[0]));
} //Vector4::vectorProduct


/*--------------------------------------------------------------------
	Get the normalised vector
	
	return: The normalised vector
  --------------------------------------------------------------------*/
Vector4 Vector4::normalised() const {
	Vector4 result(*this);
	double mag = result.magnitude();
	if (mag > 0) {
		result.m_vector[0] /= mag;
		result.m_vector[1] /= mag;
		result.m_vector[2] /= mag;
		result.m_vector[3] /= mag;
	}
	return result;
} //Vector4::normalised


/*--------------------------------------------------------------------
	Get the angle between a vector and this
	
	ref: A vector
	
	return: The angle between the vector and this
  --------------------------------------------------------------------*/
double Vector4::angleTo(const Vector4& ref) const {
	double mag1 = this->magnitude(), mag2 = ref.magnitude();
	return (isZero(mag1) || isZero(mag2)) ? 0 : acos(this->dotProduct(ref) / (mag1 * mag2));
} //Vector4::angleTo
