/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Arc.h"

#include "Active/Geometry/XList.h"
#include "Active/Geometry/Leveller.h"
#include "Active/Geometry/Line.h"
#include "Active/Geometry/LinEquation.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Plane.h"
#include "Active/Geometry/PolyPoint.h"
#include "Active/Geometry/Rotater.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Utility/MathFunctions.h"

#include <algorithm>

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Arc::Arc() : centre{} {
	normal = Vector3{0.0, 0.0, 1.0};
	radius = startAngle = sweep = 0.0;
} //Arc::Arc


/*--------------------------------------------------------------------
	Constructor
	
	centre: Centre of the arc
	start: Start angle of the arc sweep
	sweep: Arc sweep angle
	norm: Normal to the arc plane
  --------------------------------------------------------------------*/
Arc::Arc(const Point& centre, double radius, double sweep, double start, const Vector3& norm) : centre(centre), normal(norm) {
	radius = radius;
	startAngle = start;
	sweep = sweep;
} //Arc::Arc


/*--------------------------------------------------------------------
	Constructor
	
	centre: Centre of the arc
	pt1: First point on the arc
	pt2: Second point on the arc
	isClockwise: True if the arc travels in a clockwise direction from pt1 to pt2
  --------------------------------------------------------------------*/
Arc::Arc(const Point& centre, const Point& pt1, const Point& pt2, bool isClockwise) : centre(centre), normal(Vector3(0, 0, 1)) {
	radius = centre.lengthFrom2D(pt1);
	startAngle = centre.azimuthAngleTo(pt1);
	sweep = centre.azimuthAngleTo(pt2) - startAngle;
	if (isClockwise) {
		if (sweep > 0)
			sweep -= (2 * pi);
	} else {
		if (sweep < 0)
			sweep += (2 * pi);
	}
} //Arc::Arc


/*--------------------------------------------------------------------
	Constructor
	
	pt1: First point on the arc (Note: points must be consecutive)
	pt2: Second point on the arc
	pt3: Third point on the arc
  --------------------------------------------------------------------*/
Arc::Arc(const Point& pt1, const Point& pt2, const Point& pt3) {
	normal = Vector3(0, 0, 1);
	radius = startAngle = sweep = 0;
	if (auto orig = Plane::create(pt1, pt2, pt3); orig) {
		normal = orig->getNormal();
		Line l1(pt1, pt2), l2(pt2, pt3);
		if (normal[2] < 0)
			orig->setNormal(normal * -1);
		Leveller level(orig->getNormal(), 1e-8);
		level.transform(l1);
		level.transform(l2);
		if (auto ref1 = LinEquation::create(l1), ref2 = LinEquation::create(l2); ref1 && ref2) {
			if (auto lin1 = ref1->getPerpendicular(l1.midpoint()), lin2 = ref2->getPerpendicular(l2.midpoint()); lin1 && lin2) {
				if (auto intersect = lin1->intersectionWith(*lin2); intersect) {
					normal = orig->getNormal();
					centre = *intersect;
					radius = centre.lengthFrom2D(l1.origin);
					startAngle = centre.azimuthAngleTo(l1.origin);
					double aEnd = centre.azimuthAngleTo(l2.end);
					sweep = fmod(aEnd - startAngle + (2.0 * pi), (2.0 * pi));
					if (auto lin3 = LinEquation::create(Line(l1.origin, l2.end)); lin3 && (lin3->positionOf(l1.end) == Point::left))
						sweep -= (2.0 * pi);
					level.reverse().transform(centre);
				}
			}
		}
	}
} //Arc::Arc


/*--------------------------------------------------------------------
	Constructor
	
	origin: Origin point of the arc
	end: End point of the arc
	norm: Normal to the plane of the arc
  --------------------------------------------------------------------*/
Arc::Arc(const Point& origin, const PolyPoint& end) : normal(Vector3(0.0, 0.0, 1.0)) {
	if (isZero(end.sweep) || origin.isEqual2D(end))
		radius = startAngle = sweep = 0.0;
	else {
		sweep = end.sweep;
		Line chord(origin, end);
		double angle = fabs(sweep) / 2;
		radius = chord.length2D() / (2 * sin(angle));
		centre = origin;
		double offset = (sweep / fabs(sweep)) * ((pi / 2.0) - angle);
		centre.movePolar(radius, chord.azimuthAngle() + offset);
		startAngle = centre.azimuthAngleTo(chord.origin);
	}
} //Arc::Arc

// MARK: - Operators
		
/*--------------------------------------------------------------------
	Assignment operator

	source: The arc to be assigned to this

	return: A reference to this
  --------------------------------------------------------------------*/
Arc& Arc::operator= (const Arc& source) {
	if (this != &source) {
		centre = source.centre;
		normal = source.normal;
		radius = source.radius;
		startAngle = source.startAngle;
		sweep = source.sweep;
	}
	return *this;
} //Arc::operator=


/*--------------------------------------------------------------------
	Addition operator
	
	offset: The amount to add to the arc
	
	return: An offset arc
  --------------------------------------------------------------------*/
Arc Arc::operator+ (const Point& offset) const {
	return (Arc(*this) += offset);
} //Arc::operator+


/*--------------------------------------------------------------------
	Addition with assignment operator
	
	offset: The amount to add to the arc
	
	return: A reference to this
  --------------------------------------------------------------------*/
Arc& Arc::operator+= (const Point& offset) {
	centre += offset;
	return *this;
} //Arc::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	offset: The amount to subtract from the arc
	
	return: An offset arc
  --------------------------------------------------------------------*/
Arc Arc::operator- (const Point& offset) const {
	return (Arc(*this) -= offset);
} //Arc::operator-


/*--------------------------------------------------------------------
	Subtraction with assignment operator
	
	offset: The amount to subtract from the arc
	
	return: A reference to this
  --------------------------------------------------------------------*/
Arc& Arc::operator-= (const Point& offset) {
	centre -= offset;
	return *this;
} //Arc::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The amount to multiply the arc by
	
	return: A multiplied arc
  --------------------------------------------------------------------*/
Arc Arc::operator* (double scale) const {
	return (Arc(*this) *= scale);
} //Arc::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The amount to multiply the arc by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Arc& Arc::operator*= (double scale) {
	centre *= scale;
	radius *= scale;
	return *this;
} //Arc::operator*=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The matrix to multiply the arc by
	
	return: A multiplied arc
  --------------------------------------------------------------------*/
Arc Arc::operator* (const Matrix3x3& matrix) const {
	return (Arc(*this) *= matrix);
} //Arc::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The matrix to multiply the arc by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Arc& Arc::operator*= (const Matrix3x3& matrix) {
	Point ref = getOrigin();
	ref *= matrix;
	centre *= matrix;
	normal *= matrix;
	radius = centre.lengthFrom3D(ref);
	return *this;
} //Arc::operator*=

// MARK: - Functions (const)
		
/*--------------------------------------------------------------------
	Determine if the arc is valid
	
	return: True if the arc is valid
  --------------------------------------------------------------------*/
double Arc::isValid(double prec) const {
	return (isGreaterZero(radius, prec) && !isZero(sweep, prec) &&
			!normal.isEmpty(prec));
} //Arc::isValid


/*--------------------------------------------------------------------
	Determine if two arcs are equal in 2D
	
	ref: The arc to compare with this
	prec: The required precision
	
	return: True if the arcs are identical
  --------------------------------------------------------------------*/
bool Arc::isEqual2D(const Arc& ref, double prec) const {
	if (centre.isEqual2D(ref.centre, prec) && normal.isParallelTo(ref.normal, prec) &&
			isEqual(radius, ref.radius, prec) && isEqual(fabs(sweep), fabs(ref.sweep), prec)) {
		double min1 = startAngle, min2 = ref.startAngle;
		if (sweep < 0)
			min1 += sweep;
		if (ref.sweep < 0)
			min2 += ref.sweep;
		return (isEqual(min1, min2, prec));
	}
	return false;
} //Arc::isEqual2D


/*--------------------------------------------------------------------
	Determine if two arcs are equal
	
	ref: The arc to compare with this
	prec: The required precision
	
	return: True if the arcs are identical
  --------------------------------------------------------------------*/
bool Arc::isEqual3D(const Arc& ref, double prec) const {
	if (centre.isEqual3D(ref.centre, prec) && normal.isParallelTo(ref.normal, prec) &&
			isEqual(radius, ref.radius, prec) && isEqual(fabs(sweep), fabs(ref.sweep), prec)) {
		double min1 = startAngle, min2 = ref.startAngle;
		if (sweep < 0)
			min1 += sweep;
		if (ref.sweep < 0)
			min2 += ref.sweep;
		return (isEqual(min1, min2, prec));
	}
	return false;
} //Arc::isEqual3D


/*--------------------------------------------------------------------
	Get the origin point of the arc
	
	return: The origin point
  --------------------------------------------------------------------*/
Point Arc::getOrigin() const {
	Point pt(centre);
	pt.movePolar(radius, startAngle);
	Leveller level;
	initLevel(level);
	level.reverse().transform(pt);
	return pt;
} //Arc::getOrigin
/*--------------------------------------------------------------------
	Get the end point of the arc
	
	return: The end point
  --------------------------------------------------------------------*/
PolyPoint Arc::getEnd() const {
	PolyPoint pt(centre, sweep);
	pt.movePolar(radius, startAngle + sweep);
	Leveller level;
	initLevel(level);
	level.reverse().transform(pt);
	return pt;
} //Arc::getEnd


/*--------------------------------------------------------------------
	Get the midpoint of the arc

	return: The centrepoint of the arc
  --------------------------------------------------------------------*/
Point Arc::midpoint() const {
	Leveller level;
	initLevel(level);
	Point mid(centre);
	level.transform(mid);
	mid.movePolar(radius, startAngle + sweep / 2);
	level.reverse().transform(mid);
	return mid;
} //Arc::midpoint


/*--------------------------------------------------------------------
	Get the plane of the arc
	
	return: The plane of the arc
  --------------------------------------------------------------------*/
Plane Arc::getPlane() const {
	auto plane = Plane::create(centre, normal);
	return plane ? Plane() : *plane;
} //Arc::getPlane


/*--------------------------------------------------------------------
	Get the bounds of the arc
	
	return: The bounds of the arc
  --------------------------------------------------------------------*/
Box::Unique Arc::bounds() const {
		//Is the arc level?
	if (normal.isParallelTo(Vector3(0, 0, 1))) {
		Box* bounds = new Box(getOrigin(), getEnd());
		double initAngle = startAngle, endAngle = getEndAngle();
		if (sweep < 0)
			std::swap(initAngle, endAngle);
		if (endAngle < initAngle)
			endAngle += 2.0 * pi;
		double quad = fMod(initAngle + (2.0 * pi), pi / 2.0);
		quad = (isZero(quad)) ? initAngle : (initAngle - quad + (pi / 2.0));
		while (quad < endAngle) {
			bounds->merge(centre + Point(radius * cos(quad), radius * sin(quad)));
			quad += pi / 2.0;
		}
		return Box::Unique(bounds);
	}
		//The bounds of 3D arcs TBC
	return nullptr;
} //Arc::bounds


/*--------------------------------------------------------------------
	Get the edge area (line edges are always zero)
 
 	isArcOnly: True if only the area enclosed by the arc is included (not the wedge to the centre)
	isResultSigned: True if the result should be signed (reflecting the arc angle)
 
	return: The edge area
  --------------------------------------------------------------------*/
double Arc::getArea(bool isArcOnly, bool isResultSigned) const {
		//Area of the sector is a fraction of the area of a circle
	double result = pow(radius, 2) * sweep / 2;
		//Subtract the area of the 'triangle' as required
	if (isArcOnly)
		result -= sgn(result) * (getOrigin().lengthFrom2D(getEnd()) * radius * cos(sweep / 2) / 2);
		//Remove the sign as required
	if (!isResultSigned)
		result = fabs(result);
	return result;
} //Arc::getArea


/*--------------------------------------------------------------------
	Get the 2D perimeter of the arc

	return: The 2D length of the arc
  --------------------------------------------------------------------*/
double Arc::length2D() const {
	return radius * fabs(sweep);	//To be completed (no provision for rotated arcs)
} //Arc::length2D


/*--------------------------------------------------------------------
	Get the length of the arc

	return: The length of the arc
  --------------------------------------------------------------------*/
double Arc::length3D() const {
	return radius * fabs(sweep);
} //Arc::length3D


/*--------------------------------------------------------------------
	Check if two arcs are colinear
	
	ref: The reference arc
	
	return: True if the arcs are colinear
  --------------------------------------------------------------------*/
bool Arc::isColinearTo2D(const Arc& ref, double prec) const {
	return (isParallelTo2D(ref, prec) && isEqual(radius, ref.radius) &&
			(fmod(normal.azimuthAngle(), pi) == fmod(ref.normal.azimuthAngle(), pi)) &&
			(fabs(normal.altitudeAngle()) == fabs(ref.normal.altitudeAngle())));
} //Arc::isColinearTo2D


/*--------------------------------------------------------------------
	Check if two arcs are colinear
	
	ref: The reference arc
	
	return: True if the arcs are colinear
  --------------------------------------------------------------------*/
bool Arc::isColinearTo3D(const Arc& ref, double prec) const {
	return (centre.isEqual3D(ref.centre, prec) && isEqual(radius, ref.radius, prec) &&
			normal.isParallelTo(ref.normal, prec));
} //Arc::isColinearTo3D


/*--------------------------------------------------------------------
	Check if two arcs are concentric
	
	ref: The reference arc
	
	return: True if they are concentric
  --------------------------------------------------------------------*/
bool Arc::isParallelTo2D(const Arc& ref, double prec) const {
	return (centre.isEqual2D(ref.centre, prec));
} //Arc::isParallelTo2D


/*--------------------------------------------------------------------
	Check if two arcs are concentric
	
	ref: The reference arc
	
	return: True if the arcs are concentric
  --------------------------------------------------------------------*/
bool Arc::isParallelTo3D(const Arc& ref, double prec) const {
	return (centre.isEqual3D(ref.centre, prec) && normal.isParallelTo(ref.normal, prec));
} //Arc::isParallelTo3D


/*--------------------------------------------------------------------
	Get the closest point along the arc to a given point in 2D
	
	ref: The reference point
	
	return: The closest point along the arc to the reference point
  --------------------------------------------------------------------*/
Point Arc::closestPointAlong2D(const Point& ref, double prec) const {
	if (!isValid(prec) || ref.isEqual2D(centre, prec))
		return getOrigin();
	XInfo spec1(Point::all), spec2;
	XList inter(spec1, spec2);
	if (intersectionWith2D(Line(centre, ref), inter, prec) == 0)
		return getOrigin();
	auto last = inter.begin();
	auto first = last++;
	if ((inter.size() > 1) && (ref.lengthFrom2D(**first) > ref.lengthFrom2D(**last)))
		++first;
	if (((*first)->getPos(XPoint::target) & within) != undefined)
		return **first;
	Point orig = getOrigin(), end = getEnd();
	return (orig.lengthFrom2D(**first) < end.lengthFrom2D(**first)) ? orig : end;
} //Arc::closestPointAlong2D


/*--------------------------------------------------------------------
	Get the closest point along the arc to a given point
	
	ref: The reference point
	
	return: The closest point along the arc to the reference point
  --------------------------------------------------------------------*/
Point Arc::closestPointAlong3D(const Point& ref, double prec) const {
		//To be completed
	return closestPointAlong2D(ref);
} //Arc::closestPointAlong3D


/*--------------------------------------------------------------------
	Get the closest point in the arc to a given point in 2D
	
	ref: The reference point
	
	return: The closest point in the arc to the reference point
  --------------------------------------------------------------------*/
Point Arc::closestPointTo2D(const Point& ref, double prec) const {
	return closestPointTo3D(ref, prec);
} //Arc::closestPointTo2D


/*--------------------------------------------------------------------
	Get the closest point in the arc to a given point
	
	ref: The reference point
	
	return: The closest point in the arc to the reference point
  --------------------------------------------------------------------*/
Point Arc::closestPointTo3D(const Point& ref, double prec) const {
	Leveller level;
	initLevel(level);
	Point ctr(centre), pt(ref);
	level.transform(ctr);
	level.transform(pt);
	double angle = ctr.azimuthAngleTo(pt);
	ctr += Point(radius * cos(angle), radius * sin(angle));
	level.reverse().transform(ctr);
	return ctr;
} //Arc::closestPointTo3D


/*--------------------------------------------------------------------
	Get the intersection point of an arc and plane in 3D
	
	ref: The reference plane
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWith(const Plane& ref, XList& inter, double prec) const {
	vertex_index interCount = 0;
	XInfo info(inter.getFilter(XPoint::blade)), none;
	inter.setFilter(XPoint::blade, none);	//All intersection points will be on the plane
	auto lin = ref.intersectionWith(getPlane(), prec);
	if (!lin)
		interCount = intersectionWith3D(*lin, inter, prec);
	inter.setFilter(XPoint::blade, info);
	return interCount;
} //Arc::intersectionWith


/*--------------------------------------------------------------------
	Get the intersection point of an arc and line in 2D
	
	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWith2D(const Line& ref, XList& inter, double prec) const {
	auto myPlane = Plane::create(centre, normal);
	if (!myPlane)
		return 0;
	auto orig = myPlane->intersectionWith(Line(ref.origin, ref.origin + Point(0, 0, 1))),
		end = myPlane->intersectionWith(Line(ref.end, ref.end + Point(0, 0, 1)));
	if (!orig || !end)
		return 0;
	return intersectionWithLevel(Line(*orig, *end), inter, prec);
} //Arc::intersectionWith2D


/*--------------------------------------------------------------------
	Get the intersection between this and a specified arc

	return: The projected intersection between the two arcs
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWith2D(const Arc& ref, XList& inter, double prec) const {
	if (centre.isEqual2D(ref.centre, prec))
		return 0;
	auto plane1 = Plane::create(centre, normal), plane2 = Plane::create(ref.centre, ref.normal);
	if (!plane1 || !plane2)
		return 0;
	if (!plane1->isParallelTo(*plane2))
		return 0;	//To be completed - this instance is not calculated yet
	auto ref1 = clone(*this);
	if (!isEqual(plane1->getOffset(), plane2->getOffset(), prec)) {
			//Offset the reference circle plane to coincide with this
		if (auto ctr = plane2->intersectionWith(Line(centre, centre + Point(0, 0, 1))); ctr)
			ref1->centre = *ctr;
	}
	return ref1->intersectionWithLevel(ref, inter, prec);
} //Arc::intersectionWith2D


/*--------------------------------------------------------------------
	Get the intersection point of an arc and line in 3D

	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision

	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWith3D(const Line& ref, XList& inter, double prec) const {
	vertex_index totalInt = 0;
	if (normal.isPerpendicularTo(Vector3(ref), prec))
		totalInt = intersectionWithLevel(ref, inter, prec);
	else {
		auto plane = Plane::create(centre, normal);
		if (!plane)
			return 0;
		auto intersect = plane->intersectionWith(ref, prec);
		auto pos = positionOf3D(*intersect);
		if ((pos == undefined) || (pos == outside))
			return 0;
		intersect->setPos(XPoint::target, pos);
		if (inter.isPos(XPoint::blade))
			intersect->setPos(XPoint::blade, ref.positionOf3D(*intersect));
		if (inter.insert(std::move(*intersect)))
			++totalInt;
	}
	return totalInt;
} //Arc::intersectionWith3D


/*--------------------------------------------------------------------
	Get the intersection between this and a specified arc in 3D
	
	ref: The reference arc
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWith3D(const Arc& ref, XList& inter, double prec) const {
	if (centre.isEqual3D(ref.centre, prec))
		return 0;
		//Create the planes along which the arcs lie
	auto p1 = Plane::create(centre, normal), p2 = Plane::create(ref.centre, ref.normal);
	if (!p1 || !p2)
		return 0;
	vertex_index totalInt = 0;
		//Check if the planes are parallel
	if (p1->isParallelTo(*p2)) {
		if (!isEqual(p1->getOffset(), p2->getOffset(), prec))
			return 0;
		totalInt = intersectionWithLevel(ref, inter, prec);
	} else {
		auto lin = p1->intersectionWith(*p2, prec);
		if (!lin)
			return 0;
		XInfo temp(inter.getFilter(XPoint::blade)), myInfo(inter.getFilter(XPoint::blade));
		myInfo.pos = undefined;	//We don't care how the intersection relates to the line
		inter.setFilter(XPoint::blade, myInfo);
		vertex_index totalInt = intersectionWithLevel(*lin, inter, prec);
		inter.setFilter(XPoint::blade, temp);
		auto i = inter.begin();
			//Check that the intersections are on the reference arc
		for (auto n = totalInt; n--; ) {
			(**i).setPos(XPoint::blade, ref.positionOf3D(**i));
			if (inter.withPos(XPoint::blade, (**i).getPos(XPoint::blade)))
				++i;
			else {
				i = inter.erase(i);
				--totalInt;
			}
		}
	}
	return totalInt;
} //Arc::intersectionWith3D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the arc in 2D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position Arc::positionOf2D(const Point& ref, double prec) const {
	if (!isValid(prec))
		return Point::undefined;
	double span = centre.lengthFrom2D(ref);
	if (isGreater(span, radius, prec))
		return Point::outside;
	if (isLess(span, radius, prec))
		return Point::inside;
	if (ref.isEqual2D(getOrigin(), prec))
		return Point::origin;
	if (ref.isEqual2D(getEnd(), prec))
		return Point::end;
	double angle = centre.azimuthAngleTo(ref), start = startAngle, sweepAngle = sweep;
	if (sweepAngle < 0) {
		start = fmod(start + sweepAngle + (2.0 * pi), (2.0 * pi));
		sweepAngle = -sweepAngle;
	}
	if (angle < start)
		angle += (2.0 * pi);
	prec /= radius;	//The precision along the circumference has to be converted to an angle increment
	return (isWithin(angle, start, start + sweepAngle, prec)) ? Point::along : Point::radial;
} //Arc::positionOf2D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the arc in 3D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position Arc::positionOf3D(const Point& ref, double prec) const {
	if (!isValid(prec))
		return undefined;
	auto base = Plane::create(centre, normal);
	if (!base)
		return undefined;
	if (!isZero(base->lengthTo(ref), prec))
		return undefined;
	return positionOf2D(ref, prec);
} //Arc::positionOf3D


/*--------------------------------------------------------------------
	Determine if a point is enclosed by an arc in 2D
	
	ref: The reference point
	
	return: True if the arc encloses the point
  --------------------------------------------------------------------*/
bool Arc::encloses2D(const Point& ref, double prec) const {
	Position p = positionOf2D(ref, prec);
	return ((p == Point::inside) || (p == Point::along) || (p == Point::origin) || (p == Point::end));
} //Arc::encloses2D


/*--------------------------------------------------------------------
	Determine if a point is enclosed by an arc
	
	ref: The reference point
	
	return: True if the arc encloses the point
  --------------------------------------------------------------------*/
bool Arc::encloses3D(const Point& ref, double prec) const {
	auto base = Plane::create(centre, normal);
	if (!base)
		return false;
	if (!isZero(base->lengthTo(ref), prec))
		return false;
	return isLessOrEqual(centre.lengthFrom3D(ref), radius, prec);
} //Arc::encloses3D

// MARK: - Functions (mutating)
		
/*--------------------------------------------------------------------
	Move the arc by the specified distance and angle

	len: The distance to move the arc
	angle: The angle to move the arc along
  --------------------------------------------------------------------*/
void Arc::movePolar(double len, double angle) {
	centre.movePolar(len, angle);
} //Arc::movePolar


/*--------------------------------------------------------------------
	Move the arc by the specified distance and azimuth/altitude angles

	len: The distance to move the arc
	azim: The azimuth angle to move the arc along
	alt: The altitude angle to move the arc along
  --------------------------------------------------------------------*/
void Arc::movePolar(double len, double azim, double alt) {
	centre.movePolar(len, azim, alt);
} //Arc::movePolar


/*--------------------------------------------------------------------
	Flip the arc, ie reverse its direction
  --------------------------------------------------------------------*/
void Arc::flip() {
	startAngle += sweep;
	sweep = -sweep;
} //Arc::flip

// MARK: - Internal implementation

/*--------------------------------------------------------------------
	Initialise a Leveller to transform the arc to flat plane
	
	level: The Leveller to be initialised
	prec: The required precision
  --------------------------------------------------------------------*/
void Arc::initLevel(Leveller& level, double prec) const {
	level.setRotation(normal, prec);
} //Arc::initLevel


/*--------------------------------------------------------------------
	Create an intersection point
	
	pt: The intersection point
	arc: The source arc
	ref: The reference arc
	inter: The intersection list to populate
	prec: The required precision
  --------------------------------------------------------------------*/
bool Arc::createIntersect(const Point& pt, const Arc& arc, const Arc& ref, XList& inter, double prec) const {
	Position atSrc = Point::undefined, atRef = Point::undefined;
	if (inter.isPos(XPoint::target))
		atSrc = arc.positionOf2D(pt, prec);
	if (inter.isPos(XPoint::blade))
		atRef = ref.positionOf2D(pt, prec);
	return inter.insert(XPoint(pt, atSrc, atRef));
} //Arc::createIntersect


/*--------------------------------------------------------------------
	Create an intersection point
	
	pt: The intersection point
	arc: The source arc
	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision
  --------------------------------------------------------------------*/
bool Arc::createIntersect(const Point& pt, const Arc& arc, const Line& ref, XList& inter, double prec) const {
	Position atSrc = Point::undefined, atRef = Point::undefined;
	if (inter.isPos(XPoint::target))
		atSrc = arc.positionOf2D(pt, prec);
	if (inter.isPos(XPoint::blade))
		atRef = ref.positionOf2D(pt, prec);
	return inter.insert(XPoint(pt, atSrc, atRef));
} //Arc::createIntersect


/*--------------------------------------------------------------------
	Get the intersection point between a co-planar line and arc
	
	ref: The reference line
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWithLevel(const Line& ref, XList& inter, double prec) const {
	if (!isValid(prec))
		return 0;
		//Transform the arc/line onto a horizonal plane
	Leveller level;
	initLevel(level);
	auto refArc = clone(*this);
	auto refLin(ref);
	level.transform(*refArc);
	level.transform(refLin);
	Point base = refLin.closestPointTo2D(refArc->centre);
	base.z = centre.z;
	double span = refArc->centre.lengthFrom2D(base);
	vertex_index totalInt = 0;
	if (isGreater(span, refArc->radius, prec))
		return 0;
	else if (isEqual(span, refArc->radius, prec * 1e-2)) {
		if (createIntersect(base, *refArc, refLin, inter, prec))
			++totalInt;
	} else  {
		double angle, refAngle;
		if (isZero(span, prec)) {
			angle = pi / 2.0;
			refAngle = refLin.azimuthAngle() + angle;
		} else {
			angle = acos(span / refArc->radius);
			refAngle = refArc->centre.azimuthAngleTo(base);
		}
		Point intPt(refArc->centre);
		intPt.movePolar(refArc->radius, refAngle + angle);
		if (createIntersect(intPt, *refArc, refLin, inter, prec))
			++totalInt;
		intPt = refArc->centre;
		intPt.movePolar(refArc->radius, refAngle - angle);
		if (createIntersect(intPt, *refArc, refLin, inter, prec))
			++totalInt;
	}
	level.reverse();
	auto i = inter.begin();
	for (auto n = totalInt; n--; ++i)
		level.transform(**i);
	return totalInt;
} //Arc::intersectionWithLevel


/*--------------------------------------------------------------------
	Get the intersection point between two co-planar arcs
	
	ref: The reference arc
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index Arc::intersectionWithLevel(const Arc& ref, XList& inter, double prec) const {
		//Transform the arcs onto a horizonal plane
	Leveller level;
	initLevel(level);
	auto arc1 = clone(*this), arc2 = clone(ref);
	level.transform(*arc1);
	level.transform(*arc2);
	double span = arc1->centre.lengthFrom2D(arc2->centre);
	if (isZero(span, prec) || isGreater(span, arc1->radius + arc2->radius, prec) ||
			isLess(span, fabs(arc1->radius - arc2->radius)))
		return 0;
	double angle = arc1->centre.azimuthAngleTo(arc2->centre);
	Point mid(arc1->centre);
	vertex_index totalInt = 0;
	bool equalIn = isEqual(span, fabs(arc1->radius - arc2->radius), prec);
	if (isEqual(span, arc1->radius + arc2->radius, prec) || equalIn) {
		if (equalIn && (arc1->radius < arc2->radius))
			angle += pi;
		mid.movePolar(span, angle);
		if (createIntersect(mid, *arc1, *arc2, inter, prec))
			++totalInt;
	} else {
		double offset = (span * span - arc2->radius * arc2->radius +
				arc1->radius * arc1->radius) / (2 * span);
		double inc = acos(offset / arc1->radius);
		mid.movePolar(arc1->radius, angle + inc);
		if (createIntersect(mid, *arc1, *arc2, inter, prec))
			++totalInt;
		mid = arc1->centre;
		mid.movePolar(arc1->radius, angle - inc);
		if (createIntersect(mid, *arc1, *arc2, inter, prec))
			++totalInt;
	}
	level.reverse();
	auto i = inter.begin();
	for (auto n = totalInt; n--; ++i)
		level.transform(**i);
	return totalInt;
} //Arc::intersectionWithLevel
