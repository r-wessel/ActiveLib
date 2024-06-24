/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Leveller.h"

#include <algorithm>

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/Line.h"
#include "Active/Geometry/Point.h"
#include "Active/Geometry/Polygon.h"
#include "Active/Geometry/Rotater.h"

using namespace active::geometry;
using namespace active::math;

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Leveller::Leveller() {
	setRotation(Vector3(0.0, 0.0, 1.0));
} //Leveller::Leveller


/*--------------------------------------------------------------------
	Constructor

	norm Normal to the plane to be levelled
	zAngle: The required rotation about the z axis
  --------------------------------------------------------------------*/
Leveller::Leveller(const Vector3& norm, double zAngle, double prec) {
	setRotation(norm, zAngle, prec);
} //Leveller::Leveller


/*--------------------------------------------------------------------
	Copy constructor

	source: The object to copy
  --------------------------------------------------------------------*/
Leveller::Leveller(const Leveller& source) {
	m_normal = source.m_normal;
	m_rz = clone(*source.m_rz);
	m_ry = clone(*source.m_ry);
	m_rx = clone(*source.m_rx);
} //Leveller::Leveller


/*--------------------------------------------------------------------
	Destructor
  --------------------------------------------------------------------*/
Leveller::~Leveller() {}

		
/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
	
	return: A reference to this
  --------------------------------------------------------------------*/
Leveller& Leveller::operator= (const Leveller& source) {
	if (this != &source) {
		m_normal = source.m_normal;
		m_rz = clone(*source.m_rz);
		m_ry = clone(*source.m_ry);
		m_rx = clone(*source.m_rx);
	}
	return *this;
} //Leveller::operator=


/*--------------------------------------------------------------------
	Set the normal to the plane to be levelled
	
	norm: The normal to the plane to be levelled
	zAngle: The required rotation about the z axis
  --------------------------------------------------------------------*/
void Leveller::setRotation(const Vector3& norm, double zAngle, double prec) {
	m_normal = norm;
	m_rz = std::make_unique<ZRotater>(-zAngle, prec);
	m_ry = std::make_unique<YRotater>(Point().azimuthAngleTo(Point(m_normal[2], -m_normal[0], 0)), prec);
	Vector3 temp(norm);
	m_ry->transform(temp);
	m_rx = std::make_unique<XRotater>(Point().azimuthAngleTo(Point(temp[2], temp[1], 0)), prec);
} //Leveller::setRotation


/*--------------------------------------------------------------------
	Set the rotation about the z axis
	
	angle: The rotation about the z axis
  --------------------------------------------------------------------*/
void Leveller::setZRotation(double angle) {
	m_rz = std::make_unique<ZRotater>(-angle);
} //Leveller::setZRotation


/*--------------------------------------------------------------------
	Reverse the order of the transformation
	
	return: A reference to this
  --------------------------------------------------------------------*/
Leveller& Leveller::reverse() {
	std::swap(m_rz, m_ry);
	m_rz->reverse();
	m_ry->reverse();
	m_rx->reverse();
	return *this;
} //Leveller::reverse


/*--------------------------------------------------------------------
	Transform a point

	target: The point to be transformed
  --------------------------------------------------------------------*/
void Leveller::transform(Point& target) const {
	m_ry->transformPt(target);
	m_rx->transformPt(target);
	m_rz->transformPt(target);
} //Leveller::transform


/*--------------------------------------------------------------------
	Transform a line

	target: The point to be transformed
  --------------------------------------------------------------------*/
void Leveller::transform(Line& target) const {
	transform(target.origin);
	transform(target.end);
} //Leveller::transform


/*--------------------------------------------------------------------
	Transform an arc
	
	target: The arc to be transformed
  --------------------------------------------------------------------*/
void Leveller::transform(Arc& target) const {
	transform(target.centre);
	Point ref{target.normal};
	transform(ref);
	target.normal = Vector3{ref};
} //Leveller::transform


/*--------------------------------------------------------------------
	Transform a polygon
	
	target: The polygon to be transformed
  --------------------------------------------------------------------*/
void Leveller::transform(Polygon& target) const {
	auto* poly = &target;
	auto maxPoly = target.getHoleSize();
	for (auto n = 0; n <= maxPoly; ++n) {
		if (poly != nullptr)
			for (auto& i : *poly)
				transform(*i);
		if (n < maxPoly)
			poly = target.getHole(n);
	}
} //Leveller::transform
