/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_INT32_VALUE
#define ACTIVE_SETTING_INT32_VALUE

#include "Active/Setting/Values/ValueBase.h"

namespace active::setting {
	
		///A single 32-bit integer value
	using Int32Value = ValueBase<int32_t>;

	// MARK: - Operators
	
	/*!
		Assignment operator
		@param val A boolean value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(bool val) {
		data = val ? 1 : 0;
		return *this;
	}
	/*!
		Assignment operator
		@param val A 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(int32_t val) {
		data = val;
		return *this;
	}
	/*!
		Assignment operator
		@param val An unsigned 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(uint32_t val) {
		data = static_cast<int32_t>(val);
		return *this;
	}
	/*!
		Assignment operator
		@param val A 64-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(int64_t val) {
		data = static_cast<int32_t>(val);
		return *this;
	}
	/*!
		Assignment operator
		@param val A double precision value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(double val) {
		data = static_cast<int32_t>(math::round(val, 1.0));
		return *this;
	}
	/*!
		Assignment operator
		@param val A string value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(const utility::String& val) {
		if (auto intValue = val.toInt32(); intValue)
			data = *intValue;
		else
			status = bad;
		return *this;
	}
	/*!
		Assignment operator
		@param val A time value to assign
		@return A reference to this
	*/
	template<> inline
	Value& Int32Value::operator=(const utility::Time& val) {
		data = static_cast<int32_t>(val.secondsSince1970());
		return *this;
	}
	
	// MARK: - Conversion operators

	/*!
		Get a boolean value
		@return A boolean value
	*/
	template<> inline
	Int32Value::operator bool() const { return (data != 0); }
	/*!
		Get a 32-bit integer value
		@return A 32-bit integer value
	*/
	template<> inline
	Int32Value::operator int32_t() const { return data; }
	/*!
		Get an unsigned 32-bit integer value
		@return An unsigned 32-bit integer value
	*/
	template<> inline
	Int32Value::operator uint32_t() const { return static_cast<uint32_t>(data); }
	/*!
		Get a 64-bit integer value
		@return A 64-bit integer value
	*/
	template<> inline
	Int32Value::operator int64_t() const 	{ return data; }
	/*!
		Get a double precision value
		@return A double precision value
	*/
	template<> inline
	Int32Value::operator double() const { return data; }
	/*!
		Get a string value
		@return A string value
	*/
	template<> inline
	Int32Value::operator utility::String() const 	{ return utility::String{data}; }
	/*!
		Get a time value
		@return A time value
	*/
	template<> inline
	Int32Value::operator utility::Time() const { return utility::Time{static_cast<int64_t>(data)}; }
	
	// MARK: - Functions (const)
	
	/*!
		Determine if the value is null, e.g. zero, empty, undefined
		@return True if the value is null
	*/
	template<> inline
	bool Int32Value::isNull() const { return (data == 0); }
	/*!
		Get the value type
		@return The value type
	*/
	template<> inline
	Value::Type Int32Value::getType() const { return intType; };
	
}

#endif	//ACTIVE_SETTING_INT32_VALUE
