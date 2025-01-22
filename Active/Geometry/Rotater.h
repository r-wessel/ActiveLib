/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_ROTATER
#define ACTIVE_GEOMETRY_ROTATER

#include "Active/Geometry/Point.h"

namespace active::geometry {
	
	class Arc;
	class Line;
	class Polygon;
	class PolyEdge;
	class Plane;
	class Vector3;
	class Vector4;

	/// Base class for rotation classes
	class Rotater : public utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Rotater>;
			///Shared pointer
		using Shared = std::shared_ptr<Rotater>;
			///Optional
		using Option = std::optional<Rotater>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param angle The rotation angle
			@param prec The required precision
		*/
		Rotater(double angle = 0.0, double prec = math::eps) { setAngle(angle, prec); }
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Rotater(const Rotater& source);
		/*!
			Destructor
		*/
		virtual ~Rotater() = default;
		
		virtual Rotater* clonePtr() const = 0;
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Rotater& operator= (const Rotater& source);
		
		// MARK: - Functions (const)
		
		/*!
			Get the rotation angle
			@return The rotation angle
		*/
		virtual double getAngle() const { return m_angle; }
		/*!
			Determine if the rotation is necessary
			@return True is the rotation is necessary (the angle is non-zero)
		*/
		virtual bool isActive() const { return m_isActive; }
		/*!
			Rotate a point
			@param target The point to be rotated
		*/
		virtual Point& transformPt(Point& target) const = 0;
		/*!
			Rotate an arc
			@param target The arc to be rotated
		*/
		virtual void transform(Arc& target) const;
		/*!
			Rotate a line
			@param target The line to be rotated
		*/
		virtual void transform(Line& target) const;
		/*!
			Rotate an edge
			@param target The edge to be rotated
		*/
		virtual void transform(PolyEdge& target) const;
		/*!
			Rotate a vector
			@param target The vector to be rotated
		*/
		virtual void transform(Vector3& target) const;
		/*!
			Rotate a vector
			@param target The vector to be rotated
		*/
		virtual void transform(Vector4& target) const;
		/*!
			Rotate a plane
			@param target The plane to be rotated
		*/
		virtual void transform(Plane& target) const;
		/*!
			Rotate a polygon
			@param target The polygon to be rotated
		*/
		virtual void transform(Polygon& target) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the rotation angle
			@param angle The rotation angle
			@param prec The required precision
		*/
		virtual void setAngle(double angle, double prec = math::eps);
		/*!
			Reverse the angle of rotation
		*/
		virtual Rotater& reverse() { if (m_isActive) setAngle(-getAngle()); return *this; }
		
	protected:
		double k1() const { return m_k1; }
		double k2() const { return m_k2; }
		
	private:
		bool m_isActive;
		double m_angle;
		double m_k1;
		double m_k2;
	};


	/// Class to perform rotations in the x axis
	class XRotater : public Rotater {
	public:
		
		// MARK: - Constructors
		
		/*!
			Copy constructor
			@param angle The rotation angle
			@param prec The required precision
		*/
		XRotater(double angle = 0.0, double prec = math::eps)	: Rotater(angle, prec) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		XRotater(const Rotater& source) : Rotater(source) {}
		/*!
			Destructor
		*/
		virtual ~XRotater() = default;
		
		virtual Rotater* clonePtr() const { return new XRotater(*this); }
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		XRotater& operator= (const XRotater& source) { Rotater::operator=(source); return *this; }
		
		// MARK: - Functions (const)
		
		/*!
			Rotate a point about the x axis
			@param target The point to be rotated
		*/
		virtual Point& transformPt(Point& target) const;
	};


	/// Class to perform rotations in the y axis
	class YRotater : public Rotater {
	public:
		
		// MARK: - Constructors
		
		/*!
			Copy constructor
			@param angle The rotation angle
			@param prec The required precision
		*/
		YRotater(double angle = 0.0, double prec = math::eps)	: Rotater(-angle, prec) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		YRotater(const Rotater& source) : Rotater(source) {}
		/*!
			Destructor
		*/
		virtual ~YRotater() = default;
		
		virtual Rotater* clonePtr() const { return new YRotater(*this); }
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		YRotater& operator= (const YRotater& source) { Rotater::operator=(source); return *this; }
		
		// MARK: - Functions (const)
		
		/*!
			Get the rotation angle
			@return The rotation angle
		*/
		virtual double getAngle() const { return -Rotater::getAngle(); }
		/*!
			Rotate a point about the y axis
			@param target The point to be rotated
		*/
		virtual Point& transformPt(Point& target) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the rotation angle
			@param angle The rotation angle
			@param prec The required precision
		*/
		virtual void setAngle(double angle, double prec = math::eps) { Rotater::setAngle(-angle, prec); }
	};


	/// Class to perform rotations in the z axis
	class ZRotater : public Rotater {
	public:
		
		// MARK: - Constructors
		
		/*!
			Copy constructor
			@param angle The rotation angle
			@param prec The required precision
		*/
		ZRotater(double angle = 0.0, double prec = math::eps)	: Rotater(angle, prec) {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		ZRotater(const Rotater& source) : Rotater(source) {}
		/*!
			Destructor
		*/
		virtual ~ZRotater() = default;
		
		virtual Rotater* clonePtr() const { return new ZRotater(*this); }
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		ZRotater& operator= (const ZRotater& source) { Rotater::operator=(source); return *this; }
		
		// MARK: - Functions (const)
		
		/*!
			Rotate a point about the z axis
			@param target The point to be rotated
		*/
		virtual Point& transformPt(Point& target) const;
	};
	
}

#endif	//ACTIVE_GEOMETRY_ROTATER
