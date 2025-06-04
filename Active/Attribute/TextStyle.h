/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ATTRIBUTE_TEXT_STYLE
#define ACTIVE_ATTRIBUTE_TEXT_STYLE

#include "Active/Attribute/Colour.h"
#include "Active/Attribute/Pen.h"
#include "Active/Geometry/Anchor2D.h"
#include "Active/Utility/String.h"

namespace active::attribute {
	
	/*!
		Collection of attributes defining a text sryle, e.g. font, size etc
	*/
	struct TextStyle {
		
		using enum geometry::Anchor2D;

		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		TextStyle() = default;
		/*!
			Constructor
			@param font Font
			@param sz Text size
			@param colour Text body colour
			@param anch Anchor position for the rectlinear text bounds
			@param orientate True if the text can be reorientated to suit reading standards
		 */
		TextStyle(const utility::String& font, float sz, const Colour& colour = colour::black, geometry::Anchor2D anch = centreHalf, bool orientate = true) :
				fontName{font}, size{sz}, bodyColour{colour}, anchor{anch}, isReorientated{orientate} {}
		
		// MARK: - Public variables
		
			///Font name (empty = system default)
		utility::String fontName;
			///Text size in points
		float size = 12.0;
			///Text body colour (nullopt = no fill colour, e.g. outlined)
		Colour::Option bodyColour = colour::black;
			///Text outline colour (nullopt = same as body colour)
		Colour::Option outlineColour;
			///Text outline pen
		Pen outlinePen{Pen::standard[Pen::hairline]};
			///Anchor position for the rectlinear text bounds
		geometry::Anchor2D anchor = centreHalf;
			///True if the text can be orientated to suit reading standards, e.g. upright
		bool isReorientated = true;
	};
	
}

#endif //ACTIVE_ATTRIBUTE_TEXT_STYLE
