/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/UniqueString.h"

using namespace active;

/*--------------------------------------------------------------------
	Make a new unique string from a specified string
 
	seedString: The starting string
	addToExisting: True to add the new string to the set of m_existingSet strings
 
	return: A new unique string based on the specified string
  --------------------------------------------------------------------*/
string UniqueString::generate(const string& seedString, bool addToExisting) {
		string uniqueString{seedString};
		if (m_existingSet.contains(uniqueString)) {
			string textOnly{seedString};
			uint64_t suffixLen = 0;
			if (auto lastLetter = textOnly.find_last_not_of(string::allDigit); lastLetter) {
				textOnly = textOnly.substr(0, lastLetter + 1);
				suffixLen = uniqueString.length() - lastLetter - 1;
			}
			uint64_t topSuffix = 0;
			for (const auto& item : m_existingSet) {
				string text(item);
				if (textOnly.empty() || (text.find(textOnly) == 0)) {
					text = text.substr(textOnly.length());
					if (!text.empty() && !text.find_first_not_of(string::allDigit))
						topSuffix = std::max(topSuffix, text.operator uint64_t());
				}
			}
			string suffix(string{++topSuffix});
			suffix.pad_right(suffixLen, "0");
			uniqueString = textOnly + suffix;
		}
		m_existingSet.insert(uniqueString);
		return uniqueString;
} //UniqueString::generate
