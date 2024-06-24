/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/XPoint.h"

using namespace active::geometry;
	
/*--------------------------------------------------------------------
	Constructor
	
	source: The intersection point
	obj: The relationship of the point to the target
	blade: The relationship of the point to the target
  --------------------------------------------------------------------*/
XPoint::XPoint(const Point& source, Position oPos, Position bPos) :
		Point(source) {
	m_info[target] = oPos;
	m_info[blade] = bPos;
} //XPoint::XPoint


/*--------------------------------------------------------------------
	Assignment operator
 
	source: The object to copy
 
	return: A reference to this
  --------------------------------------------------------------------*/
XPoint& XPoint::operator= (const Point& source) {
	Point::operator= (source);
	return *this;
} //XPoint::operator=


/*--------------------------------------------------------------------
	Swap the blade and target intercepts
  --------------------------------------------------------------------*/
void XPoint::swapIntercept() {
	XInfo temp = m_info[target];
	m_info[target] = m_info[blade];
	m_info[blade] = temp;
} //XPoint::swapIntercept
