/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_DATA_FORMAT
#define ACTIVE_UTILITY_DATA_FORMAT

#include "Active/Utility/Memory.h"
#include "Active/Utility/TextEncoding.h"

#include <bit>

namespace active::utility {
	
	/*!
		A structure specifying i/o data formatting
	*/
	struct DataFormat {
		
		// MARK: - Types
			
			//Optional format
		using Option = std::optional<DataFormat>;
		
		inline static constexpr bool defaultEndian{std::endian::native == std::endian::big};
		
		/*!
			Create a data format from a BOM signature
			@param bom A series of bytes to test for a BOM signature
			@param len The number of available bytes
			@return The data format paired with the number of bytes in the BOM (nullopt on failure)
		*/
		static std::optional<std::pair<DataFormat, Memory::size_type>> fromBOM(const char* bom, Memory::size_type len);
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		DataFormat() {}
		/*!
			Constructor
			@param withBOM True if the text is preceded by a BOM
			@param asBigEndian True if byte ordering is big-endian
		*/
		DataFormat(bool withBOM, bool asBigEndian = defaultEndian) : isBigEndian{asBigEndian}, isBOM{withBOM} {}
		/*!
			Constructor
			@param encode The text encoding
			@param withBOM True if the text is preceded by a BOM
			@param asBigEndian True if byte ordering is big-endian
		*/
		DataFormat(TextEncoding encode, bool withBOM = false, bool asBigEndian = defaultEndian) :
					encoding{encode}, isBigEndian{asBigEndian}, isBOM{withBOM} {}
		
		// MARK: - Public variables
		
			///The text encoding
		TextEncoding encoding = TextEncoding::UTF8;
			///True if byte ordering is big-endian
		bool isBigEndian = defaultEndian;
			///True if the text is preceded by a BOM
		bool isBOM = false;
		
		// MARK: - Functions (const)
		
		/*!
			Get a BOM signature for the format
			@return The BOM signature (nullopt if none defined, e.g. for ascii)
		*/
		Memory::Option toBOM() const;
	};

}  // namespace active::utility

#endif	//ACTIVE_UTILITY_DATA_FORMAT
