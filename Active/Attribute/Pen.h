/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ATTRIBUTE_PEN
#define ACTIVE_ATTRIBUTE_PEN

#include <array>
#include <optional>

namespace active::attribute {
	
	/*!
		Representation of a weighted pen (for 2D illustration)
	*/
	struct Pen {
		
		// MARK: - Types
		
			///Enumerator for standard pen weights
		enum Weight {
			hairline,	///< Always displays as a hairline at any zoom level
			thinnest,
			thinner,
			thin,
			medium,
			thick,
			thicker,
			thickest,
			heavy,
		};
		
			///Unique pointer
		using Unique = std::unique_ptr<Pen>;
			///Shared pointer
		using Shared = std::shared_ptr<Pen>;
			///Optional
		using Option = std::optional<Pen>;
		
		// MARK: - Static variables
		
			///Standard architectural pen weights
		static inline std::array standard {
			0.0,	///< Nominally the minimum renderable thickness, not literally zero
			0.18,
			0.25,
			0.35,
			0.5,
			0.7,
			1.0,
			1.4,
			2.0,
		};
		
		using enum Weight;
		
		// MARK: - Public variables
		
			///The pen weight
		double weight = standard[thinnest];
	};
	
}

#endif	//ACTIVE_ATTRIBUTE_PEN
