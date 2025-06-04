/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/2D/Primitive.h"

#include "Active/Geometry/Matrix3x3.h"
#include "Active/Geometry/Polygon.h"

using namespace active;
using namespace active::geometry;
using namespace active::primitive;

/*--------------------------------------------------------------------
	Copy constructor
 
	source: The object to copy
  --------------------------------------------------------------------*/
Primitive::Primitive(const Primitive& source) {
	*this = source;
} //Primitive::Primitive


/*--------------------------------------------------------------------
	Assignment operator
 
	source: The object to assign
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Primitive::operator= (const Primitive& source) {
	if (this != &source) {
		m_pen = source.m_pen;
		m_edgeColour = source.m_edgeColour;
		m_fillColour = source.m_fillColour;
		m_renderAnchor = source.m_renderAnchor;
		if (source.m_renderOrigin)
			m_renderOrigin = clone(*source.m_renderOrigin);
		if (source.m_renderOffset)
			m_renderOffset = clone(*source.m_renderOffset);
	}
	return *this;
} //Primitive::operator=


/*--------------------------------------------------------------------
	Get the midpoint of the primitive
 
	return: The primitive midpoint
  --------------------------------------------------------------------*/
Point::Option Primitive::midpoint() const {
	if (auto rectBounds = bounds(); rectBounds)
		return rectBounds->getCentre();
	return std::nullopt;
} //Primitive::midpoint


/*--------------------------------------------------------------------
	Get the primitive rectilinear bounds in world coordinates
 
	pixelSize: The renderer pixel size for the scene
	scale: The primitive rendering scale
 
	return: The primitive bounds in world coordinates
  --------------------------------------------------------------------*/
Box::Option Primitive::worldBounds(double pixelSize, double scale) {
	if (auto path = worldPath(pixelSize, scale); path)
		return path->bounds();
	return std::nullopt;
} //Primitive::worldBounds


/*--------------------------------------------------------------------
	Determine if the primitive world bounds encloses a point
 
	point: The point
	pixelSize: The renderer pixel size
	scale: The primitive rendering scale
	prec: The required precision
 
	return: True if the primitive bounds encloses the point
  --------------------------------------------------------------------*/
bool Primitive::enclosesWorld(const Point& point, double pixelSize, double scale, double prec) {
	if (auto path = worldPath(pixelSize, scale); path)
		path->encloses(point, prec);
	return false;
} //Primitive::enclosesWorld


/*--------------------------------------------------------------------
	Add point coordinates
 
	toAdd: The point to add
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Primitive::add(const Point& toAdd) {
	if (isRenderSized() && m_renderOrigin)
		*m_renderOrigin += toAdd;
	return *this;
} //Primitive::add


/*--------------------------------------------------------------------
	Multiply by a specified factor
 
	toMultiply: The factor to multiply the primitive by
	isRenderOffset: True to also scale the render offset (where relevant)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Primitive::multiply(double toMultiply, bool isRenderOffset) {
	if (isRenderSized()) {
		if (m_renderOrigin)
			*m_renderOrigin *= toMultiply;
		if (isRenderOffset && m_renderOffset)
			*m_renderOffset *= toMultiply;
	}
	return *this;
} //Primitive::multiply


/*--------------------------------------------------------------------
	Multiply by the coordinates of a point
 
	toMultiply: The point coords to multiply by
	isRenderOffset: True to also scale the render offset (where relevant)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Primitive::multiply(const Point& toMultiply, bool isRenderOffset) {
	if (isRenderSized()) {
		if (m_renderOrigin)
			*m_renderOrigin *= toMultiply;
		if (isRenderOffset && m_renderOffset)
			*m_renderOffset *= toMultiply;
	}
	return *this;
} //Primitive::multiply


/*--------------------------------------------------------------------
	Multiply by a 3x3 matrix
 
	toMultiply: A 3x3 matrix
	includeRenderSized: Include render-sized geometry in the transformation
	isRenderTranslation: True if the matrix translation applies to render-sized geometry
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Primitive::multiply(const Matrix3x3& toMultiply, bool includeRenderSized, bool isRenderTranslation) {
	if (isRenderSized()) {
		if (m_renderOrigin)
			*m_renderOrigin *= toMultiply;
		if (includeRenderSized && m_renderOffset) {
			Matrix3x3 offsetTransform{toMultiply};
			offsetTransform.clearTransform();
			*m_renderOffset *= offsetTransform;
		}
	}
	return *this;
} //Primitive::multiply


/*--------------------------------------------------------------------
	Copy the attributes of a another primitive
 
	source: The primitive to copy
	isOriginalFill: True to retain the original fill
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Primitive::copyAttributes(const Primitive& source, bool isOriginalFill) {
	m_pen = source.m_pen;
	m_edgeColour = source.m_edgeColour;
	if (!isOriginalFill)
		m_fillColour = source.m_fillColour;
	return *this;
} //Primitive::copyAttributes
