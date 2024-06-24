/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Matrix3x3.h"

#include "Active/Utility/MathFunctions.h"

using namespace active::geometry;
using namespace active::math;

// MARK: Factory functions

/*--------------------------------------------------------------------
	Create a rotation matrix around the x axis
	
	angle: The rotation angle
	
	return: The requested rotation matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::createXRotate(double angle) {
	return Matrix3x3(	1.0, 0.0, 0.0,
						0.0, cos(angle), -sin(angle),
						0.0, sin(angle), cos(angle));
} //Matrix3x3::createXRotate


/*--------------------------------------------------------------------
	Create a rotation matrix around the y axis
	
	angle: The rotation angle
	
	return: The requested rotation matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::createYRotate(double angle) {
	return Matrix3x3(	cos(angle), 0.0, sin(angle),
						0.0, 1.0, 0.0,
						-sin(angle), 0, cos(angle));
} //Matrix3x3::createYRotate


/*--------------------------------------------------------------------
	Create a rotation matrix around the z axis
	
	angle: The rotation angle
	
	return: The requested rotation matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::createZRotate(double angle) {
	return Matrix3x3(	cos(angle), -sin(angle), 0.0,
						sin(angle), cos(angle), 0.0,
						0.0, 0.0, 1.0);
} //Matrix3x3::createZRotate


/*--------------------------------------------------------------------
	Create a scaling matrix
	
	x: The scale in the x axis
	y: The scale in the y axis
	z: The scale in the z axis
	
	return: The requested scaling matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::createScale(double x, double y, double z) {
	Matrix3x3 result;
	result.m_matrix[0][0] = x;
	result.m_matrix[1][1] = y;
	result.m_matrix[2][2] = z;
	return result;
} //Matrix3x3::createScale


/*--------------------------------------------------------------------
	Create a translation matrix
	
	x: The offset in the x axis
	y: The offset in the y axis
	z: The offset in the z axis
	
	return: The requested transformation matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::createTranslate(double x, double y, double z) {
	Matrix3x3 result;
	result.m_matrix[0][0] = 1;
	result.m_matrix[1][1] = 1;
	result.m_matrix[2][2] = 1;
	result.m_matrix[2][0] = x;
	result.m_matrix[2][1] = y;
	result.m_matrix[2][2] = z;
	return result;
} //Matrix3x3::createTranslate


/*--------------------------------------------------------------------
	Create an identity matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::createIdentity() {
	Matrix3x3 result;
	result.m_matrix[0][0] = 1;
	result.m_matrix[1][1] = 1;
	result.m_matrix[2][2] = 1;
	return result;
} //Matrix3x3::createIdentity
		
// MARK: Constructors

/*!
	Default constructor
*/
Matrix3x3::Matrix3x3() : m_matrix{} {
} //Matrix3x3::Matrix3x3


/*--------------------------------------------------------------------
	Constructor

	a1-c3: Matrix values
  --------------------------------------------------------------------*/
Matrix3x3::Matrix3x3(	double a1, double b1, double c1,
						double a2, double b2, double c2,
						double a3, double b3, double c3) {
	m_matrix[0][0] = a1;
	m_matrix[0][1] = b1;
	m_matrix[0][2] = c1;

	m_matrix[1][0] = a2;
	m_matrix[1][1] = b2;
	m_matrix[1][2] = c2;

	m_matrix[2][0] = a3;
	m_matrix[2][1] = b3;
	m_matrix[2][2] = c3;
} //Matrix3x3::Matrix3x3
		
// MARK: Operators

/*--------------------------------------------------------------------
	Equality operator
	
	ref: The object to compare
	
	return: True if the objects are identical
  --------------------------------------------------------------------*/
bool Matrix3x3::operator==(const Matrix3x3& ref) const {
	for (auto i = 0; i < 3; ++i) {
		for (auto j = 0; j < 3; ++j) {
			if (!isEqual(m_matrix[i][j], ref.m_matrix[i][j]))
				return false;
		}
	}
	return true;
} //Matrix3x3::operator==


/*--------------------------------------------------------------------
	Inequality operator
	
	ref: The object to compare
	
	return: True if the objects are not identical
  --------------------------------------------------------------------*/
bool Matrix3x3::operator!=(const Matrix3x3& ref) const {
   return !(*this == ref);
} //Matrix3x3::operator!=


/*--------------------------------------------------------------------
	Multiplication operator
	
	ref: The matrix to multiply this by
	
	return: The result
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& ref) const {
	Matrix3x3 matrix;
	for (auto i = 0; i < 3; ++i) {
		for (auto j = 0; j < 3; ++j) {
			double ab = 0;
			for (auto k = 0; k < 3; ++k)
				ab += m_matrix[i][k] * ref.m_matrix[k][j];
			matrix.m_matrix[i][j] = ab;
		}
	}
	return matrix;
} //Matrix3x3::operator*
		
// MARK: Functions (const)

/*--------------------------------------------------------------------
	Get the inverse matrix (or identity if none exists)
	
	return: An inverse matrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix3x3::getInverse() const {
	auto res00 = m_matrix[1][1] * m_matrix[2][2] - m_matrix[1][2] * m_matrix[2][1];
	auto res01 = m_matrix[1][2] * m_matrix[2][0] - m_matrix[1][0] * m_matrix[2][2];
	auto res02 = m_matrix[1][0] * m_matrix[2][1] - m_matrix[1][1] * m_matrix[2][0];
	auto res10 = m_matrix[2][1] * m_matrix[0][2] - m_matrix[2][2] * m_matrix[0][1];
	auto res11 = m_matrix[2][2] * m_matrix[0][0] - m_matrix[2][0] * m_matrix[0][2];
	auto res12 = m_matrix[2][0] * m_matrix[0][1] - m_matrix[2][1] * m_matrix[0][0];
	auto res20 = m_matrix[0][1] * m_matrix[1][2] - m_matrix[0][2] * m_matrix[1][1];
	auto res21 = m_matrix[0][2] * m_matrix[1][0] - m_matrix[0][0] * m_matrix[1][2];
	auto res22 = m_matrix[0][0] * m_matrix[1][1] - m_matrix[0][1] * m_matrix[1][0];
	auto determinant = getDeterminant();
	if (isZero(determinant))
		return Matrix3x3::createIdentity();
	
	auto scale = 1.0 / determinant;
	return Matrix3x3(scale * res00, scale * res10, scale * res20,
					 scale * res01, scale * res11, scale * res21,
					 scale * res02, scale * res12, scale * res22);
} //Matrix3x3::getInverse


/*--------------------------------------------------------------------
	Get the determinant
	
	return: The matrix determinant
  --------------------------------------------------------------------*/
double Matrix3x3::getDeterminant() const {
	return	(m_matrix[0][0] * (m_matrix[1][1] * m_matrix[2][2] - m_matrix[2][1] * m_matrix[1][2])) -
			(m_matrix[0][1] * (m_matrix[1][0] * m_matrix[2][2] - m_matrix[2][0] * m_matrix[1][2])) +
			(m_matrix[0][2] * (m_matrix[1][0] * m_matrix[2][1] - m_matrix[2][0] * m_matrix[1][1]));
} //Matrix3x3::getDeterminant
