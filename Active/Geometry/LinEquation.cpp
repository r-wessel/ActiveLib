/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/LinEquation.h"

#include "Active/Geometry/Line.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Geometry/XPoint.h"

#include <cmath>

using namespace active::geometry;
using namespace active::math;

/*--------------------------------------------------------------------
	Create a new linear equation from a point (origin assumed to be 0,0,0)
	
	source: The end of the line
	
	return: A linear equation (nullopt if input is invalid)
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::create(const Point& source) {
	return getEquation(Point(), source);
} //LinEquation::create


/*--------------------------------------------------------------------
	Create a new linear equation from two points in a line

	start: The start of the line
	end: The end of the line
	
	return: A linear equation (nullopt if input is invalid)
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::create(const Point& start, const Point& end) {
	return getEquation(start, end);
} //LinEquation::create


/*--------------------------------------------------------------------
	Create a new linear equation from a line

	source: A line
	
	return: A linear equation (nullopt if input is invalid)
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::create(const Line& source) {
	return getEquation(source.origin, source.end);
} //LinEquation::create


/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
LinEquation::LinEquation() {
	calculate();
} //LinEquation::LinEquation


/*--------------------------------------------------------------------
	Constructor
	
	angle: Angle of the line
	source: A point along the line
  --------------------------------------------------------------------*/
LinEquation::LinEquation(double angle, const Point& source) {
	calculate(source, cos(angle), sin(angle));
} //LinEquation::LinEquation


/*--------------------------------------------------------------------
	Copy constructor

	source: The object to be copied
  --------------------------------------------------------------------*/
LinEquation::LinEquation(const LinEquation& source) {
	*this = source;
} //LinEquation::LinEquation


/*--------------------------------------------------------------------
	Calculate the equation based on a slope and origin
	
	start: The origin for the equation
	dx: Slope delta x
	dy: Slope delta
  --------------------------------------------------------------------*/
void LinEquation::calculate(const Point& start, double dx, double dy) {
	if (isZero(dx)) {
		double sign = flow_sgn(dy);
		m_a = sign;
		m_b = 0.0;
		m_c = sign * -start.x;
	} else if (isZero(dy)) {
		double sign = flow_sgn(dx);
		m_a = 0.0;
		m_b = -sign;
		m_c = sign * start.y;
	} else {
		double ratio = maxVal(fabs(1 / dx), fabs(1 / dy));
		m_a = dy * ratio;
		m_b = -dx * ratio;
		m_c = -start.x * m_a - start.y * m_b;
	}
} //LinEquation::calculate


/*--------------------------------------------------------------------
	Get an equation from the specified points

	start: The start of the line
	end: The end of the line
	
	return: A linear equation (nullopt if input is invalid)
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::getEquation(const Point& start, const Point& end) {
	double dx = end.x - start.x;
	double dy = end.y - start.y;
	return (isZero(dx) && isZero(dy)) ? std::nullopt : std::make_optional<LinEquation>(Vector3{dx, dy}.azimuthAngle(), start);
} //LinEquation::getEquation


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
LinEquation& LinEquation::operator= (const LinEquation& source) {
	if (&source != this) {
		m_a = source.m_a;
		m_b = source.m_b;
		m_c = source.m_c;
	}
	return *this;
} //LinEquation::operator=


/*--------------------------------------------------------------------
	Equality operator
	
	ref: The object to compare
	
	return: True if ref is identical to this
  --------------------------------------------------------------------*/
bool LinEquation::operator== (const LinEquation& ref) const {
	return (isEqual(m_a, ref.m_a) && isEqual(m_b, ref.m_b) && isEqual(m_c, ref.m_c));
} //LinEquation::operator==


/*--------------------------------------------------------------------
	Test to determine if a point lies within this equation
	
	ref: A point to test
	
	return: True if the point lies along the line
  --------------------------------------------------------------------*/
bool LinEquation::operator== (const Point& ref) const {
	if (isXAxis())
		return (isYAxis()) ? false : isEqual(ref.y, (-m_a * ref.x - m_c) / m_b);
	return isEqual(ref.x, (-m_b * ref.y - m_c) / m_a);
} //LinEquation::operator==


/*--------------------------------------------------------------------
	Less-than operator
	
	ref: The object to compare
	
	return: True if the angle of this equation is less than ref
  --------------------------------------------------------------------*/
bool LinEquation::operator< (const LinEquation& ref) const {
	return (azimuthAngle() < ref.azimuthAngle());
} //LinEquation::operator<


/*--------------------------------------------------------------------
	Test two equations to determine if they are perpendicular
	
	ref: The reference line
	prec: The required precision
	
	return: True if the equations are perpendicular
  --------------------------------------------------------------------*/
bool LinEquation::isPerpendicularTo(const LinEquation& ref, double prec) const {
	return ((isEqual(m_a, ref.m_b, prec) && isEqual(m_b, -ref.m_a, prec)) ||
			(isEqual(m_a, -ref.m_b, prec) && isEqual(m_b, ref.m_a, prec)));
} //LinEquation::isPerpendicularTo


/*--------------------------------------------------------------------
	Test two equations to determine if they are parallel
	
	ref: The reference line
	prec: The required precision
	
	return: True if the equations are parallel
  --------------------------------------------------------------------*/
bool LinEquation::isParallelTo(const LinEquation& ref, double prec) const {
		//This is done so that the prec would represent angle tolerance (in radians)
	double inclination = isYAxis(prec) ? pi / 2.0 : atan(-m_a / m_b);
	double refInclination = ref.isYAxis() ? pi / 2.0 : atan(-ref.m_a / ref.m_b);
	return isEqualAngle(inclination, refInclination, prec) || isEqualAngle(inclination + pi, refInclination, prec);
} //LinEquation::isParallelTo


/*--------------------------------------------------------------------
	Calculate the angle of this equation

	return: The equation's angle (0 to 2*pi)
  --------------------------------------------------------------------*/
double LinEquation::azimuthAngle() const {
	if (isYAxis())
		return (m_a > 0.0) ? pi / 2.0 : 1.5 * pi;
	if (isXAxis())
		return (m_b < 0.0) ? 0.0 : pi;
	double angle = atan(m_a / -m_b);
	if (m_b > 0.0)
		angle += pi;
	else if (isLessZero(angle))
		angle += (2.0 * pi);
	return angle;
} //LinEquation::azimuthAngle


/*--------------------------------------------------------------------
	Create a linear equation opposite to this

	return: An unique_ptr to the opposite equation
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::getFlipped() const {
	auto opposite = std::make_optional<LinEquation>(*this);
	opposite->m_a = -m_a;
	opposite->m_b = -m_b;
	opposite->m_c = -m_c;
	return opposite;
} //LinEquation::getFlipped


/*--------------------------------------------------------------------
	Create a linear equation perpendicular to this which passes through the specified point

	ref: The point through which the new equation must pass

	return: An unique_ptr to the perpendicular equation
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::getPerpendicular(const Point& ref) const {
	auto perpendicular = std::make_optional<LinEquation>(*this);
	perpendicular->m_a = m_b;
	perpendicular->m_b = -m_a;
	perpendicular->m_c = -perpendicular->m_a * ref.x - perpendicular->m_b * ref.y;
	return perpendicular;
} //LinEquation::getPerpendicular


/*--------------------------------------------------------------------
	Create a linear equation parallel to this which passes through the specified point

	ref: The point through which the new equation must pass

	return: An unique_ptr to the parallel equation
  --------------------------------------------------------------------*/
LinEquation::Option LinEquation::getParallel(const Point& ref) const {
	double angle = azimuthAngle();
	double x = cos(angle), y = sin(angle);
	Point dest(x, y);
	dest += ref;
	return create(ref, dest);
} //LinEquation::getParallel


/*--------------------------------------------------------------------
	Calculate the angle between two linear equations
	
	ref: The reference line
	
	return: The angle between the two lines
  --------------------------------------------------------------------*/
double LinEquation::angleTo(const LinEquation& ref) const {
	double angle = ref.azimuthAngle() - azimuthAngle();
	if (angle < 0.0)
		angle += 2.0 * pi;
	return angle;
} //LinEquation::angleTo


/*--------------------------------------------------------------------
	Calculate the intersection point of two equations
	
	ref: The intersecting line
	
	return: An unique_ptr for the intersection point (or 0 if parallel)
  --------------------------------------------------------------------*/
XPoint::Option LinEquation::intersectionWith(const LinEquation& ref) const {
	if (isParallelTo(ref))
		return std::nullopt;
	double y = ((m_a * ref.m_c - m_c * ref.m_a) / (m_b * ref.m_a - m_a * ref.m_b));
	double x = (isXAxis()) ? ((-ref.m_b * y - ref.m_c) / ref.m_a) : ((-m_b * y - m_c) / m_a);
	return std::make_optional<XPoint>(Point{x, y});
} //LinEquation::intersectionWith


/*--------------------------------------------------------------------
	Determine the relationship of a point to the equation
	
	ref: The point to test
	prec: The required precision
	
	return: The relative position of the point (along, left, right)
  --------------------------------------------------------------------*/
Position LinEquation::positionOf(const Point& ref, double prec) const {
	if (isXAxis() && isYAxis())
		return Point::undefined;
	double len = lengthTo(ref);
	if (isZero(len, prec))
		return Point::along;
	double res = m_a * ref.x + m_b * ref.y + m_c;
	return (res < 0.0) ? Point::left : Point::right;
} //LinEquation::positionOf


/*--------------------------------------------------------------------
	Calculate the x coordinate at the specifed y coordinate
	
	y: The y coordinate
	x: The x coordinate (n/a if the equation is parallel to the x axis)
	
	return: False if the equation is parallel to the x axis
  --------------------------------------------------------------------*/
bool LinEquation::xAtY(double y, double& x) const {
	if (isXAxis())
		return false;
	x = ((-y * m_b) - m_c) / m_a;
	return true;
} //LinEquation::xAtY


/*--------------------------------------------------------------------
	Calculate the y coordinate at the specifed x coordinate
	
	x: The x coordinate
	y: The y coordinate (n/a if the equation is parallel to the y axis)
	
	return: False if the equation is parallel to the y axis
  --------------------------------------------------------------------*/
bool LinEquation::yAtX(double x, double& y) const {
	if (isYAxis())
		return false;
	y = ((-x * m_a) - m_c) / m_b;
	return true;
} //LinEquation::yAtX


/*--------------------------------------------------------------------
	Calculate the closest point along this to a another specified point
	
	ref: The reference point
	
	return: The closest point
--------------------------------------------------------------------*/
Point LinEquation::closestPointTo(const Point& ref) const {
	if (auto lin = getPerpendicular(ref); lin) {
		if (auto inter = intersectionWith(*lin); inter)
			return Point(*inter);
	}
	return ref;
} // LinEquation:closestPointTo


/*--------------------------------------------------------------------
	Calculate the minimum distance from a point to this
	
	ref: The reference point
	
	return: The minimum distance
  --------------------------------------------------------------------*/
double LinEquation::lengthTo(const Point& ref) const {
	if (auto lin = getPerpendicular(ref); lin) {
		if (auto inter = intersectionWith(*lin); inter)
			return inter->lengthFrom2D(ref);
	}
	return 0.0;
} //LinEquation:lengthTo
