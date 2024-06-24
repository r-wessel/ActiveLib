/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Rotater.h"

#include <cmath>

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/Line.h"
#include "Active/Geometry/Polygon.h"
#include "Active/Geometry/PolyEdge.h"
#include "Active/Geometry/Plane.h"
#include "Active/Geometry/Vector3.h"
#include "Active/Geometry/Vector4.h"

using namespace active::geometry;
using namespace active::math;

/*--------------------------------------------------------------------
	Copy constructor

	source: The object to copy
  --------------------------------------------------------------------*/
Rotater::Rotater(const Rotater& source) {
	m_angle = source.m_angle;
	m_k1 = source.m_k1;
	m_k2 = source.m_k2;
	m_isActive = source.m_isActive;
} //Rotater::Rotater

		
/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Rotater& Rotater::operator= (const Rotater& source) {
	if (this != &source) {
		m_angle = source.m_angle;
		m_k1 = source.m_k1;
		m_k2 = source.m_k2;
		m_isActive = source.m_isActive;
	}
	return *this;
} //Rotater::operator=


/*--------------------------------------------------------------------
	Set the rotation angle
	
	angle: The rotation angle
  --------------------------------------------------------------------*/
void Rotater::setAngle(double angle, double prec) {
	m_angle = angle;
	m_isActive = (!isZero(m_angle, prec));
	if (m_isActive) {
		m_k1 = sin(-angle);
		m_k2 = cos(-angle);
	}
} //Rotater::setAngle


/*--------------------------------------------------------------------
	Rotate an arc

	target: The arc to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(Arc& target) const {
	if (m_isActive) {
		Point orig = target.getOrigin(), mid = target.midpoint(), end = target.getEnd();
		transformPt(orig);
		transformPt(mid);
		transformPt(end);
		target = Arc(orig, mid, end);
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate a line

	target: The line to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(Line& target) const {
	if (m_isActive) {
		transformPt(target.origin);
		transformPt(target.end);
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate an edge

	target: The edge to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(PolyEdge& target) const {
	if (m_isActive) {
		transformPt(target.origin);
		transformPt(target.end);
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate a polygon

	target: The polygon to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(Polygon& target) const {
	if (m_isActive) {
		for (auto i = 0; i <= target.getHoleSize(); ++i) {
			Polygon* poly = (i == 0) ? &target : target.getHole(i - 1);
			for (auto n = poly->vertSize(); n--;)
				transformPt(*((*poly)[n]));
		}
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate a vector

	target: The vector to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(Vector3& target) const {
	if (m_isActive) {
		Point temp(target[0], target[1], target[2]);
		transformPt(temp);
		target = temp;
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate a vector

	target: The vector to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(Vector4& target) const {
	if (m_isActive) {
		Point temp(target[0], target[1], target[2]);
		transformPt(temp);
		double val = target[3];
		target = temp;
		target[3] = val;
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate a plane

	target: The plane to be rotated
  --------------------------------------------------------------------*/
void Rotater::transform(Plane& target) const {
	if (m_isActive) {
		Vector3 temp = target.getNormal();
		transform(temp);
		target.setNormal(temp);
	}
} //Rotater::transform


/*--------------------------------------------------------------------
	Rotate a point about the x axis

	target: The point to be rotated
  --------------------------------------------------------------------*/
Point& XRotater::transformPt(Point& target) const {
	if (isActive()) {
		double temp = target.z * k1() + target.y * k2();
		target.z = target.z * k2() - target.y * k1();
		target.y = temp;
	}
	return target;
} //XRotater::transform


/*--------------------------------------------------------------------
	Rotate a point about the y axis

	target: The point to be rotated
  --------------------------------------------------------------------*/
Point& YRotater::transformPt(Point& target) const {
	if (isActive()) {
		double temp = target.x * k2() + target.z * k1();
		target.z = -target.x * k1() + target.z * k2();
		target.x = temp;
	}
	return target;
} //YRotater::transform


/*--------------------------------------------------------------------
	Rotate a point about the z axis

	target: The point to be rotated
  --------------------------------------------------------------------*/
Point& ZRotater::transformPt(Point& target) const {
	if (isActive()) {
		double temp = target.x * k2() + target.y * k1();
		target.y = -target.x * k1() + target.y * k2();
		target.x = temp;
	}
	return target;
} //ZRotater::transform
