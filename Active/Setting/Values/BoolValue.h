/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_BOOL_VALUE
#define ACTIVE_SETTING_BOOL_VALUE

#include "Active/Setting/Values/ValueBase.h"
#include "Active/Utility/MathFunctions.h"

namespace active::setting {
	
		///A single boolean value
	using BoolValue = ValueBase<bool>;
	
	// MARK: - Operators
	
	/*!
		Assignment operator
		@param val A boolean value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(bool val) {
		data = val;
		return *this;
	}
	/*!
		Assignment operator
		@param val A 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(int32_t val) {
		data = (val != 0);
		return *this;
	}
	/*!
		Assignment operator
		@param val An unsigned 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(uint32_t val) {
		data = (val != 0);
		return *this;
	}
	/*!
		Assignment operator
		@param val A 64-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(int64_t val) {
		data = (val != 0);
		return *this;
	}
	/*!
		Assignment operator
		@param val A double precision value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(double val) {
		data = !math::isZero(val);
		return *this;
	}
	/*!
		Assignment operator
		@param val A string value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(const utility::String& val) {
		if ((val.lowercase() == "true") || (val.lowercase() == "1"))
			data = true;
		else if ((val.lowercase() == "false") || (val.lowercase() == "0"))
			data = false;
		else {
			status = bad;
		}
		return *this;
	}
	/*!
		Assignment operator
		@param val A guid value to assign
		@return A reference to this
	*/
	template<> inline
	Value& BoolValue::operator=(const utility::Guid& val) {
		data = val;
		return *this;
	}
	
	// MARK: - Conversion operators

	/*!
		Get a boolean value
		@return A boolean value
	*/
	template<> inline
	BoolValue::operator bool() const { return data; }
	/*!
		Get a 32-bit integer value
		@return A 32-bit integer value
	*/
	template<> inline
	BoolValue::operator int32_t() const { return data ? 1 : 0; }
	/*!
		Get an unsigned 32-bit integer value
		@return An unsigned 32-bit integer value
	*/
	template<> inline
	BoolValue::operator uint32_t() const { return data ? 1 : 0; }
	/*!
		Get a 64-bit integer value
		@return A 64-bit integer value
	*/
	template<> inline
	BoolValue::operator int64_t() const 	{ return data ? 1 : 0; }
	/*!
		Get a double precision value
		@return A double precision value
	*/
	template<> inline
	BoolValue::operator double() const { return data ? 1.0 : 0.0; }
	/*!
		Get a string value
		@return A string value
	*/
	template<> inline
	BoolValue::operator utility::String() const 	{ return data ? "true" : "false"; }
	
	// MARK: - Functions (const)
	
	/*!
		Determine if the value is null, e.g. zero, empty, undefined
		@return True if the value is null
	*/
	template<> inline
	bool BoolValue::isNull() const { return !data; }
	/*!
		Get the value type
		@return The value type
	*/
	template<> inline
	Value::Type BoolValue::getType() const { return boolType; };
	
}

#endif	//ACTIVE_SETTING_BOOL_VALUE
