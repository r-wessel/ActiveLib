/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Geometry/Anchor2D.h"

#include <array>

using namespace active::geometry;
using namespace active::utility;

namespace {
	
	std::array anchorName{
		"leftFront",
		"centreFront",
		"rightFront",
		"leftHalf",
		"centreHalf",
		"rightHalf",
		"leftBack",
		"centreBack",
		"rightBack",
	};
	
}  // namespace

/*--------------------------------------------------------------------
	Get an Anchor2D enumerator from text
 
	text: The incoming text
 
	return: The equivalent anchor
  --------------------------------------------------------------------*/
std::optional<Anchor2D> active::geometry::toAnchor2D(const String& text) {
	for (auto i = 0; i < anchorName.size(); ++i)
		if (text == anchorName[i])
			return static_cast<Anchor2D>(i);
	return std::nullopt;
} //active::geometry::toAnchor2D


/*--------------------------------------------------------------------
	Get the text for a Anchor2D value
 
	anchor: The incoming anchor
 
	return: The anchor name as text
  --------------------------------------------------------------------*/
String active::geometry::fromAnchor2D(Anchor2D anchor) {
	return anchorName.at(static_cast<size_t>(anchor));
} //active::geometry::fromAnchor2D
