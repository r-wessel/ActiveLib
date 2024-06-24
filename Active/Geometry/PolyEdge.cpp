/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/PolyEdge.h"

#include <algorithm>

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/XList.h"
#include "Active/Geometry/Leveller.h"
#include "Active/Geometry/LinEquation.h"
#include "Active/Geometry/Matrix3x3.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors
		
/*--------------------------------------------------------------------
	Constructor
	
	origin: Origin point of the edge
	end: End point of the edge
  --------------------------------------------------------------------*/
PolyEdge::PolyEdge(const Point& origin, const PolyPoint& end) : origin{origin}, end{end} {
} //PolyEdge::PolyEdge


/*--------------------------------------------------------------------
	Constructor
 
	origin: Origin point of the edge
	end: End point of the edge
	radius: The edge radius (sign indicates arc centre side (+ve = right, -ve = left, 0 = linear)
	rotation: The rotation direction of the edge (undefined = use existing)
	prec: The required precision
  --------------------------------------------------------------------*/
PolyEdge::PolyEdge(const Point& origin, const Point& end, double radius, Rotation rotation, double prec) : origin{origin}, end{end} {
	if (!isZero(radius, prec) && !origin.isEqual2D(end))
		setRadius(radius, rotation, prec);
} //PolyEdge::PolyEdge


/*--------------------------------------------------------------------
	Constructor
 
	arc: An arc describing the poly-edge
  --------------------------------------------------------------------*/
PolyEdge::PolyEdge(const Arc& arc) : origin{arc.getOrigin()}, end{arc.getEnd()} {
} //PolyEdge::PolyEdge
		
// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment operator

	source: The edge to be assigned to this

	return: A reference to this
  --------------------------------------------------------------------*/
PolyEdge& PolyEdge::operator= (const PolyEdge& source) {
	if (this != &source) {
		origin = source.origin;
		end = source.end;
	}
	return *this;
} //PolyEdge::operator=


/*--------------------------------------------------------------------
	Addition operator
	
	offset: The amount to add to the edge
	
	return: An offset edge
  --------------------------------------------------------------------*/
PolyEdge PolyEdge::operator+ (const Point& offset) const {
	return (PolyEdge(*this) += offset);
} //PolyEdge::operator+


/*--------------------------------------------------------------------
	Addition with assignment operator
	
	offset: The amount to add to the edge
	
	return: A reference to this
  --------------------------------------------------------------------*/
PolyEdge& PolyEdge::operator+= (const Point& offset) {
	origin += offset;
	end += offset;
	return *this;
} //PolyEdge::operator+=


/*--------------------------------------------------------------------
	Subtraction operator
	
	offset: The amount to subtract from the edge
	
	return: An offset edge
  --------------------------------------------------------------------*/
PolyEdge PolyEdge::operator- (const Point& offset) const {
	return (PolyEdge(*this) -= offset);
} //PolyEdge::operator-


/*--------------------------------------------------------------------
	Subtraction with assignment operator
	
	offset: The amount to subtract from the edge
	
	return: A reference to this
  --------------------------------------------------------------------*/
PolyEdge& PolyEdge::operator-= (const Point& offset) {
	origin -= offset;
	end -= offset;
	return *this;
} //PolyEdge::operator-=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The amount to multiply the edge by
	
	return: A multiplied edge
  --------------------------------------------------------------------*/
PolyEdge PolyEdge::operator* (double scale) const {
	return (PolyEdge(*this) *= scale);
} //PolyEdge::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The amount to multiply the edge by
	
	return: A reference to this
  --------------------------------------------------------------------*/
PolyEdge& PolyEdge::operator*= (double scale) {
	origin *= scale;
	end *= scale;
	return *this;
} //PolyEdge::operator*=


/*--------------------------------------------------------------------
	Multiplication operator
	
	scale: The matrix to multiply the edge by
	
	return: A multiplied edge
  --------------------------------------------------------------------*/
PolyEdge PolyEdge::operator* (const Matrix3x3& matrix) const {
	return (PolyEdge(*this) *= matrix);
} //PolyEdge::operator*


/*--------------------------------------------------------------------
	Multiplication with assignment operator
	
	scale: The matrix to multiply the edge by
	
	return: A reference to this
  --------------------------------------------------------------------*/
PolyEdge& PolyEdge::operator*= (const Matrix3x3& matrix) {
	origin *= matrix;
	end *= matrix;
	return *this;
} //PolyEdge::operator*=

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if two edges are equal in 2D
	
	ref: The edge to compare with this
	prec: The required precision
	
	return: True if the edges are identical
  --------------------------------------------------------------------*/
bool PolyEdge::isEqual2D(const PolyEdge& ref, double prec) const {
	return (((origin.isEqual2D(ref.origin, prec)) && end.isEqual2D(ref.end, prec) && isEqual(end.sweep, ref.end.sweep, prec)) ||
			(end.isEqual2D(ref.origin, prec) && origin.isEqual2D(ref.end, prec) && isEqual(end.sweep, -ref.end.sweep, prec)));
} //PolyEdge::isEqual


/*--------------------------------------------------------------------
	Determine if two edges are equal
	
	ref: The edge to compare with this
	prec: The required precision
	
	return: True if the edges are identical
  --------------------------------------------------------------------*/
bool PolyEdge::isEqual3D(const PolyEdge& ref, double prec) const {
	return (((origin.isEqual3D(ref.origin)) && end.isEqual3D(ref.end)) ||
			((end.isEqual3D(ref.origin) && origin.isEqual3D(ref.end)) &&
			isEqual(end.sweep, ref.end.sweep, prec)));
} //PolyEdge::isEqual3D


/*--------------------------------------------------------------------
	Check if two edges are parallel
	
	ref: The reference edge
	
	return: True if they are parallel
  --------------------------------------------------------------------*/
bool PolyEdge::isParallelTo2D(const PolyEdge& ref, double prec) const {
	bool result = false;
	if (end.isArc()) {
		if (ref.isArc())
			result = Arc(origin, end).isParallelTo2D(Arc(ref.origin, ref.end), prec);
	} else if (!ref.isArc())
		return Line(origin, end).isParallelTo2D(Line(ref.origin, ref.end), prec);
	return result;
} //PolyEdge::isParallelTo2D


/*--------------------------------------------------------------------
	Check if two edges are parallel
	
	ref: The reference edge
	
	return: True if the edges are parallel
  --------------------------------------------------------------------*/
bool PolyEdge::isParallelTo3D(const PolyEdge& ref, double prec) const {
	bool result = 0;
	if (end.isArc()) {
		if (ref.isArc())
			result = Arc(origin, end).isParallelTo3D(Arc(ref.origin, ref.end), prec);
	} else if (!ref.isArc())
		result = Line(origin, end).isParallelTo3D(Line(ref.origin, ref.end), prec);
	return result;
} //PolyEdge::isParallelTo3D


/*--------------------------------------------------------------------
	Check if two edges are colinear
	
	ref: The reference edge
	
	return: True if the edges are colinear
  --------------------------------------------------------------------*/
bool PolyEdge::isColinearTo2D(const PolyEdge& ref, double prec) const {
	if (!isParallelTo2D(ref, prec))
		return false;
	return isZero(closestPointTo2D(ref.origin, prec).lengthFrom2D(ref.origin), prec);
} //PolyEdge::isColinearTo2D


/*--------------------------------------------------------------------
	Check if two edges are colinear
	
	ref: The reference edge
	
	return: True if the edges are colinear
  --------------------------------------------------------------------*/
bool PolyEdge::isColinearTo3D(const PolyEdge& ref, double prec) const {
	if (!isParallelTo3D(ref, prec))
		return false;
	return isZero(closestPointTo3D(ref.origin, prec).lengthFrom3D(ref.origin), prec);
} //PolyEdge::isColinearTo3D


/*--------------------------------------------------------------------
	Check if two edges are tangential (i.e. check that the end tangent of the reference edge is parallel to the start tangent of this edge)
	
	ref: The reference edge
 	prec: The coordinate precision
	anglePrec: The angle precision
	
	return: True if the edges are tangential
  --------------------------------------------------------------------*/
bool PolyEdge::isTangentialTo2D(const PolyEdge& ref, double prec, double anglePrec) const {
		//The end of ref must be the origin of this edge
	if (!origin.isEqual2D(ref.end))
		return false;
	return isEqualAngle(ref.endTangent(), startTangent(), anglePrec);
} //PolyEdge::isTangentialTo2D


/*--------------------------------------------------------------------
	Get the radius of the edge
 
	isSigned: True if the radius should be signed for the side of arc centres (left = -ve, right = +ve)
 
	return: The radius of the edge (0 = straight edge)
  --------------------------------------------------------------------*/
double PolyEdge::getRadius(bool isSigned) const {
	if (!isArc())
		return 0.0;
	Arc arc(origin, end);
	double result = arc.radius;
		//Negative radius when the arc centre is to the left
	if (isSigned && (LinEquation(azimuthAngle(), origin).positionOf(arc.centre) == Point::left))
		result = -result;
	return result;
} //PolyEdge::getRadius


/*--------------------------------------------------------------------
	Get the 2D length of the edge

	return: The 2D length of the edge
  --------------------------------------------------------------------*/
double PolyEdge::length2D() const {
	return isArc() ? Arc(origin, end).length2D() : end.lengthFrom2D(origin);
} //PolyEdge::length2D


/*--------------------------------------------------------------------
	Get the length of the edge

	return: The length of the edge
  --------------------------------------------------------------------*/
double PolyEdge::length3D() const {
	return isArc() ? Arc(origin, end).length3D() : end.lengthFrom3D(origin);
} //PolyEdge::length3D


/*--------------------------------------------------------------------
	Get the edge as an arc
 
	prec: The required precision
 
	return: The edge as an arc (nullopt if the edge is not an arc)
  --------------------------------------------------------------------*/
Arc::Option PolyEdge::asArc(double prec) const {
	return isArc(prec) ? std::make_optional<Arc>(origin, end) : std::nullopt;
} //PolyEdge::asArc


/*--------------------------------------------------------------------
	Get the azimuth angle of the edge
	
	return: The azimuth angle
  --------------------------------------------------------------------*/
double PolyEdge::azimuthAngle() const {
	return origin.azimuthAngleTo(end);
} //PolyEdge::azimuthAngle


/*--------------------------------------------------------------------
	Get the altitude angle of the edge
	
	return: The altitude angle
  --------------------------------------------------------------------*/
double PolyEdge::altitudeAngle() const {
	return origin.altitudeAngleTo(end);
} //PolyEdge::altitudeAngle


/*--------------------------------------------------------------------
	Get the tangent angle at the edge start
	
	return: The start tangent angle
  --------------------------------------------------------------------*/
double PolyEdge::startTangent() const {
	return getTangentAt(origin);
} //PolyEdge::startTangent


/*--------------------------------------------------------------------
	Get the tangent angle at the edge end
	
	return: The end tangent angle
  --------------------------------------------------------------------*/
double PolyEdge::endTangent() const {
	return getTangentAt(end);
} //PolyEdge::endTangent


/*--------------------------------------------------------------------
	Get the tangent angle at the specified point
 
	ref: The reference point
 
	return: The tangent angle at the specified point
  --------------------------------------------------------------------*/
double PolyEdge::getTangentAt(const Point& ref) const {
	if (auto arc = asArc(); arc)
		return arc->centre.azimuthAngleTo(ref) + ((arc->sweep < 0) ? -(pi / 2.0) : (pi / 2.0));
	return azimuthAngle();
} //PolyEdge::getTangentAt


/*--------------------------------------------------------------------
	Get the center of the edge
	
	return: The edge centre (centre of arc or midpoint of straight line)
  --------------------------------------------------------------------*/
Point PolyEdge::centre() const {
	return (end.isArc()) ? Arc(origin, end).centre : Line(origin, end).midpoint();
} //PolyEdge::centre


/*--------------------------------------------------------------------
	Get the midpoint of the edge

	return: The centrepoint of the edge
  --------------------------------------------------------------------*/
Point PolyEdge::midpoint() const {
	return (end.isArc()) ? Arc(origin, end).midpoint() : Line(origin, end).midpoint();
} //PolyEdge::midpoint


/*--------------------------------------------------------------------
	Get the edge area (line edges are always zero)
 
	isResultSigned: True if the result should be signed (reflecting the arc angle)
 
	return: The edge area
  --------------------------------------------------------------------*/
double PolyEdge::getArea(bool isResultSigned) const {
	return (isArc()) ? Arc(origin, end).getArea(true, isResultSigned) : 0;
} //PolyEdge::getArea


/*--------------------------------------------------------------------
	Get the closest point in the edge to a given point in 2D
	
	ref: The reference point
	
	return: The closest point in the edge to the reference point
  --------------------------------------------------------------------*/
Point PolyEdge::closestPointTo2D(const Point& ref, double prec) const {
	return (end.isArc()) ? Arc(origin, end).closestPointTo2D(ref, prec) : Line(origin, end).closestPointTo2D(ref, prec);
} //PolyEdge::closestPointTo2D


/*--------------------------------------------------------------------
	Get the closest point in the edge to a given point
	
	ref: The reference point
	
	return: The closest point in the edge to the reference point
  --------------------------------------------------------------------*/
Point PolyEdge::closestPointTo3D(const Point& ref, double prec) const {
	return (end.isArc()) ? Arc(origin, end).closestPointTo3D(ref, prec) : Line(origin, end).closestPointTo3D(ref, prec);
} //PolyEdge::closestPointTo3D


/*--------------------------------------------------------------------
	Get the closest point along the edge to a given point in 2D
	
	ref: The reference point
	
	return: The closest point along the edge to the reference point
  --------------------------------------------------------------------*/
Point PolyEdge::closestPointAlong2D(const Point& ref, double prec) const {
	return (end.isArc()) ? Arc(origin, end).closestPointAlong2D(ref, prec) : Line(origin, end).closestPointAlong2D(ref, prec);
} //PolyEdge::closestPointAlong2D


/*--------------------------------------------------------------------
	Get the closest point along the edge to a given point
	
	ref: The reference point
	
	return: The closest point along the edge to the reference point
  --------------------------------------------------------------------*/
Point PolyEdge::closestPointAlong3D(const Point& ref, double prec) const {
	return (end.isArc()) ? Arc(origin, end).closestPointAlong3D(ref, prec) : Line(origin, end).closestPointAlong3D(ref, prec);
} //PolyEdge::closestPointAlong3D


/*--------------------------------------------------------------------
	Get the intersection between this and a specified edge
	
	ref: The reference edge
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index PolyEdge::intersectionWith2D(const PolyEdge& ref, XList& inter, double prec) const {
	if (auto arc = asArc(); arc) {
		if (auto refArc = ref.asArc(); refArc)
			return arc->intersectionWith2D(*refArc, inter, prec);
		else
			return arc->intersectionWith2D(Line(ref.origin, ref.end), inter, prec);
	}
	if (auto refArc = ref.asArc(); refArc) {
		inter.swapFilters();
		auto result = refArc->intersectionWith2D(Line(origin, end), inter, prec);
		inter.swapFilters();
		return result;
	}
	return Line(origin, end).intersectionWith2D(Line(ref.origin, ref.end), inter, prec);
} //PolyEdge::intersectionWith2D


/*--------------------------------------------------------------------
	Get the intersection between this and a specified edge in 3D
	
	ref: The reference edge
	inter: The intersection list to populate
	prec: The required precision
	
	return: The number of intersections calculated
  --------------------------------------------------------------------*/
vertex_index PolyEdge::intersectionWith3D(const PolyEdge& ref, XList& inter, double prec) const {
	if (auto arc = asArc(); arc) {
		if (auto refArc = ref.asArc(); refArc)
			return arc->intersectionWith3D(*refArc, inter, prec);
		else
			return arc->intersectionWith3D(Line(ref.origin, ref.end), inter, prec);
	}
	if (auto refArc = ref.asArc(); refArc) {
		inter.swapFilters();
		auto result = refArc->intersectionWith3D(Line(origin, end), inter, prec);
		inter.swapFilters();
		return result;
	}
	return Line(origin, end).intersectionWith3D(Line(ref.origin, ref.end), inter, prec);
} //PolyEdge::intersectionWith3D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the edge in 2D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position PolyEdge::positionOf2D(const Point& ref, double prec) const {
	return (end.isArc()) ? Arc(origin, end).positionOf2D(ref, prec) : Line(origin, end).positionOf2D(ref, prec);
} //PolyEdge::positionOf2D


/*--------------------------------------------------------------------
	Determine the relationship of a point to the edge in 3D
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point
  --------------------------------------------------------------------*/
Position PolyEdge::positionOf3D(const Point& ref, double prec) const {
	return (end.isArc()) ? Arc(origin, end).positionOf3D(ref, prec) : Line(origin, end).positionOf3D(ref, prec);
} //PolyEdge::positionOf3D


/*--------------------------------------------------------------------
	Determine if a point is enclosed by a edge in 2D
	
	ref: The reference point
	
	return: True if the edge encloses the point
  --------------------------------------------------------------------*/
bool PolyEdge::encloses2D(const Point& ref, double prec) const {
	Position p = positionOf2D(ref, prec);
	return ((p == Point::along) || (p == Point::origin) || (p == Point::end));
} //PolyEdge::encloses2D


/*--------------------------------------------------------------------
	Determine if a point is enclosed by a edge
	
	ref: The reference point
	
	return: True if the edge encloses the point
  --------------------------------------------------------------------*/
bool PolyEdge::encloses3D(const Point& ref, double prec) const {
	Position p = positionOf3D(ref, prec);
	return ((p == Point::along) || (p == Point::origin) || (p == Point::end));
} //PolyEdge::encloses3D


/*--------------------------------------------------------------------
	Determine if this edge is overlapped by a reference edge (a touching vertex is not an overlap)
 
	ref: The reference edge
	prec: The required precision
 
	return: True if the edge overlaps the reference edge
  --------------------------------------------------------------------*/
bool PolyEdge::overlaps2D(const PolyEdge& ref, double prec) const {
		//An arc and a line never overlap
	if (isArc() != ref.isArc())
		return false;
	const auto* major = this;
	const auto* minor = &ref;
	if (major->length2D() < minor->length2D())
		std::swap(major, minor);
	return ((major->positionOf2D(minor->midpoint()) == Point::along) ||
			(major->positionOf2D(minor->origin) == Point::along) ||
			(major->positionOf2D(minor->end) == Point::along));
} //PolyEdge::overlaps2D
		
// MARK: - Functions (mutating)
		
/*--------------------------------------------------------------------
	Set the edge radius
 
	radius: The edge radius (sign indicates arc centre side (+ve = right, -ve = left, 0 = linear)
	rotation: The rotation direction of the edge (undefined = use existing, default = anticlockwise)
	prec: The required precision
  --------------------------------------------------------------------*/
void PolyEdge::setRadius(double radius, std::optional<Rotation> rotation, double prec) {
	double sweepAngle = 0.0;
	if (!isZero(radius, prec)) {
		if (!rotation) {
			if (isZero(end.sweep))
				rotation = anticlockwise;
			else
				rotation = (end.sweep > 0) ? anticlockwise : clockwise;
		}
		double span = origin.lengthFrom2D(end);
		if (isLessOrEqual(span, fabs(2 * radius))) {
			if (isEqual(span, fabs(2 * radius)))
				sweepAngle = (rotation == clockwise) ? pi : -pi;
			else {
				Point centre{span / 2, -sgn(radius) * sqrt(pow(radius, 2) - pow(span / 2, 2))};
				sweepAngle = Arc{centre, Point{}, Point{span, 0}, rotation == clockwise}.sweep;
			}
		}
	}
	end.sweep = sweepAngle;
} //PolyEdge::setRadius


/*--------------------------------------------------------------------
	Stretch the origin point for this wall to the specified point (maintaining reference)
	
	origin: The point to stretch to
	
 	return: True if the origin was stretched.
  --------------------------------------------------------------------*/
void PolyEdge::stretchOrigin(const Point& pt, bool canInvert, double prec) {
	auto projected = closestPointTo2D(pt);
	auto pos = positionOf2D(projected);
	if (isArc()) {
		switch (pos) {
			case Point::radial: {
				Arc arc(projected, origin, end);
				origin = arc.getOrigin();
				end = arc.getEnd();
				break;
			}
			case Point::along: {
				Arc arc(end, origin, projected);
				Point mid(arc.midpoint());
				mid.movePolar(2 * mid.lengthFrom2D(arc.centre), mid.azimuthAngleTo(arc.centre));
				Arc arc2(projected, mid, end);
				origin = arc2.getOrigin();
				end = arc2.getEnd();
				break;
			}
			default:
				break;	//Not interested in other values
		}
	} else {
		if (canInvert) {
			switch (pos) {
				case Point::along: case Point::before:
					origin = projected;
					break;
				case Point::after:
					origin = end;
					end = projected;
					break;
				default:
					break;	//Not interested in other values
			}
		} else
			origin = projected;
	}
} //PolyEdge::stretchOrigin


/*--------------------------------------------------------------------
	Stretch the end point for this wall to the specified point (maintaining reference)
	
	end: The point to stretch to
  --------------------------------------------------------------------*/
void PolyEdge::stretchEnd(const Point& end, double prec) {
	flip();
	stretchOrigin(end, false, prec);
	flip();
} //PolyEdge::stretchEnd


/*--------------------------------------------------------------------
	Set the level (z coordinate) of the edge vertices

	z: The required level
  --------------------------------------------------------------------*/
void PolyEdge::setBaseLevel(double z) {
	origin.z = end.z = z;
} //PolyEdge::setBaseLevel


/*--------------------------------------------------------------------
	Offset the edge by the specified amount
	
	shift: The amount to offset the edge by
  --------------------------------------------------------------------*/
void PolyEdge::offset(double shift) {
	if (auto arc = asArc(); arc) {
		arc->expand((isGreaterZero(end.sweep)) ? -shift : shift);
		origin = arc->getOrigin();
		end = arc->getEnd();
	} else {
		double angle = azimuthAngle() + (pi / 2.0);
		origin.movePolar(shift, angle);
		end.movePolar(shift, angle);
	}
} //PolyEdge::offset


/*--------------------------------------------------------------------
	Extend the edge length by the specified amount
	
	It is also possible to shorten the edge or to set *len* to negative values.
	If the edge is an arc, it cannot be extended further than to a full circle,
	both to the positive and to the negative direction.

	len: The amount to extend the edge by
	byEnd: True to extend the edge by the endpoint
  --------------------------------------------------------------------*/
void PolyEdge::extend(double len, bool byEnd) {
	if (isZero(len))
		return;
	if (auto arc = asArc(); arc) {
		double delta = sign(arc->sweep) * len / arc->radius;
		arc->sweep = arc->sweep + delta;
		if (isGreaterOrEqual(fabs(arc->sweep), 2 * pi))
			arc->sweep = sign(arc->sweep) * 2 * pi;
		else if (!byEnd)
			arc->startAngle -= delta;
		origin = arc->getOrigin();
		end = arc->getEnd();
	} else {
		Point& targetEnd((byEnd) ? end : origin);
		targetEnd.movePolar(len, azimuthAngle(), altitudeAngle());
	}
} //PolyEdge::extend


/*--------------------------------------------------------------------
	Extend the edge to the point nearest te specified point
	
	ref: The point to extend to
 	byEnd: True to extend the edge by the endpoint
  --------------------------------------------------------------------*/
void PolyEdge::extend(const Point& ref, bool byEnd) {
	auto pt(closestPointTo2D(ref));
	if (auto arc = asArc(); arc) {
		if (byEnd) {
			Arc newEdge(arc->centre, origin, pt, arc->sweep < 0);
			end = newEdge.getEnd();
		} else {
			Arc newEdge(arc->centre, pt, end, arc->sweep < 0);
			origin = newEdge.getOrigin();
			end = newEdge.getEnd();
		}
	} else {
		if (byEnd)
			end = pt;
		else
			origin = pt;
	}
} //PolyEdge::extend


/*--------------------------------------------------------------------
	Split the edge at the specified point
	
	pos: The point to split the edge at
 	offcut: The offcut created by the split
	keepOrig: True to maintain the edge origin
  --------------------------------------------------------------------*/
void PolyEdge::split(const Point& pos, PolyEdge& offcut, bool keepOrig) {
	offcut = *this;
	Point prevAnchor((keepOrig) ? origin : end);
	PolyPoint intersect(closestPointAlong2D(pos));
	switch (positionOf2D(intersect)) {
		case Point::origin:
			end = origin;
			break;
		case Point::end:
			offcut.origin = end;
			break;
		case Point::undefined:
			return;
		default: {
			if (isArc()) {
				Arc arc(origin, end);
				Arc newArc(arc.centre, arc.getOrigin(), pos, arc.sweep < 0);
				intersect.sweep = newArc.sweep;
				offcut.end.sweep = arc.sweep - newArc.sweep;
			}
			end = intersect;
			offcut.origin = intersect;
			break;
		}
	}
	if (!prevAnchor.isEqual2D((keepOrig) ? origin : end))
		std::swap(*this, offcut);
} //PolyEdge::split


/*--------------------------------------------------------------------
	Move the edge by the specified distance and angle

	len: The distance to move the edge
	angle: The angle to move the edge along
  --------------------------------------------------------------------*/
void PolyEdge::movePolar(double len, double angle) {
	origin.movePolar(len, angle);
	end.movePolar(len, angle);
} //PolyEdge::movePolar


/*--------------------------------------------------------------------
	Move the edge by the specified distance and azimuth/altitude angles

	len: The distance to move the edge
	azim: The azimuth angle to move the edge along
	alt: The altitude angle to move the edge along
  --------------------------------------------------------------------*/
void PolyEdge::movePolar(double len, double azim, double alt) {
	origin.movePolar(len, azim, alt);
	end.movePolar(len, azim, alt);
} //PolyEdge::movePolar


/*--------------------------------------------------------------------
	Flip the edge, ie reverse its direction
  --------------------------------------------------------------------*/
PolyEdge& PolyEdge::flip() {
	double sweep = end.sweep;
	Point temp = origin;
	origin = end;
	end = temp;
	end.sweep = -sweep;
	return *this;
} //PolyEdge::flip
