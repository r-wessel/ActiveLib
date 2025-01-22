/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_VECTOR_4
#define ACTIVE_GEOMETRY_VECTOR_4

#include "Active/Geometry/Point.h"

#include <array>

namespace active::primitive {
	class Vertex;
}

namespace active::geometry {
	
	class Line;
	class Matrix4x4;
	class Vector3;
	
	///A 1x4 vector class
	class Vector4 {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Vector4>;
			///Shared pointer
		using Shared = std::shared_ptr<Vector4>;
			///Optional
		using Option = std::optional<Vector4>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		Vector4();
		/*!
			Constructor
			@param x X coefficient
			@param y Y coefficient
			@param z Z coefficient
		*/
		Vector4(double x, double y, double z);
		/*!
			Constructor
			@param source A point to construct the vector from
		*/
		explicit Vector4(const Point& source);
		/*!
			Constructor
			@param source A vertex to construct the vector from
		*/
		explicit Vector4(const primitive::Vertex& source);
		/*!
			Constructor
			@param source A line to construct the vector from
		*/
		Vector4(const Line& source);
		/*!
			Constructor
			@param source The object to copy
		*/
		Vector4(const Vector3& source);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Vector4(const Vector4& source);
		/*!
			Destructor
		*/
		virtual ~Vector4() = default;
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Vector4& operator=(const Point& source);
		/*!
			Assignment operator
			@param source The vertex to copy
			@return A reference to this
		*/
		Vector4& operator=(const primitive::Vertex& source);
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Vector4& operator=(const Line& source);
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Vector4& operator=(const Vector3& source);
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Vector4& operator=(const Vector4& source);
		/*!
			Addition operator
			@param vect The vector to add
			@return The resultant vector
		*/
		Vector4 operator+(const Vector4& vect) const;
		/*!
			Addition and assignment operator
			@param vect The vector to add
			@return A reference to this
		*/
		Vector4& operator+=(const Vector4& vect);
		/*!
			Subtraction operator
			@param vect The vector to substract
			@return The resultant vector
		*/
		Vector4 operator-(const Vector4& vect) const;
		/*!
			Subtraction and assignment operator
			@param vect The vector to subtract
			@return A reference to this
		*/
		Vector4& operator-=(const Vector4& vect);
		/*!
			Multiplication operator
			@param scale The scale factor
			@return The resultant vector
		*/
		Vector4 operator*(double scale) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply by
			@return The resultant vector
		*/
		Vector4 operator*(const Matrix4x4& matrix) const;
		/*!
			Multiplication and assignment operator
			@param scale The scale factor
			@return The resultant vector
		*/
		Vector4& operator*=(double scale);
 		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply by
			@return A reference to this
		*/
		Vector4& operator*=(const Matrix4x4& matrix);
		/*!
			Subscript operator
			@param row Index into the vector
			@return The requested coefficient
		*/
		double& operator[](int row) { return m_vector[row % 4]; }
		/*!
			Subscript operator
			@param row Index into the vector
			@return The requested coefficient
		*/
		const double operator[](int row) const { return m_vector[row % 4]; }
		/*!
			Conversion operator
			@return Point derived from this
		*/
		operator Point() const;
		/*!
			Conversion operator
			@return Vertex derived from this
		*/
		operator primitive::Vertex() const;
		
		// MARK: - Functions (const)
		
		/*!
			Determine of a vector is parallel to this
			@param ref A vector
			@param prec The precision of the test
			@return True if the vector is parallel to this
		*/
		bool isParallelTo(const Vector4& ref, double prec = math::eps) const;
		/*!
			Determine if a vector is perpendicular to this
			@param ref A vector
			@param prec The precision of the test
			@return True if the vector is perpendicular to this
		*/
		bool isPerpendicularTo(const Vector4& ref, double prec = math::eps) const;
		/*!
			Determine if a vector has the same sense as this
			@param ref A vector
			@param prec The precision of the test
			@return True if the vector has the same sense as this
		*/
		bool isSameSense(const Vector4& ref, double prec = math::eps) const;
		/*!
			Determine if the vector is empty (all null values)
			@param prec The precision of the test
			@return True if the vector is empty
		*/
		bool isEmpty(double prec = math::eps) const;
		/*!
			Get the dot product of two vectors
			@param ref A vector
			@return The dot product
		*/
		double dotProduct(const Vector4& ref) const;
		/*!
			Get the vectorial product of two vectors (a perpendicular vector)
			@param ref A vector
			@return The vectorial product
		*/
		Vector4 vectorProduct(const Vector4& ref) const;
		/*!
			Get the modulus of the vector
			@return The modulus of the vector
		*/
		double modulus() const { return magnitude(); }
		/*!
			Get the magnitude of the vector
			@return The magnitude of the vector
		*/
		double magnitude() const;
		/*!
			Get the normalised vector
			@return The normalised vector
		*/
		Vector4 normalised() const;
		/*!
			Get the angle between a vector and this
			@param ref A vector
			@return The angle between the vector and this
		*/
		double angleTo(const Vector4& ref) const;
		
	private:
		std::array<double, 4> m_vector;
	};

}

#endif //ACTIVE_GEOMETRY_VECTOR_4
