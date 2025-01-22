/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/2D/Path.h"

using namespace active::geometry;
using namespace active::primitive;
using namespace active::utility;

/*--------------------------------------------------------------------
	Constructor
 
	polygon: The primitive polygon
	pn: The primitive linework pen
	edgeCol: The edge colour (nullopt = no edge)
	fillCol: The fill colour (nullopt = no fill)
  --------------------------------------------------------------------*/
Path::Path(const geometry::Polygon& polygon, attribute::Pen pn,
							 attribute::Colour::Option edgeCol, attribute::Colour::Option fillCol) :
		Primitive{pn, edgeCol, fillCol},
		Polygon{static_cast<vertex_index>(polygon.size()), 0, polygon.isClosed} {
	for (auto& vertex : polygon)
		emplace_back(PolyPoint{*vertex});
} //Path::Path


/*--------------------------------------------------------------------
	Add point coordinates
 
	toAdd: The point to add
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Path::add(const Point& toAdd) {
	Polygon::operator+=(toAdd);
	return Primitive::add(toAdd);
} //Path::add


/*--------------------------------------------------------------------
	Multiply by a specified factor
 
	toMultiply: The factor to multiply the primitive by
	isRenderOffset: True to also scale the render offset (where relevant)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Path::multiply(double toMultiply, bool isRenderOffset) {
	Polygon::operator*=(toMultiply);
	return Primitive::multiply(toMultiply);
} //Path::multiply


/*--------------------------------------------------------------------
	Multiply by the coordinates of a point
 
	toMultiply: The point coords to multiply by
	isRenderOffset: True to also scale the render offset (where relevant)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Path::multiply(const Point& toMultiply, bool isRenderOffset) {
	Polygon::operator*=(toMultiply);
	return Primitive::multiply(toMultiply);
} //Path::multiply


/*--------------------------------------------------------------------
	Multiply by a 3x3 matrix
 
	toMultiply: A 3x3 matrix
	includeRenderSized: Include render-sized geometry in the transformation
	isRenderTranslation: True if the matrix translation applies to render-sized geometry
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Path::multiply(const Matrix3x3& toMultiply, bool includeRenderSized, bool isRenderTranslation) {
	Polygon::operator*=(toMultiply);
	return Primitive::multiply(toMultiply, includeRenderSized, isRenderTranslation);
} //Path::multiply
