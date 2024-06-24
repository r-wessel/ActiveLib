/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Matrix4x4.h"

#include "Active/Utility/MathFunctions.h"

using namespace active::geometry;
using namespace active::math;

// MARK: Factory functions

/*--------------------------------------------------------------------
	Create a rotation matrix about the x axis
	
	angle: The rotation angle
	
	return: The requested rotation matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::createXRotate(const double& angle) {
	return Matrix4x4(	1.0,	0.0,		0.0,			0.0,
						0.0,	cos(angle),	-sin(angle),	0.0,
						0.0,	sin(angle),	cos(angle),		0.0,
						0.0,   0.0,			0.0,			1.0);
} //Matrix4x4::createXRotate


/*--------------------------------------------------------------------
	Create a rotation matrix about the y axis
	
	angle: The rotation angle
	
	return: The requested rotation matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::createYRotate(const double& angle) {
	return Matrix4x4(	cos(angle),		0.0,	sin(angle),	0.0,
						0.0,			1.0,	0.0,		0.0,
						-sin(angle),	0.0,	cos(angle),	0.0,
						0.0,			0.0,	0.0,		1.0);
} //Matrix4x4::createYRotate


/*--------------------------------------------------------------------
	Create a rotation matrix about the z axis
	
	angle: The rotation angle
	
	return: The requested rotation matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::createZRotate(const double& angle) {
	return Matrix4x4(	cos(angle),	-sin(angle),	0.0,	0.0,
						sin(angle),	cos(angle),		0.0,	0.0,
						0.0,		0.0,			1.0,	0.0,
						0.0,		0.0,			0.0,	1.0);
} //Matrix4x4::createZRotate


/*--------------------------------------------------------------------
	Create a scaling matrix
	
	x: The scale in the x axis
	y: The scale in the y axis
	z: The scale in the z axis
	
	return: The requested scaling matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::createScale(const double& x, const double& y, const double& z) {
	Matrix4x4 result;
	result.m_matrix[0][0] = x;
	result.m_matrix[1][1] = y;
	result.m_matrix[2][2] = z;
	result.m_matrix[3][3] = 1;
	return result;
} //Matrix4x4::createScale


/*--------------------------------------------------------------------
	Create a translation matrix
	
	x: The offset in the x axis
	y: The offset in the y axis
	z: The offset in the z axis
	
	return: The requested transformation matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::createTranslate(const double& x, const double& y, const double& z) {
	Matrix4x4 result;
	result.m_matrix[0][0] = 1;
	result.m_matrix[1][1] = 1;
	result.m_matrix[2][2] = 1;
	result.m_matrix[3][0] = x;
	result.m_matrix[3][1] = y;
	result.m_matrix[3][2] = z;
	result.m_matrix[3][3] = 1;
	return result;
} //Matrix4x4::createTranslate


/*--------------------------------------------------------------------
	Create an identity matrix
	
	return: The requested identity matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::createIdentity() {
	Matrix4x4 result;
	result.m_matrix[0][0] = 1;
	result.m_matrix[1][1] = 1;
	result.m_matrix[2][2] = 1;
	result.m_matrix[3][3] = 1;
	return result;
} //Matrix4x4::createIdentity
		
// MARK: Constructors

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Matrix4x4::Matrix4x4() {
	for (auto i = 0; i < 4; ++i) {
		for (auto j = 0; j < 4; ++j)
			m_matrix[i][j] = 0;
	}
} //Matrix4x4::Matrix4x4


/*--------------------------------------------------------------------
	Constructor

	a1-d4: Matrix values
  --------------------------------------------------------------------*/
Matrix4x4::Matrix4x4(	double a1, double b1, double c1, double d1,
					 	double a2, double b2, double c2, double d2,
					 	double a3, double b3, double c3, double d3,
					 	double a4, double b4, double c4, double d4) {
	m_matrix[0][0] = a1;
	m_matrix[0][1] = b1;
	m_matrix[0][2] = c1;
	m_matrix[0][3] = d1;

	m_matrix[1][0] = a2;
	m_matrix[1][1] = b2;
	m_matrix[1][2] = c2;
	m_matrix[1][3] = d2;

	m_matrix[2][0] = a3;
	m_matrix[2][1] = b3;
	m_matrix[2][2] = c3;
	m_matrix[2][3] = d3;

	m_matrix[3][0] = a4;
	m_matrix[3][1] = b4;
	m_matrix[3][2] = c4;
	m_matrix[3][3] = d4;
} //Matrix4x4::Matrix4x4

// MARK: Operators

/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Matrix4x4& Matrix4x4::operator=(const Matrix4x4& source) {
	if (&source != this) {
		for (auto i = 0; i < 4; ++i) {
			for (auto j = 0; j < 4; ++j)
				m_matrix[i][j] = source.m_matrix[i][j];
		}
	}
	return *this;
} //Matrix4x4::operator=


/*--------------------------------------------------------------------
	Equality operator
	
	ref: The object to compare
	
	return: True if the objects are identical
  --------------------------------------------------------------------*/
bool Matrix4x4::operator==(const Matrix4x4& ref) const {
	for (auto i = 0; i < 4; ++i) {
		for (auto j = 0; j < 4; ++j) {
			if (!isEqual(m_matrix[i][j], ref.m_matrix[i][j], 1e-6))
				return false;
		}
	}
	return true;
} //Matrix4x4::operator==


/*--------------------------------------------------------------------
	Inequality operator
	
	ref: The object to compare
	
	return: True if the objects are not identical
  --------------------------------------------------------------------*/
bool Matrix4x4::operator!=(const Matrix4x4& ref) const {
   return !(*this == ref);
} //Matrix4x4::operator!=


/*--------------------------------------------------------------------
	Multiplication operator
	
	ref: The matrix to multiply this by
	
	return: The result
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& ref) const {
	Matrix4x4 matrix;
	for (auto i=0; i < 4; ++i) {
		for (auto j = 0; j < 4; ++j) {
			double ab = 0;
			for (auto k = 0; k < 4; ++k)
				ab += m_matrix[i][k] * ref.m_matrix[k][j];
			matrix.m_matrix[i][j] = ab;
		}
	}
   return matrix;
} //Matrix4x4::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	ref: The matrix to multiply this by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& ref) {
	*this = *this * ref;
	return *this;
} //Matrix4x4::operator*=

// MARK: Functions (const)

/*--------------------------------------------------------------------
	Get the inverse matrix (or identity if none exists)
	
	return: An inverse matrix
  --------------------------------------------------------------------*/
Matrix4x4 Matrix4x4::getInverse() const {
	auto mdet = getDeterminant();
	if (isZero(mdet))
		return Matrix4x4::createIdentity();
	double invDet = 1.0 / mdet;
	Matrix4x4 result;
	for (auto row = 0; row < 4; ++row) {
		for (auto col = 0; col < 4; ++col) {
			double sign = 1.0 - 2.0 * ((row + col) % 2);
			result[row][col] = (getSubmatrix(col, row).getDeterminant() * sign) * invDet;
		}
	}
	return result;
} //Matrix4x4::getInverse()


/*--------------------------------------------------------------------
	Get the determinant
	
	return: The matrix determinant
  --------------------------------------------------------------------*/
double Matrix4x4::getDeterminant() const {
	double result = 0.0, i = 1.0;
	for (auto n = 0; n < 4; ++n) {
		result += (m_matrix[0][n] * getSubmatrix(0, n).getDeterminant() * i);
		i *= -1.0;
	}
	return result;
} //Matrix4x4::getDeterminant


/*--------------------------------------------------------------------
	Get the requested submatrix
	
	row: Row position
	column: Column position
	
	return: The requested submatrix
  --------------------------------------------------------------------*/
Matrix3x3 Matrix4x4::getSubmatrix(unsigned short row, unsigned short column) const {
	Matrix3x3 result;
	for (auto srcRow = 0; srcRow < 4; ++srcRow) {
		unsigned short destRow;
		if (srcRow < row) {
			destRow = srcRow;
		} else if (srcRow > 0) {
			destRow = srcRow - 1;
		} else {
			continue;
		}
		for (auto srcCol = 0; srcCol < 4; ++srcCol) {
			unsigned short destCol;
			if (srcCol < column) {
				destCol = srcCol;
			} else if (srcCol > 0) {
				destCol = srcCol - 1;
			} else {
				continue;
			}
			if ((srcRow != row) && (srcCol != column)) {
				result(destRow, destCol) = m_matrix[srcRow][srcCol];
			}
		}
	}
	return result;
} //Matrix4x4::getSubmatrix
