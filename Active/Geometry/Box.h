/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_GEOMETRY_BOX
#define ACTIVE_GEOMETRY_BOX

#include "Active/Geometry/Anchor2D.h"
#include "Active/Geometry/Point.h"

#include <memory>

namespace active::geometry {
	
	/*!
		Class to represent a box (rectilinear bounds in 3 dimensions described by 2 diagonally opposite points)
	*/
	class Box {
	public:
		
		// MARK: - Types
		
		using enum Anchor2D;
		
			///Unique pointer
		using Unique = std::unique_ptr<Box>;
			///Shared pointer
		using Shared = std::shared_ptr<Box>;
			///Optional
		using Option = std::optional<Box>;
		
		// MARK: - Constructors

		/*!
			Constructor
		*/
		Box() {}
		/*!
			Constructor
			@param point A point defining the box position/bounds
		*/
		Box(const Point& point) : origin(point), end(point) {}
		/*!
			Constructor
			@param origin One corner of the box
			@param end The opposite corner of the box
		*/
		Box(const Point& origin, const Point& end) : origin(origin),  end(end) {}
		/*!
			Constructor
			@param x1 Bounding coordinates of the box
		*/
		Box(double x1, double y1, double z1, double x2, double y2, double z2) : origin(x1, y1, z1), end(x2, y2, z2) {}
		/*!
			Constructor
			@param width The box width
			@param depth The box depth
		*/
		Box(double width, double depth) : origin(), end(width, depth) {}
		/*!
			Destructor
		*/
		~Box() {}
		
		// MARK: - Variables
		
			///Box origin (for diagonally opposite points)
		Point origin;
			///Box end
		Point end;
		
		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref The box to compare with this
			@return True if the boxes are identical
		*/
		bool operator== (const Box& ref) const { return isEqual3D(ref); }
		/*!
			Inequality operator
			@param ref The box to compare with this
			@return True if the boxes are different
		*/
		bool operator!= (const Box& ref) const { return !isEqual3D(ref); }
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Box& operator= (const Box& source);
		/*!
			Addition operator
			@param offset The amount to add to the box
			@return An offset box
		*/
		Box operator+ (const Point& offset) const;
		/*!
			Addition with assignment operator
			@param offset The amount to add to the box
			@return A reference to this
		*/
		Box& operator+= (const Point& offset);
		/*!
			Subtraction operator
			@param offset The amount to subtract from the box
			@return An offset box
		*/
		Box operator- (const Point& offset) const;
		/*!
			Subtraction with assignment operator
			@param offset The amount to subtract from the box
			@return A reference to this
		*/
		Box& operator-= (const Point& offset);
		/*!
			Multiplication operator
			@param scale The amount to multiply the box by
			@return A multiplied box
		*/
		Box operator* (double scale) const;
		/*!
			Multiplication with assignment operator
			@param scale The amount to multiply the box by
			@return A reference to this
		*/
		Box& operator*= (double scale);
		/*!
			Division operator
			@param scale The amount to divide the box bounds by
			@return A multiplied box
		*/
		Box operator/ (double scale) const { return (*this) * (1 / scale); }
		/*!
			Division with assignment operator
			@param scale The amount to divide the box bounds by
			@return A reference to this
		*/
		Box& operator/= (double scale) { return (*this) *= (1 / scale); }
		
		// MARK: - Functions (const)
		
		/*!
			Determine if two boxes are equal in 2D
			@param ref The box to compare with this
			@param prec The required precision
			@return True if the boxes are identical
		*/
		bool isEqual2D(const Box& ref, double prec = math::eps) const;
		/*!
			Determine if two boxes are equal
			@param ref The box to compare with this
			@param prec The required precision
			@return True if the boxes are identical
		*/
		bool isEqual3D(const Box& ref, double prec = math::eps) const;
		/*!
			Get the centre point of the box
			@return The box centre point
		*/
		Point getCentre() const { return (origin + end) / 2; }
		/*!
			Get the position of a specified 2D anchor
		 	@param anchor The required anchor
			@return The requested anchor position
		*/
		Point getAnchor2D(Anchor2D anchor) const;
		/*!
			Get the box 2D area
			@return The box 2D area
		*/
		double getArea() const;
		/*!
			Get the box volume
			@return The box volume
		*/
		double getVolume() const;
		/*!
			Get the box width
			@return The box width
		*/
		double getWidth() const { return fabs(origin.x - end.x); }
		/*!
			Get the box depth
			@return The box depth
		*/
		double getDepth() const { return fabs(origin.y - end.y); }
		/*!
			Get the box height
			@return The box height
		*/
		double getHeight() const { return fabs(origin.z - end.z); }
		/*!
			Get the greatest length in any dimension
			@return The greatest length in any dimension
		*/
		double getMaxLength() const;
		/*!
			Determine the relationship of a point to the box in 2D
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf2D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine the relationship of a point to the box
			@param ref The point to test
			@param prec The required precision
			@return The relative position of the point
		*/
		Position positionOf3D(const Point& ref, double prec = math::eps) const;
		/*!
			Determine if the this encloses a specified box in 2D
			@param ref The reference box
			@param prec The required precision
			@return True if the box is enclosed by this (in or along)
		*/
		bool encloses2D(const Box& ref, double prec = math::eps) const;
		/*!
			Determine if the this encloses a specified box in 3D
		 	@param ref The reference box
		 	@param prec The required precision
		 	@return True if the box is enclosed by this (in or along)
		*/
		bool encloses3D(const Box& ref, double prec = math::eps) const;
		/*!
			Determine if the this overlaps a another box in 2D
			@param ref The reference box
			@param prec The required precision
			@return True if the box is overlapped by this (partial or total)
		*/
		bool overlaps2D(const Box& ref, double prec = math::eps) const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Move the origin to the specified location (retaining the overall bounds)
			@param position The position to move to
		*/
		void moveTo(const Point& position) { *this += (position - getAnchor2D(leftFront)); }
		/*!
			Set the centre point of the box
			@param centre The box centre point
		*/
		void setCentre(const Point& centre) { *this += (centre - getCentre()); }
		/*!
			Merge the specified point into the box bounds
			@param ref The point to merge
		*/
		void merge(const Point& ref);
		/*!
			Merge the specified box into the box bounds
			@param ref The box to merge
		*/
		void merge(const Box& ref);
		/*!
			Sort the box bounds in ascending order
		*/
		void sort();
		/*!
			Scale the box whilst maintaining a common centre
			@param scale The ratio to scale the box by
		*/
		void magnify(double scale);
		/*!
			Resize the box whilst maintaining a common centre
			@param len The amount to adjust the box bounds by
		*/
		void resize(double len);
		/*!
			Rotate the box (calculate a new bounding box from the rotated box)
			@param angle The rotation angle
		*/
		void rotate(double angle);
	};
	
}

#endif	//ACTIVE_GEOMETRY_BOX
