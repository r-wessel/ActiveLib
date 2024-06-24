/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Box.h"

#include "Active/Geometry/Rotater.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Box& Box::operator= (const Box& source) {
	if (this != &source) {
		origin = source.origin;
		end = source.end;
	}
	return *this;
} //Box::operator=


/*--------------------------------------------------------------------
	Addition operator
	
	offset: The amount to add to the box
	
	return: An offset box
  --------------------------------------------------------------------*/
Box Box::operator+ (const Point& offset) const {
	return (Box(*this) += offset);
} //Box::operator+


/*--------------------------------------------------------------------
	Addition with assignment operator
	
	offset: The amount to add to the box
	
	return: A reference to this
  --------------------------------------------------------------------*/
Box& Box::operator+= (const Point& offset) {
	origin += offset;
	end += offset;
	return *this;
} //Box::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	offset: The amount to subtract from the box
	
	return: An offset box
  --------------------------------------------------------------------*/
Box Box::operator- (const Point& offset) const {
	return (Box(*this) -= offset);
} //Box::operator-


/*--------------------------------------------------------------------
	Subtraction with assignment operator
	
	offset: The amount to subtract from the box
	
	return: A reference to this
  --------------------------------------------------------------------*/
Box& Box::operator-= (const Point& offset) {
	origin -= offset;
	end -= offset;
	return *this;
} //Box::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The amount to multiply the box by
	
	return: A multiplied box
  --------------------------------------------------------------------*/
Box Box::operator* (double scale) const {
	return (Box(*this) *= scale);
} //Box::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The amount to multiply the box by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Box& Box::operator*= (double scale) {
	origin *= scale;
	end *= scale;
	return *this;
} //Box::operator*=

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if two boxes are equal in 2D
	
	ref: The box to compare with this
	prec: The required precision
	
	return: True if the boxes are identical
  --------------------------------------------------------------------*/
bool Box::isEqual2D(const Box& ref, double prec) const {
	return (origin.isEqual2D(ref.origin) && end.isEqual2D(ref.end));
} //Box::isEqual2D


/*--------------------------------------------------------------------
	Determine if two boxes are equal
	
	ref: The box to compare with this
	prec: The required precision
	
	return: True if the boxes are identical
  --------------------------------------------------------------------*/
bool Box::isEqual3D(const Box& ref, double prec) const {
	return (origin.isEqual3D(ref.origin) && end.isEqual3D(ref.end));
} //Box::isEqual3D


/*--------------------------------------------------------------------
	Get the box 2D area
	
	return: The box 2D area
  --------------------------------------------------------------------*/
double Box::getArea() const {
	return (getWidth() * getDepth());
} //Box::getArea


/*--------------------------------------------------------------------
	Get the box volume
	
	return: The box volume
  --------------------------------------------------------------------*/
double Box::getVolume() const {
	return (getWidth() * getDepth() * getHeight());
} //Box::getVolume


/*--------------------------------------------------------------------
	Get the position of a specified 2D anchor
	
	anchor: The required anchor
	
	return: The requested anchor position
  --------------------------------------------------------------------*/
Point Box::getAnchor2D(Anchor2D anchor) const {
	short anchorVal = static_cast<short>(anchor);
	short xAnch = anchorVal % 3, yAnch = (anchorVal / 3) % 3;
	Box temp(*this);
	temp.sort();
	return temp.origin + Point(getWidth() * static_cast<double>(xAnch) / 2, getDepth() * static_cast<double>(yAnch) / 2);
} //Box::getAnchor2D


/*--------------------------------------------------------------------
	Get the greatest length in any dimension
	
	return: The greatest length in any dimension
  --------------------------------------------------------------------*/
double Box::getMaxLength() const {
	return maxVal(maxVal(getWidth(), getDepth()), getHeight());
} //Box::getMaxLength


/*--------------------------------------------------------------------
	Determine the relationship of a point to the box
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position Box::positionOf3D(const Point& ref, double prec) const {
	if (isLess(origin.x, end.x, prec)) {
		if (isLess(ref.x, origin.x, prec) || isGreater(ref.x, end.x, prec))
			return Point::outside;
	} else {
		if (isLess(ref.x, end.x, prec) || isGreater(ref.x, origin.x, prec))
			return Point::outside;
	}
	if (isLess(origin.y, end.y, prec)) {
		if (isLess(ref.y, origin.y, prec) || isGreater(ref.y, end.y, prec))
			return Point::outside;
	} else {
		if (isLess(ref.y, end.y, prec) || isGreater(ref.y, origin.y, prec))
			return Point::outside;
	}
	if (isLess(origin.z, end.z, prec)) {
		if (isLess(ref.z, origin.z, prec) || isGreater(ref.z, end.z, prec))
			return Point::outside;
	} else {
		if (isLess(ref.z, end.z, prec) || isGreater(ref.z, origin.z, prec))
			return Point::outside;
	}
	return ((isEqual(origin.x, ref.x, prec) || isEqual(end.x, ref.x, prec)) &&
			(isEqual(origin.y, ref.y, prec) || isEqual(end.y, ref.y, prec)) &&
			(isEqual(origin.z, ref.z, prec) || isEqual(end.z, ref.z, prec))) ?
		Point::along : Point::inside;
} //Box::positionOf3D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the box in 2D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position Box::positionOf2D(const Point& ref, double prec) const {
	if (isLess(origin.x, end.x, prec)) {
		if (isLess(ref.x, origin.x, prec) || isGreater(ref.x, end.x, prec))
			return Point::outside;
	} else {
		if (isLess(ref.x, end.x, prec) || isGreater(ref.x, origin.x, prec))
			return Point::outside;
	}
	if (isLess(origin.y, end.y, prec)) {
		if (isLess(ref.y, origin.y, prec) || isGreater(ref.y, end.y, prec))
			return Point::outside;
	} else {
		if (isLess(ref.y, end.y, prec) || isGreater(ref.y, origin.y, prec))
			return Point::outside;
	}
	return ((isEqual(origin.x, ref.x, prec) || isEqual(end.x, ref.x, prec)) &&
			(isEqual(origin.y, ref.y, prec) || isEqual(end.y, ref.y, prec))) ?
		Point::along : Point::inside;
} //Box::positionOf2D


/*--------------------------------------------------------------------
	Determine if the this encloses a specified box in 2D
	
	ref: The reference box
	prec: The required precision
	
	return: True if the box is enclosed by this (in or along)
  --------------------------------------------------------------------*/
bool Box::encloses2D(const Box& ref, double prec) const {
	if (positionOf2D(ref.origin) == Point::outside)
		return false;
	if (positionOf2D(ref.end) == Point::outside)
		return false;
	Point corner(ref.origin);
	corner.y = ref.end.y;
	if (positionOf2D(corner) == Point::outside)
		return false;
	corner = ref.origin;
	corner.x = ref.end.x;
	return !(positionOf2D(corner) == Point::outside);
} //Box::encloses2D


/*--------------------------------------------------------------------
 	Determine if the this encloses a specified box in 3D
 
	ref: The reference box
	prec: The required precision
 
	return: True if the box is enclosed by this (in or along)
  --------------------------------------------------------------------*/
bool Box::encloses3D(const Box& ref, double prec) const {
	return (encloses2D(ref) && isGreaterOrEqual(ref.origin.z, origin.z, prec) && isLessOrEqual(ref.end.z, end.z, prec));
} //Box::encloses3D


/*--------------------------------------------------------------------
	Determine if the this overlaps a another box in 2D
	
	ref: The reference box
	prec: The required precision
	
	return: True if the box is overlapped by this (partial or total)
  --------------------------------------------------------------------*/
bool Box::overlaps2D(const Box& ref, double prec) const {
	double max, min, maxRef, minRef;
	if (origin.x > end.x) {
		max = origin.x;
		min = end.x;
	} else{
		max = end.x;
		min = origin.x;
	}
	if (ref.origin.x > ref.end.x) {
		maxRef = ref.origin.x;
		minRef = ref.end.x;
	} else{
		maxRef = ref.end.x;
		minRef = ref.origin.x;
	}
	if (isGreaterOrEqual(min, maxRef) || isLessOrEqual(max, minRef))
		return false;
	if (origin.y > end.y) {
		max = origin.y;
		min = end.y;
	} else{
		max = end.y;
		min = origin.y;
	}
	if (ref.origin.y > ref.end.y) {
		maxRef = ref.origin.y;
		minRef = ref.end.y;
	} else{
		maxRef = ref.end.y;
		minRef = ref.origin.y;
	}
	return !(isGreaterOrEqual(min, maxRef) || isLessOrEqual(max, minRef));
} //Box::overlaps2D

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Merge the specified point into the box bounds
	
	ref: The point to merge
  --------------------------------------------------------------------*/
void Box::merge(const Point& ref) {
	sort();
	if (origin.x > ref.x)
		origin.x = ref.x;
	else if (end.x < ref.x)
		end.x = ref.x;
	if (origin.y > ref.y)
		origin.y = ref.y;
	else if (end.y < ref.y)
		end.y = ref.y;
	if (origin.z > ref.z)
		origin.z = ref.z;
	else if (end.z < ref.z)
		end.z = ref.z;
} //Box::merge


/*--------------------------------------------------------------------
	Merge the specified box into the box bounds
	
	ref: The box to merge
  --------------------------------------------------------------------*/
void Box::merge(const Box& ref) {
	merge(ref.origin);
	merge(ref.end);
} //Box::merge


/*--------------------------------------------------------------------
	Sort the box bounds in ascending order
  --------------------------------------------------------------------*/
void Box::sort() {
	if (origin.x > end.x)
		std::swap(origin.x, end.x);
	if (origin.y > end.y)
		std::swap(origin.y, end.y);
	if (origin.z > end.z)
		std::swap(origin.z, end.z);
} //Box::sort


/*--------------------------------------------------------------------
	Resize the box whilst maintaining a common centre
	
	scale: The ratio to scale the box by
  --------------------------------------------------------------------*/
void Box::magnify(double scale) {
	Point centre = getCentre();
	(*this) *= scale;
	setCentre(centre);
} //Box::magnify


/*--------------------------------------------------------------------
	Resize the box whilst maintaining a common centre
	
	len: The amount to adjust the box bounds by
  --------------------------------------------------------------------*/
void Box::resize(double len) {
	sort();
	Point offset(len, len, len);
	origin -= offset;
	end += offset;
} //Box::resize


/*--------------------------------------------------------------------
	Rotate the box (calculate a new bounding box from the rotated box)
 
	angle: The rotation angle
  --------------------------------------------------------------------*/
void Box::rotate(double angle) {
	Point topLeft(getWidth() / 2, getDepth() / 2);
	Point topRight(-topLeft.x, topLeft.y);
	ZRotater rotater(angle);
	rotater.transformPt(topLeft);
	rotater.transformPt(topRight);
	Box newBox(2 * maxVal(fabs(topLeft.x), fabs(topRight.x)), 2 * maxVal(fabs(topLeft.y), fabs(topRight.y)));
	newBox.setCentre(getCentre());
	operator=(newBox);
} //Box::rotate
