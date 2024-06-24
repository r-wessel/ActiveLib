/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/PolyPoint.h"

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/Line.h"
#include "Active/Geometry/LinEquation.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor
	
	source: The point position
	angle: The point position
	id: The point ID
  --------------------------------------------------------------------*/
PolyPoint::PolyPoint(const Point& source, double angle, vertex_id id) : Point(source) {
	sweep = angle;
	this->id = id;
} //PolyPoint::PolyPoint


/*--------------------------------------------------------------------
	Copy constructor

	source: The object to be copied
  --------------------------------------------------------------------*/
PolyPoint::PolyPoint(const PolyPoint& source) : Point(source) {
	sweep = source.sweep;
	id = source.id;
} //PolyPoint::PolyPoint

// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment operator
	
	source: The point to assign
	
	return: A reference to this
  --------------------------------------------------------------------*/
PolyPoint& PolyPoint::operator= (const PolyPoint& source) {
	if (this != &source) {
		Point::operator= (source);
		sweep = source.sweep;
		id = source.id;
	}
	return *this;
} //PolyPoint::operator=

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Calculates the distant from a specified point to this in 2D

	ref: A point to calculate the distance from

	return: The distance from the point to this
  --------------------------------------------------------------------*/
double PolyPoint::lengthFrom2D(const Point& ref) const {
		//Test for arcs
	return (isArc()) ? Arc(ref, *this).length2D() : Point::lengthFrom2D(ref);
} //PolyPoint::lengthFrom2D

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Set the sweep angle to this point from the specified line
	
	ref: The reference line (centre to sweep start)
  --------------------------------------------------------------------*/
void PolyPoint::setSweep(const Line& ref) {
	double sweep = (*this == ref.end) ? 2.0 * pi : 2.0 * asin(ref.end.lengthFrom2D(*this) / (2.0 * ref.length2D()));
	auto lin = LinEquation::create(ref);
	auto pos = lin ? Point::along : lin->positionOf(*this);
	this->sweep = (pos == Point::right) ? -sweep : sweep;
} //PolyPoint::setSweep


/*--------------------------------------------------------------------
	Set the sweep angle for an arc starting parallel to a line
	
	ref: The reference line
  --------------------------------------------------------------------*/
void PolyPoint::setSweepParallel(const Line& ref) {
	bool isClock = false;
	auto lin = LinEquation::create(ref);
	if (!lin) {
		sweep = 0.0;
		return;
	}
	switch (lin->positionOf(*this)) {
		case left:
			break;
		case right:
			isClock = true;
			break;
		default:
			sweep = 0.0;
			return;
	}
	double sweep = 2.0 * angleMod(ref.end.azimuthAngleTo(*this) - ref.azimuthAngle());
	this->sweep = (isClock) ? -sweep : sweep;
} //PolyPoint::setSweepParallel
