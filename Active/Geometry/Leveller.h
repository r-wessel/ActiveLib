/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_LEVELLER
#define ACTIVE_GEOMETRY_LEVELLER

#include "Active/Geometry/Vector3.h"

namespace active::geometry {
	
	class Arc;
	class Point;
	class Polygon;
	class Line;
	class Rotater;
	
	/// Class to reorientate geometric objects to a horizontal plane
	class Leveller {
	public:
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		Leveller();
		/*!
			Constructor
			@param norm Normal to the plane to be levelled
			@param zAngle The required rotation about the z axis
			@param prec The required precision
		*/
		Leveller(const Vector3& norm, double zAngle = 0.0, double prec = math::eps);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Leveller(const Leveller& source);
		/*!
			Destructor
		*/
		~Leveller();
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Leveller& operator= (const Leveller& source);
		
		// MARK: - Functions (const)

		/*!
			Get the normal to the plane to be levelled
			@return The normal to the plane to be levelled
		*/
		const Vector3& getNormal() const { return m_normal; }
		/*!
			Transform a point
			@param target The point to be transformed
		*/
		void transform(Point& target) const;
		/*!
			Transform a line
			@param target The line to be transformed
		*/
		void transform(Line& target) const;
		/*!
			Transform an arc
			@param target The arc to be transformed
		*/
		void transform(Arc& target) const;
		/*!
			Transform a polygon
			@param target The polygon to be transformed
		*/
		void transform(Polygon& target) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the normal to the plane to be levelled
			@param norm The normal to the plane to be levelled
			@param zAngle The required rotation about the z axis
			@param prec The required precision
		*/
		void setRotation(const Vector3& norm, double zAngle = 0, double prec = math::eps);
		/*!
			Set the rotation about the z axis
			@param angle The rotation about the z axis
		*/
		void setZRotation(double angle);
		/*!
			Reverse the order of the transformation
			@return A reference to this
		*/
		Leveller& reverse();
		
	private:
		Vector3 m_normal;
		std::unique_ptr<Rotater> m_rz;
		std::unique_ptr<Rotater> m_ry;
		std::unique_ptr<Rotater> m_rx;
	};
	
}

#endif	//ACTIVE_GEOMETRY_LEVELLER
