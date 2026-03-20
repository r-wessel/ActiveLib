/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ATTRIBUTE_COLOUR
#define ACTIVE_ATTRIBUTE_COLOUR

#include "Active/Utility/Hash.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/string/string_utf8.h"

namespace active::attribute {
	
	/*!
		Representation of a colour (for 2D/3D rendering)
	*/
	struct Colour {

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
		Colour(const String& hex);

		// MARK: - Public variables

			///Red component
		char8_t r = 0;
			///Green component
		char8_t g = 0;
			///Blue component
		char8_t b = 0;
			///Alpha component (0.0=transparent -> 1.0=opaque)
		float a = 1.0;

		// MARK: - Operators
		
		/*!
		 Equality operator
		 @param ref The object to compare
		 @return True if the objects are identical
		 */
		bool operator== (const Colour& ref) const { return (r == ref.r) && (g == ref.g) && (b == ref.b) && math::isEqual(a, ref.a); }

		// MARK: - Functions (const)

			///True if the colour is transparent
		bool isTransparent() const { return math::isZero(a); }
			///True if the colour is opaque
		bool isOpaque() const { return math::isEqual(a, 1.0); }
		/*!
			Get the colour in hex digits
			@param isAlpha True to include the alpha value
			@return The colour as hex
		*/
		String hex(bool isAlpha = false) const;
		/*!
		 Get a hash value for the colour
		 @param format The required has format
		 @return The hash formatted as specified
		 */
		String hash(HashFormat format = {}) const;

		// MARK: - Functions (mutating)
		
		/*!
		 Copy a specified colour
		 @param source The colour to copy
		 @param isAlphaCopied True if the alpha channel is also copied
		 @return A reference to this
		 */
		Colour& copy(const Colour& source, bool isAlphaCopied = true);
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
