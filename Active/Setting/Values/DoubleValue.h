/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_DOUBLE_VALUE
#define ACTIVE_SETTING_DOUBLE_VALUE

#include "Active/Setting/Values/ValueBase.h"
#include "Active/Utility/MathFunctions.h"

namespace active::setting {
	
		///A double precision floating point value
	using DoubleValue = ValueBase<double>;

	// MARK: - Operators
	
	/*!
		Assignment operator
		@param val A boolean value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(bool val) {
		data = val ? 1.0 : 0.0;
		status = good;
		return *this;
	}
	/*!
		Assignment operator
		@param val A 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(int32_t val) {
		data = static_cast<double>(val);
		status = good;
		return *this;
	}
	/*!
		Assignment operator
		@param val An unsigned 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(uint32_t val) {
		data = static_cast<double>(val);
		status = good;
		return *this;
	}
	/*!
		Assignment operator
		@param val A 64-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(int64_t val) {
		data = static_cast<double>(val);
		status = good;
		return *this;
	}
	/*!
		Assignment operator
		@param val A double precision value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(double val) {
		data = val;
		status = good;
		return *this;
	}
	/*!
		Assignment operator
		@param val A string value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(const utility::String& val) {
		if (auto doubleValue = val.toDouble(); doubleValue) {
			data = *doubleValue;
			status = good;
		} else
			status = bad;
		return *this;
	}
	/*!
		Assignment operator
		@param val A string value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(const char* val) {
		return operator=(utility::String{val});
	}
	/*!
		Assignment operator
		@param val A time value to assign
		@return A reference to this
	*/
	template<> inline
	Value& DoubleValue::operator=(const utility::Time& val) {
		data = static_cast<double>(val.secondsSince1970()) + (static_cast<double>(val.microsecond()) / 1e6);
		status = good;
		return *this;
	}
	
	// MARK: - Conversion operators

	/*!
		Get a boolean value
		@return A boolean value
	*/
	template<> inline
	DoubleValue::operator bool() const { return !math::isZero(data); }
	/*!
		Get a 32-bit integer value
		@return A 32-bit integer value
	*/
	template<> inline
	DoubleValue::operator int32_t() const { return static_cast<int32_t>(data); }
	/*!
		Get an unsigned 32-bit integer value
		@return An unsigned 32-bit integer value
	*/
	template<> inline
	DoubleValue::operator uint32_t() const { return static_cast<uint32_t>(data); }
	/*!
		Get a 64-bit integer value
		@return A 64-bit integer value
	*/
	template<> inline
	DoubleValue::operator int64_t() const 	{ return static_cast<int64_t>(data); }
	/*!
		Get a double precision value
		@return A double precision value
	*/
	template<> inline
	DoubleValue::operator double() const { return data; }
	/*!
		Get a string value
		@return A string value
	*/
	template<> inline
	DoubleValue::operator utility::String() const 	{ return utility::String{data}; }
	/*!
		Get a time value
		@return A time value
	*/
	template<> inline
	DoubleValue::operator utility::Time() const { return utility::Time{data}; }
	
	// MARK: - Functions (const)
	
	/*!
		Determine if the value is null, e.g. zero, empty, undefined
		@return True if the value is null
	*/
	template<> inline
	bool DoubleValue::isNull() const { return math::isZero(data); }
	/*!
		Get the value type
		@return The value type
	*/
	template<> inline
	Value::Type DoubleValue::getType() const { return floatType; };

}

#endif	//ACTIVE_SETTING_DOUBLE_VALUE
