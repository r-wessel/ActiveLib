/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_TEXT_FORMAT
#define ACTIVE_TEXT_FORMAT

#include "Active/string/text_encoding.h"

#include <bit>
#include <optional>

namespace active {
	
	/*!
	 A structure specifying i/o text formatting
	 */
	struct text_format {
		
			// MARK: - Types
		
		static inline constexpr bool defaultEndian{std::endian::native == std::endian::big};
		
		/*!
		 Create a data format from a BOM signature
		 @param bom A series of bytes to test for a BOM signature
		 @param len The number of available bytes
		 @return The data format paired with the number of bytes in the BOM (nullopt on failure)
		 */
		static std::optional<std::pair<text_format, size_t>> from_bom(const char* bom, size_t len);
		
			// MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		text_format() {}
		/*!
		 Constructor
		 @param withBOM True if the text is preceded by a BOM
		 @param asBigEndian True if byte ordering is big-endian
		 */
		text_format(bool withBOM, bool asBigEndian = defaultEndian) : is_big_endian{asBigEndian}, is_bom{withBOM} {}
		/*!
		 Constructor
		 @param encode The text encoding
		 @param withBOM True if the text is preceded by a BOM
		 @param asBigEndian True if byte ordering is big-endian
		 */
		text_format(text_encoding encode, bool withBOM = false, bool asBigEndian = defaultEndian) :
		encoding{encode}, is_big_endian{asBigEndian}, is_bom{withBOM} {}
		
			// MARK: - Public variables
		
			///The text encoding
		text_encoding encoding = text_encoding::UTF8;
			///True if byte ordering is big-endian
		bool is_big_endian = defaultEndian;
			///True if the text is preceded by a BOM
		bool is_bom = false;
	};
	
}  // namespace active

#endif	//ACTIVE_TEXT_FORMAT
