/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/UniqueString.h"

using namespace active::utility;

/*--------------------------------------------------------------------
	Make a new unique string from a specified string
 
	seedString: The starting string
	addToExisting: True to add the new string to the set of m_existingSet strings
 
	return: A new unique string based on the specified string
  --------------------------------------------------------------------*/
String UniqueString::generate(const String& seedString, bool addToExisting) {
		String uniqueString{seedString};
		if (m_existingSet.contains(uniqueString)) {
			String textOnly{seedString};
			uint32_t suffixLen = 0;
			if (auto lastLetter = textOnly.findLastNotOf(String::allDigit); lastLetter) {
				textOnly = textOnly.substr(0, *lastLetter + 1);
				suffixLen = static_cast<uint32_t>(uniqueString.length() - *lastLetter - 1);
			}
			uint32_t topSuffix = 0;
			for (const auto& item : m_existingSet) {
				String text(item);
				if (textOnly.empty() || (text.find(textOnly) == 0)) {
					text = text.substr(textOnly.length());
					if (!text.empty() && !text.findFirstNotOf(String::allDigit))
						topSuffix = math::maxVal(topSuffix, text.operator uint32_t());
				}
			}
			String suffix(String{++topSuffix});
			suffix.padRight(suffixLen, "0");
			uniqueString = textOnly + suffix;
		}
		m_existingSet.insert(uniqueString);
		return uniqueString;
} //UniqueString::generate
