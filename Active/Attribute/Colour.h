/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ATTRIBUTE_COLOUR
#define ACTIVE_ATTRIBUTE_COLOUR

#include "Active/Utility/String.h"

namespace active::attribute {
	
	/*!
		Representation of a colour (for 2D/3D rendering)
	*/
	struct Colour {

		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Colour>;
			///Shared pointer
		using Shared = std::shared_ptr<Colour>;
			///Optional
		using Option = std::optional<Colour>;

		// MARK: - Constructors

		/*!
			Default constructor
		*/
		Colour() = default;
		/*!
			Default constructor
		*/
		Colour(char8_t red, char8_t green = 0, char8_t blue = 0, float alpha = 1.0) : r{red}, g{green}, b{blue}, a{alpha} {}
		/*!
			Constructor
			@param hex A colour in hex digits
		*/
		Colour(const utility::String& hex);

		// MARK: - Public variables

			///Red component
		char8_t r = 0;
			///Green component
		char8_t g = 0;
			///Blue component
		char8_t b = 0;
			///Alpha component
		float a = 1.0;

		// MARK: - Functions (const)

			///True if the colour is transparent
		bool isTransparent() const { return math::isZero(a); }
		/*!
			Get the colour in hex digits
			@param isAlpha True to include the alpha value
			@return The colour as hex
		*/
		utility::String hex(bool isAlpha = false) const;
	};
	
	namespace colour {
		
			 ///Red
		 inline Colour red{255};
			 ///Green
		 inline Colour green{0, 255};
			 ///Blue
		 inline Colour blue{0, 0, 255};
			 ///Black
		 inline Colour black{};
			 ///White
		 inline Colour white{255, 255, 255};

	}

}

#endif //ACTIVE_ATTRIBUTE_COLOUR
