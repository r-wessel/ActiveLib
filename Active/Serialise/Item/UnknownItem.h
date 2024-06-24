/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_UNKNOWN_ITEM
#define ACTIVE_SERIALISE_UNKNOWN_ITEM

#include "Active/Serialise/Item/Item.h"

namespace active::serialise {
	
	/*!
		A package for dealing with unknown/unwanted item cargo
	*/
	class UnknownItem : public Item {
	public:
		/*!
			Write the item data to a string
			@param dest The string to write the const data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override { return false; }	//This is read-only
		
		// MARK: - Functions (mutating)
		
		/*!
			Set to the default package content
		*/
		void setDefault() override {}
		/*!
			Read the item data from the specified string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override { return true; }
	};
	
}

#endif	//ACTIVE_SERIALISE_UNKNOWN_ITEM
