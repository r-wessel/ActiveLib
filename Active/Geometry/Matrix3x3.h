/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_MATRIX_3x3
#define ACTIVE_GEOMETRY_MATRIX_3x3

#include <array>
#include <memory>
#include <optional>

namespace active::geometry {

	///A 3x3 matrix class
	class Matrix3x3 {
	public:
		
		// MARK: Types
		
			//Matrix row
		using row_t = std::array<double, 3>;
			//Matrix container
		using base_t = std::array<row_t, 3>;
			///Unique pointer
		using Unique = std::unique_ptr<Matrix3x3>;
			///Shared pointer
		using Shared = std::shared_ptr<Matrix3x3>;
			///Optional
		using Option = std::optional<Matrix3x3>;
		
		// MARK: Factory functions
		
		/*!
			Create a rotation matrix 
			@param angle The rotation angle
			@return The requested rotation matrix
		*/
		static Matrix3x3 createRotate(double angle);
		/*!
			Create a scaling matrix
			@param x The scale in the x axis
			@param y The scale in the y axis
			@return The requested scaling matrix
		*/
		static Matrix3x3 createScale(double x, double y);
		/*!
			Create a translation matrix
			@param x The offset in the x axis
			@param y The offset in the y axis
			@return The requested transformation matrix
		*/
		static Matrix3x3 createTranslate(double x, double y);
		/*!
			Create an identity matrix
			@return The requested identity matrix
		*/
		static Matrix3x3 createIdentity();
		
		// MARK: Constructors
		
		/*!
			Default constructor
		*/
		Matrix3x3();
		/*!
			Constructor
		*/
		Matrix3x3(	double a1, double b1, double c1,
					double a2, double b2, double c2,
					double a3, double b3, double c3);
		
		// MARK: Operators
		
		/*!
			Equality operator
			@param ref The object to compare
			@return True if the objects are identical
		*/
		bool operator==(const Matrix3x3& ref) const;
		/*!
			Inequality operator
			@param ref The object to compare
			@return True if the objects are not identical
		*/
		bool operator!=(const Matrix3x3& ref) const;
		/*!
			Multiplication operator
			@param ref The matrix to multiply this by
			@return The result
		*/
		Matrix3x3 operator*(const Matrix3x3& ref) const;
		/*!
			Multiplication with assignment operator
			@param ref The matrix to multiply this by
			@return A reference to this
		*/
		Matrix3x3 &operator*=(const Matrix3x3& ref);
		/*!
			Row subscript operator
			@param index The required row
			@return The matrix row
		*/
		const row_t& operator[](unsigned short index) const { return m_matrix[index]; }
		/*!
			Row subscript operator
			@param index The required row
			@return The matrix row
		*/
		row_t& operator[](unsigned short index) { return m_matrix[index]; }
		/*!
			Subscript operator
			@param row The required row
			@param col The required column
			@return The value at the specified position
		*/
		const double& operator()(unsigned short row, unsigned short col) const { return m_matrix[row][col]; }
		/*!
			Subscript operator
			@param row The required row
			@param col The required column
			@return The value at the specified position
		*/
		double& operator()(unsigned short row, unsigned short col) { return m_matrix[row][col]; }
		
		// MARK: Functions (const)
		
		/*!
			Get the inverse matrix (or identity if none exists)
			return: An inverse matrix
		*/
		Matrix3x3 getInverse() const;
		/*!
			Get the determinant
			@return The matrix determinant
		*/
		double getDeterminant() const;
		/*!
			Clear the matrix transformation (set to 0.0)
		*/
		void clearTransform();

	private:
			///The 3x3 matrix
		base_t m_matrix;
	};

}

#endif //ACTIVE_GEOMETRY_MATRIX_3x3
