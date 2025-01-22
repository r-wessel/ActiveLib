/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Primitives/2D/Text.h"

using namespace active::geometry;
using namespace active::primitive;
using namespace active::utility;

/*--------------------------------------------------------------------
	Constructor
 
	txt: The text
	origin: Origin point of the text (text is rendered relative to this position using the style anchor and offset)
	angl: Rotation angle of the text (about the origin point)
	styl: The text style
	offset: An offset from the origin (in render units, added after anchor and offset have been applied)
  --------------------------------------------------------------------*/
Text::Text(const utility::String& txt, const geometry::Point& origin, double angl,
							 const attribute::TextStyle styl, const geometry::Point& offset) :
		Point{origin}, text{txt}, angle{angl}, style{styl}, renderOffset{offset} {}
