/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/XList.h"

#include "Active/Geometry/PolyEdge.h"
#include "Active/Geometry/Rotation.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors
		
/*--------------------------------------------------------------------
	Constructor
 
	targetFilter: A filter for intersections with the target
	bladeFilter: A filter for intersections with the blade
  --------------------------------------------------------------------*/
XList::XList(const XInfo& objInfo, const XInfo& bladeInfo) : m_intersect() {
	m_filter[XPoint::target] = objInfo;
	m_filter[XPoint::blade] = bladeInfo;
	m_isSwapped = false;
} //XList::XList

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Determine if the list accepts intersections having a relative position to a component with a specified role
 
	role: The component role
	pos: The relative position of the point with respect to the component
 
	return: True if the list accepts this relative position for the role
  --------------------------------------------------------------------*/
bool XList::withPos(XPoint::Role role, Position pos) const {
	if (m_filter[role].pos == Point::undefined)
		return true;
	return ((m_filter[role].pos & pos) != Point::undefined);
} //XList::withPos


/*--------------------------------------------------------------------
	Determine if the list filters intersections for a specified role based on its relative position
 
	role: The component role
 
	return: True if the list has a defined filter for this role
  --------------------------------------------------------------------*/
bool XList::isPos(XPoint::Role role) const {
	return (m_filter[role].pos != Point::undefined);
} //XList::isPos

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Add an intersection type to the target info
	
	pos: The specified type
  --------------------------------------------------------------------*/
void XList::addPos(XPoint::Role role, Position pos) {
	m_filter[role].pos = Position(m_filter[role].pos | pos);
} //XList::addPos


/*--------------------------------------------------------------------
	Swap the target and blade filters
  --------------------------------------------------------------------*/
void XList::swapFilters() {
	m_isSwapped = !m_isSwapped;
} //XList::swapFilters


/*--------------------------------------------------------------------
	Insert an intersection point
	
	pt: A new intersection point
	
	return: True if the point was inserted
  --------------------------------------------------------------------*/
bool XList::insert(XPoint&& pt) {
	return insert(std::make_unique<XPoint>(std::move(pt)));
} //XList::insert


/*--------------------------------------------------------------------
	Insert an intersection point
	
	pt: A new intersection point
	
	return: True if the point was inserted
  --------------------------------------------------------------------*/
bool XList::insert(XPoint::Unique&& pt) {
	pt->setVertex(XPoint::target, m_filter[XPoint::target].vertexIndex);
	pt->setPart(XPoint::target, m_filter[XPoint::target].partIndex);
	pt->setVertex(XPoint::blade, m_filter[XPoint::blade].vertexIndex);
	pt->setPart(XPoint::blade, m_filter[XPoint::blade].partIndex);
	if (m_isSwapped) {
		Position temp = pt->getPos(XPoint::target);
		pt->setPos(XPoint::target, pt->getPos(XPoint::blade));
		pt->setPos(XPoint::blade, temp);
	}
	if (withPos(XPoint::target, pt->getPos(XPoint::target)) && withPos(XPoint::blade, pt->getPos(XPoint::blade))) {
		m_intersect.emplace_front(std::move(pt));
		return true;
	}
	return false;
} //XList::insert


/*--------------------------------------------------------------------
	Remove duplicate intersections
 
	prec: The comparison precision
  --------------------------------------------------------------------*/
void XList::removeDuplicates(double prec) {
	for (auto i = begin(); i != end(); ) {
		auto n = i;
		for (++n; (n != end()) && (((*i)->getPart(XPoint::blade) != (*n)->getPart(XPoint::blade)) ||
				((*i)->getPart(XPoint::target) != (*n)->getPart(XPoint::target)) || !(*i)->isEqual2D(**n, prec)); ++n) {}
		if (n == end())
			++i;
		else
			i = m_intersect.erase(i);
	}
} //XList::removeDuplicates


/*--------------------------------------------------------------------
	Constructor
 
	edge: The poly-edge to sort the point along
	prec: The comparison precision
  --------------------------------------------------------------------*/
AlongLengthOf::AlongLengthOf(const PolyEdge& edge) : m_origin(edge.origin) {
	m_radius = edge.getRadius(true);
	m_isClockwise = (edge.end.sweep < 0.0);
} //AlongLengthOf::AlongLengthOf


/*--------------------------------------------------------------------
	Compare the offset of 2 points along the length of a poly-edge
 
	return: True if the transaction is viable (will not run otherwise)
  --------------------------------------------------------------------*/
bool AlongLengthOf::operator() (XPoint::Unique& pos1, XPoint::Unique& pos2) const {
	using enum Rotation;
	PolyEdge edge1(m_origin, *pos1, m_radius, (m_isClockwise) ? clockwise : anticlockwise);
	PolyEdge edge2(m_origin, *pos2, m_radius, (m_isClockwise) ? clockwise : anticlockwise);
	return edge1.length2D() < edge2.length2D();
} //AlongLengthOf::operator()
