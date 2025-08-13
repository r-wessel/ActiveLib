/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ATTRIBUTE_FINISH
#define ACTIVE_ATTRIBUTE_FINISH

#include "Active/Attribute/Colour.h"
#include "Active/Utility/Hash.h"

namespace active::attribute {
	
	/*!
		Representation of a surface finish (for 3D rendering)
	
		NB: This structure is likely to expand over time as additiona attributes are added to the definition of a finish, e.g. texture-mapping
	*/
	struct Finish {
		
		// MARK: Types

			///Optional
		using Option = std::optional<Finish>;
		
		// MARK: Constructors

		/*!
			Constructor
			@param col The finish colour
		*/
		Finish(const Colour& col = colour::white) : colour{col} {}
		/*!
			Constructor
			@param col The finish colour
			@param nm The finish name
			@param ident The finish identifier
		*/
		Finish(const Colour& col, const utility::String& nm, const utility::String& ident = utility::String{}) :
				colour{col}, name{nm}, id{ident} {}

		// MARK: Public variables

			///Finish colour (RGBA)
		Colour colour;
			///Specular colour (RGBA)
		Colour specularColour;
			///Emission colour (RGBA)
		Colour emissionColour;
			///Ambient reflection (0.0 -> 1.0)
		double ambientReflection = 0.0;
			///Diffuse reflection (0.0 -> 1.0)
		double diffuseReflection = 0.0;
			///Specular reflection (0.0 -> 1.0)
		double specularReflection = 0.0;
			///Metalness (0.0 -> 1.0)
		double metalness = 0.0;
			///Roughness (0.0 -> 1.0)
		double roughness = 1.0;
			///Finish name (optional - application-specific)
		utility::String name;
			///Finish identifier (optional - application-specific)
		mutable utility::String id;

		// MARK: Functions (const)

			///True if the finish is transparent
		bool isTransparent() const { return colour.isTransparent(); }
		/*!
		 Get a hash value for the finish
		 @param format The required has format
		 @return The hash formatted as specified
		 */
		utility::String hash(utility::HashFormat format = {}) const;
	};
	
}

#endif //ACTIVE_ATTRIBUTE_FINISH



