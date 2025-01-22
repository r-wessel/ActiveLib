/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/2D/Point.h"

using namespace active::primitive;
using namespace active::utility;

/*--------------------------------------------------------------------
	Add point coordinates
 
	toAdd: The point to add
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Point::add(const geometry::Point& toAdd) {
	geometry::Point::operator+=(toAdd);
	return Primitive::add(toAdd);
} //Point::add


/*--------------------------------------------------------------------
	Multiply by a specified factor
 
	toMultiply: The factor to multiply the primitive by
	isRenderOffset: True to also scale the render offset (where relevant)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Point::multiply(double toMultiply, bool isRenderOffset) {
	geometry::Point::operator*=(toMultiply);
	return Primitive::multiply(toMultiply);
} //Point::multiply


/*--------------------------------------------------------------------
	Multiply by the coordinates of a point
 
	toMultiply: The point coords to multiply by
	isRenderOffset: True to also scale the render offset (where relevant)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Point::multiply(const geometry::Point& toMultiply, bool isRenderOffset) {
	geometry::Point::operator*=(toMultiply);
	return Primitive::multiply(toMultiply);
} //Point::multiply


/*--------------------------------------------------------------------
	Multiply by a 3x3 matrix
 
	toMultiply: A 3x3 matrix
	includeRenderSized: Include render-sized geometry in the transformation
	isRenderTranslation: True if the matrix translation applies to render-sized geometry
 
	return: A reference to this
  --------------------------------------------------------------------*/
Primitive& Point::multiply(const geometry::Matrix3x3& toMultiply, bool includeRenderSized, bool isRenderTranslation) {
	geometry::Point::operator*=(toMultiply);
	return Primitive::multiply(toMultiply, includeRenderSized, isRenderTranslation);
} //Point::multiply
