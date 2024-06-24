/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Line.h"

#include "Active/Geometry/XList.h"
#include "Active/Geometry/Leveller.h"
#include "Active/Geometry/LinEquation.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Plane.h"
#include "Active/Geometry/Vector3.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Operators

/*--------------------------------------------------------------------
	Addition operator
	
	offset: The amount to add to the line
	
	return: An offset line
  --------------------------------------------------------------------*/
Line Line::operator+ (const Point& offset) const {
	return (Line(*this) += offset);
} //Line::operator+


/*--------------------------------------------------------------------
	Addition with assignment operator
	
	offset: The amount to add to the line
	
	return: A reference to this
  --------------------------------------------------------------------*/
Line& Line::operator+= (const Point& offset) {
	origin += offset;
	end += offset;
	return *this;
} //Line::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	offset: The amount to subtract from the line
	
	return: An offset line
  --------------------------------------------------------------------*/
Line Line::operator- (const Point& offset) const {
	return (Line(*this) -= offset);
} //Line::operator-


/*--------------------------------------------------------------------
	Subtraction with assignment operator
	
	offset: The amount to subtract from the line
	
	return: A reference to this
  --------------------------------------------------------------------*/
Line& Line::operator-= (const Point& offset) {
	origin -= offset;
	end -= offset;
	return *this;
} //Line::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The amount to multiply the line by
	
	return: A multiplied line
  --------------------------------------------------------------------*/
Line Line::operator* (double scale) const {
	return (Line(*this) *= scale);
} //Line::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The amount to multiply the line by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Line& Line::operator*= (double scale) {
	origin *= scale;
	end *= scale;
	return *this;
} //Line::operator*=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The matrix to multiply the line by
	
	return: A multiplied line
  --------------------------------------------------------------------*/
Line Line::operator* (const Matrix3x3& matrix) const {
	return (Line(*this) *= matrix);
} //Line::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The matrix to multiply the line by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Line& Line::operator*= (const Matrix3x3& matrix) {
	origin *= matrix;
	end *= matrix;
	return *this;
} //Line::operator*=

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if two lines are equal in 2D
	
	ref: The line to compare with this
	prec: The required precision
	
	return: True if the lines are identical
  --------------------------------------------------------------------*/
bool Line::isEqual2D(const Line& ref, double prec) const {
	return (((origin.isEqual2D(ref.origin)) && end.isEqual2D(ref.end)) ||
			(end.isEqual2D(ref.origin) && origin.isEqual2D(ref.end)));
} //Line::isEqual2D


/*--------------------------------------------------------------------
	Determine if two lines are equal
	
	ref: The line to compare with this
	prec: The required precision
	
	return: True if the lines are identical
  --------------------------------------------------------------------*/
bool Line::isEqual3D(const Line& ref, double prec) const {
	return (((origin.isEqual3D(ref.origin)) && end.isEqual3D(ref.end)) ||
			(end.isEqual3D(ref.origin) && origin.isEqual3D(ref.end)));
} //Line::isEqual3D


/*--------------------------------------------------------------------
	Get the 2D length of the line

	return: The 2D length of the line
  --------------------------------------------------------------------*/
double Line::length2D() const {
	return origin.lengthFrom2D(end);
} //Line::length2D


/*--------------------------------------------------------------------
	Get the length of the line

	return: The length of the line
  --------------------------------------------------------------------*/
double Line::length3D() const {
	return end.lengthFrom3D(origin);
} //Line::length3D


/*--------------------------------------------------------------------
	Get the azimuth angle of the line
	
	return: The azimuth angle
  --------------------------------------------------------------------*/
double Line::azimuthAngle() const {
	return origin.azimuthAngleTo(end);
} //Line::azimuthAngle


/*--------------------------------------------------------------------
	Get the altitude angle of the line
	
	return: The altitude angle
  --------------------------------------------------------------------*/
double Line::altitudeAngle() const {
	return origin.altitudeAngleTo(end);
} //Line::altitudeAngle


/*--------------------------------------------------------------------
	Get the angle between two lines
	
	ref: The reference line
	
	return: The angle between the lines
  --------------------------------------------------------------------*/
double Line::angleTo(const Line& ref) const {
	return (Vector3(end - origin).angleTo(Vector3(ref.end - ref.origin)));
} //Line::angleTo


/*--------------------------------------------------------------------
	Get the midpoint of the line

	return: The centrepoint of the line
  --------------------------------------------------------------------*/
Point Line::midpoint() const {
	return (origin + end) / 2;
} //Line::midpoint


/*--------------------------------------------------------------------
	Get the height of the line at a specified point
	
	ref: The point at which to calculate the height
	
	return: The height at the specified point
  --------------------------------------------------------------------*/
double Line::heightAt(const Point& ref) const {
	if (isEqual(origin.z, end.z))
		return origin.z;
	auto orig = Plane::create(ref, Vector3(end.x - origin.x, end.y - origin.y, 0));
	if (!orig)
		return origin.z;
	auto intersect = orig->intersectionWith(*this);
	return (intersect) ? intersect->z : origin.z;
} //Line::heightAt


/*--------------------------------------------------------------------
	Get the closest point in the line to a given point in 2D
	
	ref: The reference point
	
	return: The closest point in the line to the reference point
  --------------------------------------------------------------------*/
Point Line::closestPointTo2D(const Point& ref, double prec) const {
	auto lin = LinEquation::create(*this);
	if (lin) {
		auto orig = lin->getPerpendicular(ref);
		if (orig) {
			auto intersect = lin->intersectionWith(*orig);
			if (intersect)
				return {*intersect};
		}
	}
	return origin;
} //Line::closestPointTo2D


/*--------------------------------------------------------------------
	Get the closest point in the line to a given point
	
	ref: The reference point
	
	return: The closest point in the line to the reference point
  --------------------------------------------------------------------*/
Point Line::closestPointTo3D(const Point& ref, double prec) const {
	auto orig = Plane::create(ref, Vector3(origin - end));
	if (!orig)
		return origin;
	auto intersect = orig->intersectionWith(*this, prec);
	if (intersect)
		return *intersect;
	else
		return origin;
} //Line::closestPointTo3D


/*--------------------------------------------------------------------
	Get the closest point along the line to a given point in 2D
	
	ref: The reference point
	
	return: The closest point along the line to the reference point
  --------------------------------------------------------------------*/
Point Line::closestPointAlong2D(const Point& ref, double prec) const {
	auto lin = LinEquation::create(*this);
	if (!lin)
		return origin;
	auto orig = lin->getPerpendicular(ref);
	if (orig) {
		auto intersect = lin->intersectionWith(*orig);
		if (intersect) {
			auto pos = positionOf2D(*intersect);
			if (pos == Point::before)
				return origin;
			else if (pos == Point::after)
				return end;
			return Point(*intersect);
		}
	}
	return origin;
} //Line::closestPointAlong2D


/*--------------------------------------------------------------------
	Get the closest point along the line to a given point
	
	ref: The reference point
	
	return: The closest point along the line to the reference point
  --------------------------------------------------------------------*/
Point Line::closestPointAlong3D(const Point& ref, double prec) const {
	auto orig = Plane::create(ref, Vector3(origin - end));
	if (!orig)
		return origin;
	auto intersect = orig->intersectionWith(*this, prec);
	if (!intersect)
		return origin;
	auto pos = positionOf3D(*intersect);
	if (pos == Point::before)
		return origin;
	else if (pos == Point::after)
		return end;
	return *intersect;
} //Line::closestPointAlong3D


/*--------------------------------------------------------------------
	Get the intersection point of two lines in 2D (NB: either along or projected beyond the line extents)
 
	ref: The reference line
	prec: The required precision
 
	return: The intersection point (nullopt if no intersection is possible)
  --------------------------------------------------------------------*/
XPoint::Option Line::intersectionWith2D(const Line& ref, double prec) const {
	auto eq1 = LinEquation::create(*this);
	auto eq2 = LinEquation::create(ref);
	if (eq1 && eq2)
		return eq1->intersectionWith(*eq2);
	return std::nullopt;
} //Line::intersectionWith2D


/*--------------------------------------------------------------------
	Get the intersection between this and a specified line
	
	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Line::intersectionWith2D(const Line& ref, XList& inter, double prec) const {
	if (auto intersect = intersectionWith2D(ref); intersect) {
		if (inter.isPos(XPoint::target))
			intersect->setPos(XPoint::target, positionOf2D(*intersect, prec));
		if (inter.isPos(XPoint::blade))
			intersect->setPos(XPoint::blade, ref.positionOf2D(*intersect, prec));
		if (inter.insert(std::move(*intersect)))
			return 1;
	}
	return 0;
} //Line::intersectionWith2D


/*--------------------------------------------------------------------
	Get the intersection between this and a specified line in 3D
	
	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Line::intersectionWith3D(const Line& ref, XList& inter, double prec) const {
		//Calculate a plane in which both lines are horizontal
	auto orig = Plane::create((*this).origin, (*this).end,
			ref.end - (ref.origin - (*this).origin));
	if (!orig)
		return 0;
	Line l1(*this), l2(ref);
	Leveller level(orig->getNormal(), 1e-8);
		//Transform the lines onto a horizontal plane
	level.transform(l1);
	level.transform(l2);
		//Check the line planes align
	if (isEqual(l1.origin.z, l2.origin.z, prec)) {
		if (l1.intersectionWith2D(l2, inter, prec) != 0) {
			(**inter.begin()).z = (l1.origin.z + l2.origin.z) / 2;
				//Restore the intersection to the original context
			level.reverse().transform(**inter.begin());
			return 1;
		}
	}
	return 0;
} //Line::intersectionWith3D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the line in 2D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position Line::positionOf2D(const Point& ref, double prec) const {
	if (ref.isEqual2D(origin, prec))
		return Point::origin;
	if (ref.isEqual2D(end, prec))
		return Point::end;
	auto base = LinEquation::create(*this);
	if (!base)
		return Point::undefined;
	if (!isZero(ref.lengthFrom2D(base->closestPointTo(ref)), prec))
		return Point::undefined;
	auto orig = base->getPerpendicular(ref);
	auto origPos = orig->positionOf(origin, prec);
	if (origPos == Point::along)
		return Point::origin;
	auto endPos = orig->positionOf(end, prec);
	if (endPos == Point::along)
		return Point::end;
	if (endPos != origPos)
		return Point::along;
	return (orig->lengthTo(origin) < orig->lengthTo(end)) ? Point::before : Point::after;
} //Line::positionOf2D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the line in 3D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position Line::positionOf3D(const Point& ref, double prec) const {
	Vector3 vect(end - origin);
	Leveller level(vect);
	Point lineRef(origin), ptRef(ref);
	level.transform(lineRef);
	level.transform(ptRef);
	if (!isZero(lineRef.lengthFrom2D(ptRef)))
		return Point::undefined;
	auto base = Plane::create(origin, vect);
	if (!base)
		return Point::undefined;
	auto basePos = base->positionOf(ref, prec);
	if (basePos != Point::front)
		return (basePos == Point::along) ? Point::origin : Point::before;
	auto top = Plane::create(end, vect);
	if (!top)
		return Point::undefined;
	auto topPos = top->positionOf(ref, prec);
	if (topPos != Point::back)
		return (topPos == Point::along) ? Point::end : Point::after;
	return Point::along;
} //Line::positionOf3D


/*--------------------------------------------------------------------
	Check if two lines are parallel
	
	ref: The reference line
	
	return: True if they are parallel
  --------------------------------------------------------------------*/
bool Line::isParallelTo2D(const Line& ref, double prec) const {
	Vector3 v1(*this), v2(ref);
	v1[2] = v2[2] = 0.0;
	double mod1 = v1.modulus(), mod2 = v2.modulus();
	if (isZero(mod1, prec) || isZero(mod2, prec))
		return true;	//Can't determine if a point is parallel
	if (mod1 < mod2)
		v2 *= mod1 / mod2;
	else
		v1 *= mod2 / mod1;
	return isZero((v1 - v2).modulus(), eps);
} //Line::isParallelTo2D


/*--------------------------------------------------------------------
	Check if two lines are parallel
	
	ref: The reference line
	
	return: True if the lines are parallel
  --------------------------------------------------------------------*/
bool Line::isParallelTo3D(const Line& ref, double prec) const {
	return Vector3(*this).isParallelTo(Vector3(ref));
} //Line::isParallelTo3D


/*--------------------------------------------------------------------
	Determine if a point is enclosed by a line in 2D
	
	ref: The reference point
	
	return: True if the lineline encloses the point
  --------------------------------------------------------------------*/
bool Line::encloses2D(const Point& ref, double prec) const {
	Position p = positionOf2D(ref, prec);
	return ((p == Point::along) || (p == Point::origin) || (p == Point::end));
} //Line::encloses2D


/*--------------------------------------------------------------------
	Determine if a point is enclosed by a line
	
	ref: The reference point
	
	return: True if the lineline encloses the point
  --------------------------------------------------------------------*/
bool Line::encloses3D(const Point& ref, double prec) const {
	auto p = positionOf3D(ref, prec);
	return ((p == Point::along) || (p == Point::origin) || (p == Point::end));
} //Line::encloses3D


/*--------------------------------------------------------------------
	Check if two lines are colinear
	
	ref: The reference line
	
	return: True if the lines are colinear
  --------------------------------------------------------------------*/
bool Line::isColinearTo2D(const Line& ref, double prec) const {
	if (!isParallelTo2D(ref, prec))
		return false;
	return isZero(closestPointTo2D(ref.origin, prec).lengthFrom2D(ref.origin), prec);
} //Line::isColinearTo2D


/*--------------------------------------------------------------------
	Check if two lines are colinear
	
	ref: The reference line
	
	return: True if the lines are colinear
  --------------------------------------------------------------------*/
bool Line::isColinearTo3D(const Line& ref, double prec) const {
	if (!isParallelTo3D(ref, prec))
		return false;
	return isZero(closestPointTo3D(ref.origin, prec).lengthFrom3D(ref.origin), prec);
} //Line::isColinearTo3D


/*--------------------------------------------------------------------
	Get the angle between two lines
	
	ref: The reference line
	
	return: The angle between the lines
  --------------------------------------------------------------------*/
double Line::angleTo2D(const Line& ref) const {
	double angle = ref.azimuthAngle() - azimuthAngle();
	if (angle < 0.0)
		angle += 2.0 * pi;
	return angle;
} //Line::angleTo2D

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Extend the line length by the specified amount
	
	len: The amount to extend the line by
  --------------------------------------------------------------------*/
void Line::extend(double len) {
	end.movePolar(len, azimuthAngle(), altitudeAngle());
} //Line::extend


/*--------------------------------------------------------------------
	Move the line by the specified distance and angle

	len: The distance to move the line
	angle: The angle to move the line along
  --------------------------------------------------------------------*/
void Line::movePolar(double len, double angle) {
	origin.movePolar(len, angle);
	end.movePolar(len, angle);
} //Line::movePolar


/*--------------------------------------------------------------------
	Move the line by the specified distance and azimuth/altitude angles

	len: The distance to move the line
	azim: The azimuth angle to move the line along
	alt: The altitude angle to move the line along
  --------------------------------------------------------------------*/
void Line::movePolar(double len, double azim, double alt) {
	origin.movePolar(len, azim, alt);
	end.movePolar(len, azim, alt);
} //Line::movePolar


/*--------------------------------------------------------------------
	Flip the line, ie reverse its direction
  --------------------------------------------------------------------*/
void Line::flip() {
	Point temp = origin;
	origin = end;
	end = temp;
} //Line::flip

