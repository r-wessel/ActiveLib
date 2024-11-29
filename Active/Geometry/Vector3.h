/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_VECTOR_3
#define ACTIVE_GEOMETRY_VECTOR_3

#include "Active/Geometry/Point.h"

#include <array>

namespace active::geometry {
	
	class Line;
	class Matrix3x3;
	class Matrix4x4;
	class Vertex;

	/// A 1x3 vector class
	class Vector3 {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Vector3>;
			///Shared pointer
		using Shared = std::shared_ptr<Vector3>;
			///Optional
		using Option = std::optional<Vector3>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		Vector3();
		/*!
			Constructor
			@param x X coefficient
			@param y Y coefficient
			@param z Z coefficient
		*/
		Vector3(double x, double y, double z = 0.0);
		/*!
			Constructor
			@param source A point to construct the vector from
		*/
		explicit Vector3(const Point& source);
		/*!
			Constructor
			@param source A vertex to construct the vector from
		*/
		explicit Vector3(const Vertex& source);
		/*!
			Constructor
			@param source A line to construct the vector from
		*/
		Vector3(const Line& source);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Vector3(const Vector3& source);
		/*!
			Destructor
		*/
		virtual ~Vector3() = default;
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The point to copy
			@return A reference to this
		*/
		Vector3& operator= (const Point& source);
		/*!
			Assignment operator
			@param source The vertex to copy
			@return A reference to this
		*/
		Vector3& operator= (const Vertex& source);
		/*!
			Assignment operator
			@param source The line to copy
			@return A reference to this
		*/
		Vector3& operator= (const Line& source);
		/*!
			Assignment operator
			@param source The vector to copy
			@return A reference to this
		*/
		Vector3& operator= (const Vector3& source);
		/*!
			Addition operator
			@param vect The vector to add
			@return The resultant vector
		*/
		Vector3 operator+ (const Vector3& vect) const;
		/*!
			Addition and assignment operator
			@param vect The vector to add
			@return A reference to this
		*/
		Vector3& operator+= (const Vector3& vect);
		/*!
			Subtraction operator
			@param vect The vector to substract
			@return The resultant vector
		*/
		Vector3 operator- (const Vector3& vect) const;
		/*!
			Subtraction and assignment operator
			@param vect The vector to subtract
			@return A reference to this
		*/
		Vector3& operator-= (const Vector3& vect);
		/*!
			Multiplication operator
			@param scale The scale factor
			@return The resultant vector
		*/
		Vector3 operator* (double scale) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply by
			@return The resultant vector
		*/
		Vector3 operator* (const Matrix3x3& matrix) const;
		/*!
			Multiplication operator
			@param matrix The matrix to multiply by
			@return The resultant vector
		*/
		Vector3 operator* (const Matrix4x4& matrix) const;
		/*!
			Multiplication and assignment operator
			@param scale The scale factor
			@return The resultant vector
		*/
		Vector3& operator*= (double scale);
 		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply by
			@return A reference to this
		*/
		Vector3& operator*= (const Matrix3x3& matrix);
 		/*!
			Multiplication and assignment operator
			@param matrix The matrix to multiply by
			@return A reference to this
		*/
		Vector3& operator*= (const Matrix4x4& matrix);
		/*!
			Subscript operator
			@param row Index into the vector
			@return The requested coefficient
		*/
		double& operator[] (int row) { return m_vector[row % 3]; }
		/*!
			Subscript operator
			@param row Index into the vector
			@return The requested coefficient
		*/
		const double operator[] (int row) const { return m_vector[row % 3]; }
		/*!
			Conversion operator
			@return Point derived from this
		*/
		operator Point() const;
		/*!
			Conversion operator
			@return Vertex derived from this
		*/
		operator Vertex() const;
		
		// MARK: - Functions (const)

		/*!
			Get the azimuth angle of the vector
			@return The azimuth angle
		*/
		double azimuthAngle() const;
		/*!
			Get the altitude angle of the vector
			@return The altitude angle
		*/
		double altitudeAngle() const;
		/*!
			Determine of a vector is parallel to this
			@param ref A vector
			@param prec The precision of the test
			@return True if the vector is parallel to this
		*/
		bool isParallelTo(const Vector3& ref, double prec = math::eps) const;
		/*!
			Determine if a vector is perpendicular to this
			@param ref A vector
			@param prec The precision of the test
			@return True if the vector is perpendicular to this
		*/
		bool isPerpendicularTo(const Vector3& ref, double prec = math::eps) const;
		/*!
			Determine if a vector has the same sense as this
			@param ref A vector
			@param prec The precision of the test
			@return True if the vector has the same sense as this
		*/
		bool isSameSense(const Vector3& ref, double prec = math::eps) const;
		/*!
			Determine if the vector is empty (all null values)
			@param prec The precision of the test
			@return True if the vector is empty
		*/
		bool isEmpty(double prec = math::eps) const;
		/*!
			Determine if the vector is along the z axis
			@return True if the vector is along the z axis
		*/
		bool isZAxis(double prec = math::eps) const
				{ return (math::isZero(m_vector[0], prec) && math::isZero(m_vector[1], prec) && !math::isZero(m_vector[2], prec)); }
		/*!
			Determine if the vector is in the plane of the x/y axis
			@return True if the vector is in the plane of the x/y axis
		*/
		bool isXyPlane(double prec = math::eps) const
				{ return ((!math::isZero(m_vector[0], prec) || !math::isZero(m_vector[1], prec)) && math::isZero(m_vector[2], prec)); }
		/*!
			Get the dot product of two vectors
			@param ref A vector
			@return The dot product
		*/
		double dotProduct(const Vector3& ref) const;
		/*!
			Get the vectorial product of two vectors
			@param ref A vector
			@return The vectorial product (orthogonal to both)
		*/
		Vector3 vectorProduct(const Vector3& ref) const;
		/*!
			Get the magnitude of the vector
			@return The magnitude of the vector
		*/
		double magnitude() const;
		/*!
			Get the modulus of the vector
			@return The modulus of the vector
		*/
		double modulus() const { return magnitude(); }
		/*!
			Get the normalised vector
			@return The normalised vector
		*/
		Vector3 normalised() const;
		/*!
			Get the angle between a vector and this
			@param ref A vector
			@return The angle between the vector and this
		*/
		double angleTo(const Vector3& ref) const;
		
	private:
		std::array<double, 3> m_vector;
	};

}

#endif //ACTIVE_GEOMETRY_VECTOR_3
