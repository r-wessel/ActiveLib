/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_GUID_VALUE
#define ACTIVE_SETTING_GUID_VALUE

#include "Active/Setting/Values/ValueBase.h"

namespace active::setting {
	
		///A unique ID (guid) value
	using GuidValue = ValueBase<utility::Guid>;

	// MARK: - Operators
	
	/*!
		Assignment operator
		@param val A guid value to assign
		@return A reference to this
	*/
	template<> inline
	Value& GuidValue::operator=(const utility::Guid& val) {
		utility::Guid temp{val};
		if (temp)
			data = temp;
		else
			status = bad;
		return *this;
	}
	
	// MARK: - Conversion operators

	/*!
		Get a boolean value
		@return A boolean value
	*/
	template<> inline
	GuidValue::operator bool() const { return data; }
	/*!
		Get a string value
		@return A string value
	*/
	template<> inline
	GuidValue::operator utility::String() const 	{ return data; }
	/*!
		Get a guid value
		@return A guid value
	*/
	template<> inline
	GuidValue::operator utility::Guid() const { return data; }
	
	// MARK: - Functions (const)
	
	/*!
		Determine if the value is null, e.g. zero, empty, undefined
		@return True if the value is null
	*/
	template<> inline
	bool GuidValue::isNull() const { return !data; }
	/*!
		Get the value type
		@return The value type
	*/
	template<> inline
	Value::Type GuidValue::getType() const { return idType; };
	
}

#endif	//ACTIVE_SETTING_GUID_VALUE
