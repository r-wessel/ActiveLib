/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ATTRIBUTE_FINISH
#define ACTIVE_ATTRIBUTE_FINISH

#include "Active/Attribute/Colour.h"

namespace active::attribute {
	
	/**
		Representation of a surface finish (for 3D rendering)
	
		NB: This structure is likely to expand over time as additiona attributes are added to the definition of a finish, e.g. texture-mapping
	*/
	struct Finish {

		// MARK: Constructors

		/**
			Constructor
			@param col The finish colour
		*/
		Finish(const Colour& col = colour::white) : colour{col} {
		}

		// MARK: Public variables

			///Finish colour
		Colour colour;

		// MARK: Functions (const)

			///True if the finish is transparent
		bool isTransparent() const { return colour.isTransparent(); }
	};
	
}

#endif //ACTIVE_ATTRIBUTE_FINISH



