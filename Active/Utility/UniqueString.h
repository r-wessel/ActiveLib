/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_UNIQUE_STRING
#define ACTIVE_UTILITY_UNIQUE_STRING

#include "Active/Utility/String.h"

#include <set>

namespace active::utility {
		
	/*!
		A class representing a GUID type
	*/
	class UniqueString {
	public:
		
		// MARK: - Types
		
			///Optional
		using Set = std::set<String>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		UniqueString() {}
		/*!
			Constructor
		*/
		UniqueString(const UniqueString::Set existing) : m_existingSet{existing} {}
		
		// MARK: - Functions (mutating)
		
		/*!
			Make a new unique string from a specified string
			@param seedString The starting string
			@param addToExisting True to add the new string to the set of existing strings
			@return A new unique string based on the specified string
		*/
		String generate(const String& seedString, bool addToExisting = true);
		
	private:
			//Existing strings
		Set m_existingSet;
	};
	
}

#endif	//ACTIVE_UTILITY_UNIQUE_STRING
