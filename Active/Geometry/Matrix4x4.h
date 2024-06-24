/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_MATRIX_4x4
#define ACTIVE_GEOMETRY_MATRIX_4x4

#include "Active/Geometry/Matrix3x3.h"

namespace active::geometry {

	///A 4x4 matrix class
	class Matrix4x4 {
	public:
		
		// MARK: Types
		
			//Matrix row
		using row_t = std::array<double, 4>;
			//Matrix container
		using base_t = std::array<row_t, 4>;
			///Unique pointer
		using Unique = std::unique_ptr<Matrix4x4>;
			///Shared pointer
		using Shared = std::shared_ptr<Matrix4x4>;
			///Optional
		using Option = std::optional<Matrix4x4>;
		
		// MARK: Factory functions

		/*!
			Create a rotation matrix about the x axis
			@param angle The rotation angle
			@return The requested rotation matrix
		*/
		static Matrix4x4 createXRotate(const double& angle);
		/*!
			Create a rotation matrix about the y axis
			@param angle The rotation angle
			@return The requested rotation matrix
		*/
		static Matrix4x4 createYRotate(const double& angle);
		/*!
			Create a rotation matrix about the z axis
			@param angle The rotation angle
			@return The requested rotation matrix
		*/
		static Matrix4x4 createZRotate(const double& angle);
		/*!
			Create a scaling matrix
			@param x The scale in the x axis
			@param y The scale in the y axis
			@param z The scale in the z axis
			@return The requested scaling matrix
		*/
		static Matrix4x4 createScale(const double& x, const double& y, const double& z);
		/*!
			Create a translation matrix
			@param x The offset in the x axis
			@param y The offset in the y axis
			@param z The offset in the z axis
			@return The requested transformation matrix
		*/
		static Matrix4x4 createTranslate(const double& x, const double& y, const double& z);
		/*!
			Create an identity matrix
			@return The requested identity matrix
		*/
		static Matrix4x4 createIdentity();
		
		// MARK: Constructors

		/*!
			Constructor
		*/
		Matrix4x4();
		/*!
			Constructor
		*/
		Matrix4x4(	double a1, double b1, double c1, double d1,
					double a2, double b2, double c2, double d2,
					double a3, double b3, double c3, double d3,
					double a4, double b4, double c4, double d4);
		
		// MARK: Operators

		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Matrix4x4& operator=(const Matrix4x4& source);
		/*!
			Equality operator
			@param ref The object to compare
			@return True if the objects are identical
		*/
		bool operator==(const Matrix4x4& ref) const;
		/*!
			Inequality operator
			@param ref The object to compare
			@return True if the objects are not identical
		*/
		bool operator!=(const Matrix4x4& ref) const;
		/*!
			Multiplication operator
			@param ref The matrix to multiply this by
			@return The result
		*/
		Matrix4x4 operator*(const Matrix4x4& ref) const;
		/*!
			Multiplication with assignment operator
			@param ref The matrix to multiply this by
			@return A reference to this
		*/
		Matrix4x4& operator*=(const Matrix4x4& ref);
		/*!
			Unchecked subscript method
			@param index An index into the matrix
			@return The value at the specified index
		*/
		row_t operator[](unsigned short index) { return m_matrix[index]; }
		/*!
			Checked subscript method
			@param row The required row
			@param col The required column
			@return The value at the specified position
		*/
		double& operator()(unsigned short row, unsigned short col) { return m_matrix[row][col]; }
		/*!
			Checked subscript method
			@param row The required row
			@param col The required column
			@return The value at the specified position
		*/
		const double& operator()(unsigned short row, unsigned short col) const { return m_matrix[row][col]; }
		
		// MARK: Functions (const)

		/*!
			Get the inverse matrix (or identity if none exists)
			@return An inverse matrix
		*/
		Matrix4x4 getInverse() const;
		/*!
			Get the determinant
			@return The matrix determinant
		*/
		double getDeterminant() const;
		
	protected:
		/*!
			Get the requested submatrix
			@param row Row position
			@param col Column position
			@return The requested submatrix
		*/
		Matrix3x3 getSubmatrix(unsigned short row, unsigned short col) const;
		
	private:
			///The 4x4 matrix
		base_t m_matrix;
	};

}

#endif //ACTIVE_GEOMETRY_MATRIX_4x4
