/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Plane.h"

#include <cmath>

#include "Active/Geometry/Box.h"
#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Matrix4x4.h"
#include "Active/Geometry/Vector4.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Factory functions
		
/*--------------------------------------------------------------------
	Create a new plane from a normal and offset from the origin
	
	offset: Distance along normal from the origin to the plane
	norm: Normal to the plane
	
	return: An unique_ptr to a plane, or 0 if invalid
  --------------------------------------------------------------------*/
Plane::Option Plane::create(double offset, const Vector3& norm) {
	return norm.isEmpty() ? std::nullopt : std::make_optional(Plane(offset, norm));
} //Plane::create


/*--------------------------------------------------------------------
	Create a new plane from a point and normal
	
	point: A point on the plane
	normal: Normal to the plane
	
	return: An unique_ptr to a plane, or 0 if invalid
  --------------------------------------------------------------------*/
Plane::Option Plane::create(const Point& point, const Vector3& norm) {
	return norm.isEmpty() ? std::nullopt : std::make_optional(Plane(norm.normalised().dotProduct(Vector3(point)), norm));
} //Plane::create


/*--------------------------------------------------------------------
	Create a plane from three points
	
	p1: First point on the plane
	p2: Second point on the plane
	p3: Third point on the plane
	
	return: An unique_ptr to a plane, or 0 if invalid
  --------------------------------------------------------------------*/
Plane::Option Plane::create(const Point& p1, const Point& p2, const Point& p3) {
	Vector3 pt1(p1 - p2);
	Vector3 pt2(p3 - p2);
	Vector3 norm(pt1.vectorProduct(pt2).normalised());
	return norm.isEmpty() ? std::nullopt : std::make_optional(Plane(norm.dotProduct(p1), norm));
} //Plane::create
		
// MARK: - Constructors
		
/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Plane::Plane() : m_normal{0.0, 0.0, 1.0} {
	m_offset = 0.0;
} //Plane::Plane


/*--------------------------------------------------------------------
	Constructor
	
	norm: The plane normal
  --------------------------------------------------------------------*/
Plane::Plane(const Vector3& norm) : m_normal{norm} {
	if (isZero(norm.magnitude()))
		m_normal = Vector3(0.0, 0.0, 1.0);
	m_offset = 0.0;
} //Plane::Plane


/*--------------------------------------------------------------------
	Constructor
	
	offset: Distance along normal from the origin to the plane
	norm: Normal to the plane
  --------------------------------------------------------------------*/
Plane::Plane(double offset, const Vector3& norm) : m_normal{norm.normalised()} {
	m_offset = offset;
} //Plane::Plane


/*--------------------------------------------------------------------
	Constructor
	
	point: A point on the plane
	normal: Normal to the plane
  --------------------------------------------------------------------*/
Plane::Plane(const Point& point, const Vector3& norm) : m_normal{norm.normalised()} {
	m_offset = m_normal.dotProduct(Vector3(point));
} //Plane::Plane
		
// MARK: - Functions (const)
		
/*--------------------------------------------------------------------
	Get the normal to the plane
	
	return: The normal to the plane
  --------------------------------------------------------------------*/
const Vector3& Plane::getNormal() const {
	return m_normal;
} //Vector3& Plane::getNormal


/*--------------------------------------------------------------------
	Get the offset from the origin to the plane along the normal
	
	return: The offset from the origin to the plane along the normal
  --------------------------------------------------------------------*/
double Plane::getOffset() const {
	return m_offset;
} //Plane::getOffset


/*--------------------------------------------------------------------
	Addition operator
	
	offset: The amount to offset this by
	
	return: The result of the addition
  --------------------------------------------------------------------*/
Plane Plane::operator+ (const Point& offset) {
	return Plane{*this} += offset;
} //Plane::operator+


/*--------------------------------------------------------------------
	Addition and assignment operator
	
	offset: The amount to offset this by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Plane& Plane::operator+= (const Point& offset) {
	Point orig{m_normal * m_offset};
	orig += offset;
	return (*this = Plane{orig, m_normal});
} //Plane::operator+=


/*--------------------------------------------------------------------
	Multiplication operator
	
	mult: The factor to multiply this by
	
	return: The result of the multiplication of this and the specified factor
  --------------------------------------------------------------------*/
Plane Plane::operator* (const double& mult) const {
	return Plane{*this} *= mult;
} //Plane::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply this by
	
	return: The result of the multiplication
  --------------------------------------------------------------------*/
Plane Plane::operator* (const Matrix3x3& matrix) const {
	return Plane{*this} *= matrix;
} //Plane::operator*


/*--------------------------------------------------------------------
	Multiplication operator
	
	matrix: The matrix to multiply this by
	
	return: The result of the multiplication
  --------------------------------------------------------------------*/
Plane Plane::operator* (const Matrix4x4& matrix) const {
	return Plane{*this} *= matrix;
} //Plane::operator*


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	mult: The factor to multiply this by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Plane& Plane::operator*= (const double& mult) {
	m_offset *= mult;
	if (mult < 0.0)
		m_normal *= -1.0;
	return *this;
} //Plane::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply this by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Plane& Plane::operator*= (const Matrix3x3& matrix) {
	Vector3 vect{m_normal * m_offset};
	vect *= matrix;
	m_offset = vect.magnitude();
	m_normal = (m_normal *= matrix).normalised();
	return *this;
} //Plane::operator*=


/*--------------------------------------------------------------------
	Multiplication and assignment operator
	
	matrix: The matrix to multiply this by
	
	return: A reference to this
  --------------------------------------------------------------------*/
Plane& Plane::operator*= (const Matrix4x4& matrix) {
	Vector4 vect{m_normal * m_offset};
	vect *= matrix;
	m_normal *= matrix;
	return (*this = Plane{Point(vect), m_normal});
} //Plane::operator*=


/*--------------------------------------------------------------------
	Get the position of a point with respect to the plane
	
	ref: The reference point
	
	return: The relative point position
  --------------------------------------------------------------------*/
Position Plane::positionOf(const Point& ref, double prec) const {
	double len = m_normal.dotProduct(ref);
	if (isLess(len, m_offset, prec))
		return Point::back;
	else if (isGreater(len, m_offset, prec))
		return Point::front;
	return Point::along;
} //Plane::positionOf


/*--------------------------------------------------------------------
	Get the point on the plane closest to the reference point
	
	ref: The reference point
	
	return: The closest point (perpendicular to the plane)
  --------------------------------------------------------------------*/
Point Plane::closestPointTo(const Point& ref) const {
	Point result{ref};
	double scal = (m_offset - m_normal.dotProduct(ref)) / m_normal.dotProduct(m_normal);
	result.x += scal * m_normal[0];
	result.y += scal * m_normal[1];
	result.z += scal * m_normal[2];
	return result;
} //Plane::closestPointTo


/*--------------------------------------------------------------------
	Get the minimum length from a point to the plane
	
	ref: The reference point
	
	return: The minimum distance between the point and this plane
  --------------------------------------------------------------------*/
double Plane::lengthTo(const Point& ref) const {
	return (m_normal.dotProduct(ref) - m_offset) / m_normal.modulus();
} //Plane::lengthTo


/*--------------------------------------------------------------------
	Get the height of a point projected vertically onto the plane
	
	ref: The reference point
	
	return: The height of the point projected vertically onto the plane
  --------------------------------------------------------------------*/
double Plane::heightAt(const Point& ref, double prec) const {
	if (isZero(m_normal[2], prec))
		return ref.z;
	auto inter = intersectionWith(Line(ref, ref + Point(0, 0, 1)), prec);
	return inter ? ref.z : inter->z;
} //Plane::heightAt


/*--------------------------------------------------------------------
	Get the point of intersection between a line and the plane
	
	ref: The reference line
	
	return: The point of intersection between the line and plane
  --------------------------------------------------------------------*/
XPoint::Option Plane::intersectionWith(const Line& ref, double prec) const {
	Vector3 vect{Vector3(ref).normalised()};
	double dot = m_normal.dotProduct(vect);
	if (isZero(dot, prec))
		return std::nullopt;
	auto result = std::make_optional<XPoint>(ref.origin);
	double scal = (m_offset - m_normal.dotProduct(*result)) / dot;
	result->x += vect[0] * scal;
	result->y += vect[1] * scal;
	result->z += vect[2] * scal;
	return result;
} //Plane::intersectionWith


/*--------------------------------------------------------------------
	Get the line at the intersection of two planes
	
	ref: The reference plane
	
	return: The line at which the planes intersect (or 0 if they are parallel)
  --------------------------------------------------------------------*/
Line::Option Plane::intersectionWith(const Plane& ref, double prec) const {
	Vector3 ortho{m_normal.vectorProduct(ref.m_normal)};
	if (ortho.isEmpty())
		return std::nullopt;
	short maxFact;
	if (fabs(ortho[0]) > fabs(ortho[1]))
		maxFact = (fabs(ortho[0]) > fabs(ortho[2])) ? 0 : 2;
	else
		maxFact = (fabs(ortho[1]) > fabs(ortho[2])) ? 1 : 2;
	Point inter;
	switch (maxFact) {
		case 0:
			inter.x = 0.0;
			inter.y = (m_offset * ref.m_normal[2] - ref.m_offset * m_normal[2]) / ortho[0];
			inter.z = (ref.m_offset * m_normal[1] - m_offset * ref.m_normal[1]) / ortho[0];
			break;
		case 1:
			inter.x = (ref.m_offset * m_normal[2] - m_offset * ref.m_normal[2]) / ortho[1];
			inter.y = 0.0;
			inter.z = (m_offset * ref.m_normal[0] - ref.m_offset * m_normal[0]) / ortho[1];
			break;
		case 2:
			inter.x = (m_offset * ref.m_normal[1] - ref.m_offset * m_normal[1]) / ortho[2];
			inter.y = (ref.m_offset * m_normal[0] - m_offset * ref.m_normal[0]) / ortho[2];
			inter.z = 0.0;
			break;
	}
	return std::make_optional<Line>(inter, inter + ortho);
} //Plane::intersectionWith


/*--------------------------------------------------------------------
	Determine whether this plane is parallel to another

	ref: The plane to compare

	return: True if the planes are parallel
  --------------------------------------------------------------------*/
bool Plane::isParallelTo(const Plane& ref, double prec) const {
	return m_normal.isParallelTo(ref.m_normal, prec);
} //Plane::isParallelTo


/*--------------------------------------------------------------------
	Determine whether this plane cuts through the specified volume
	
	ref: The volume to compare
	prec: The required precision
	
	return: True if the plane cuts through the specified volume
  --------------------------------------------------------------------*/
bool Plane::cutsThrough(const Box& ref, double prec) const {
	Box refSort(ref);
	refSort.sort();
	double width = refSort.getWidth(), depth = refSort.getDepth(), height = refSort.getHeight();
	Point testPt{refSort.origin};
	auto refPos = Point::undefined;
	for (auto z = 2; z--; ) {
		testPt.y = refSort.origin.y;
		for (auto y = 2; y--; ) {
			testPt.x = refSort.origin.x;
			for (auto x = 2; x--; ) {
				auto testPos = positionOf(testPt);
				if (testPos != refPos) {
					if (refPos == Point::undefined)
						refPos = testPos;
					else
						return true;
				}
				testPt.x += width;
			}
			testPt.y += depth;
		}
		testPt.z += height;
	}
	return false;			
} //Plane::cutsThrough
		
// MARK: - Functions (mutating)
		
/*--------------------------------------------------------------------
	Set the normal to the plane
	
	vect: The normal to the plane
 
 	return: True if the normal was applied (false = normal invalid)
  --------------------------------------------------------------------*/
bool Plane::setNormal(const Vector3& vect) {
	if (vect.isEmpty())
		return false;
	m_normal = vect;
	return true;
} //Plane::setNormal
