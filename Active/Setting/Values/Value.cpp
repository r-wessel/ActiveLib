/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Value.h"

#include <array>

using namespace active::setting;
using namespace active::utility;

namespace {
	
		//Names for fundamental value types
	std::array typeName{
		"null",
		"bool",
		"id",
		"int",
		"float",
		"string",
		"time",
	};
	
}  // namespace


/*--------------------------------------------------------------------
	Get a Value::Type enumerator from text
 
	text: The incoming text
 
	return: The equivalent type (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<Value::Type> Value::typeFromName(const String& text) {
	for (auto i = 0; i < typeName.size(); ++i)
		if (text == typeName[i])
			return static_cast<Type>(i);
	return std::nullopt;
} //Value::typeFromName


/*--------------------------------------------------------------------
	Get the text for a Value::Type
 
	type: The incoming type
 
	return: The type name as text
  --------------------------------------------------------------------*/
String Value::nameForType(Value::Type type) {
	return typeName.at(static_cast<size_t>(type));
} //Value::nameForType
