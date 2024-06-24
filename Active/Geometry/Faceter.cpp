/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Faceter.h"

#include "Active/Geometry/Arc.h"
#include "Active/Geometry/PolyPoint.h"

using namespace active::geometry;
using namespace active::math;

// MARK: - Constructors
		
/*--------------------------------------------------------------------
	Constructor
	
	orig: The arc origin
	prec: The arc end
	isStart: Include a vertex at the start angle
	isEnd: Include a vertex at the end angle
 	toler: The arc tolerance
  --------------------------------------------------------------------*/
Faceter::Faceter(const Point& orig, const PolyPoint& end, bool isStart, bool isEnd, double toler) :
		m_arc(orig, end) {
	m_isAlong = false;
	m_step = m_remainder = 0;
	m_incAngle = m_incHeight = 0;
	m_currentStep = m_startStep = m_endStep = 0;
	if (!m_arc.isValid() || isLessOrEqual(m_arc.radius, 2 * toler))
		return;
	m_incAngle = 2 * acos(m_arc.radius / (m_arc.radius + toler));
	double startAngle = m_arc.startAngle, endAngle = m_arc.getEndAngle();
	bool isReverse = (m_arc.sweep < 0);
	if (isReverse) {
		std::swap(isStart, isEnd);
		std::swap(startAngle, endAngle);
	}
	vertex_index arcSteps = static_cast<vertex_index>((2.0 * pi) / m_incAngle);
	if (!isZero(fmod(2.0 * pi, m_incAngle))) {
		++arcSteps;
		m_incAngle = (2.0 * pi) / arcSteps;
	}
	if (endAngle < startAngle)
		endAngle += (2.0 * pi);
	m_startStep = static_cast<vertex_index>(math::round(startAngle / m_incAngle));
	if (!isStart) {
		++m_startStep;
		startAngle = m_incAngle * m_startStep;
	}
	m_endStep = static_cast<vertex_index>(math::round(endAngle / m_incAngle));
	bool atAngle = isZero(fmod(endAngle, m_incAngle));
	if (!isEnd) {
		if (atAngle)
			--m_endStep;
		endAngle = m_incAngle * m_endStep;
	} else if (!atAngle)
		++m_endStep;
	if (startAngle > endAngle)
		startAngle = endAngle;
	if (m_endStep < m_startStep)
		m_endStep = m_startStep;
	if (isReverse) {
		std::swap(startAngle, endAngle);
		std::swap(m_startStep, m_endStep);
	}
	m_arc.startAngle = startAngle;
	m_arc.sweep = endAngle - startAngle;
} //Faceter::Faceter


/*--------------------------------------------------------------------
	Constructor
 
	orig: The arc origin
	prec: The arc end
	step: The step value along the path
  --------------------------------------------------------------------*/
Faceter::Faceter(const Point& orig, const PolyPoint& end, double step) : m_edge(orig, end) {
	m_isAlong = true;
	m_incAngle = m_incHeight = 0;
	m_currentStep = m_startStep = 0;
	m_step = fabs(step);
	m_endStep = 0;
	m_remainder = 0;
	if (!isZero(m_step)) {
		m_endStep = static_cast<vertex_index>(m_edge.length3D() / m_step) + 1;
		m_remainder = fMod(m_edge.length3D(), m_step);
	}
} //Faceter::Faceter


/*--------------------------------------------------------------------
	Copy constructor

	source: The object to copy
  --------------------------------------------------------------------*/
Faceter::Faceter(const Faceter& source) : m_arc(source.m_arc) {
	m_incAngle = source.m_incAngle;
	m_incHeight = source.m_incHeight;
	m_currentStep = source.m_currentStep;
	m_startStep = source.m_startStep;
	m_endStep = source.m_endStep;
	m_isAlong = source.m_isAlong;
	m_edge = source.m_edge;
	m_step = source.m_step;
	m_remainder = source.m_remainder;
} //Faceter::Faceter
		
// MARK: - Operators
		
/*--------------------------------------------------------------------
	Increment operator
  --------------------------------------------------------------------*/
void Faceter::operator++ () {
	if (!isAtEnd())
		m_currentStep += (m_endStep < m_startStep) ? -1 : 1;
} //Faceter::operator++


/*--------------------------------------------------------------------
	Postfix increment operator
	
	return: A copy of this
  --------------------------------------------------------------------*/
Faceter Faceter::operator++ (int) {
	Faceter temp(*this);
	operator++();
	return temp;
} //Faceter::operator++
		
// MARK: - Functions (const)
		
/*--------------------------------------------------------------------
	Get the current vertex
	
	return: The current vertex
  --------------------------------------------------------------------*/
Point Faceter::getVertex() const {
	Point result;
	if (m_isAlong) {
		PolyEdge temp(m_edge);
		temp.flip();
		temp.extend(-m_currentStep * m_step);
		temp.flip();
		result = temp.origin;
	} else {
		result = m_arc.centre;
		double angle;
		if (isAtEnd())
			angle = m_arc.getEndAngle();
		else if (isAtStart())
			angle = m_arc.startAngle;
		else
			angle = (m_startStep + m_currentStep) * m_incAngle;
		result.movePolar(m_arc.radius, angle);
	}
	return result;
} //Faceter::getVertex
