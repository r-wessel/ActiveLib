/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Item/Item.h"

#include "Active/Setting/Values/Value.h"

using namespace active::serialise;
using namespace active::utility;
using namespace active::setting;

/*--------------------------------------------------------------------
	Read the cargo data from the specified setting
 
	source: The setting to read
 
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool Item::read(const Value& source) {
	String text = source;
	return read(text);	//The default behaviour is to treat the incoming data as a string - subclasses can override for exceptions
} //Item::read
