/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_TIME_VALUE
#define ACTIVE_SETTING_TIME_VALUE

#include "Active/Setting/Values/ValueBase.h"
#include "Active/Serialise/XML/Item/XMLDateTime.h"

namespace active::setting {
	
		///A single time value
	using TimeValue = ValueBase<utility::Time>;

	// MARK: - Operators
	
	/*!
		Assignment operator
		@param val A boolean value to assign
		@return A reference to this
	*/
	template<> inline
	Value& TimeValue::operator=(int32_t val) {
		data.setSecondsSince1970(val);
		return *this;
	}
	/*!
		Assignment operator
		@param val An unsigned 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& TimeValue::operator=(uint32_t val) {
		data.setSecondsSince1970(val);
		return *this;
	}
	/*!
		Assignment operator
		@param val A 64-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& TimeValue::operator=(int64_t val) {
		data.setSecondsSince1970(static_cast<int32_t>(val));
		return *this;
	}
	/*!
		Assignment operator
		@param val A double precision value to assign
		@return A reference to this
	*/
	template<> inline
	Value& TimeValue::operator=(double val) {
		data.setSecondsSince1970(static_cast<int32_t>(val));
		data.setMicrosecond(static_cast<uint32_t>(1e6 * fmod(val, 1.0)));
		return *this;
	}
	/*!
		Assignment operator
		@param val A string value to assign
		@return A reference to this
	*/
	template<> inline
	Value& TimeValue::operator=(const utility::String& val) {
		utility::Time time;
		serialise::xml::XMLDateTime xmlParser{time};
		if (xmlParser.read(val))
			data = time;
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
	Value& TimeValue::operator=(const utility::Time& val) {
		data = val;
		return *this;
	}
	
	// MARK: - Conversion operators

	/*!
		Get a boolean value
		@return A boolean value
	*/
	template<> inline
	TimeValue::operator bool() const { return false; }
	/*!
		Get a 32-bit integer value
		@return A 32-bit integer value
	*/
	template<> inline
	TimeValue::operator int32_t() const { return static_cast<int32_t>(data.secondsSince1970()); }
	/*!
		Get an unsigned 32-bit integer value
		@return An unsigned 32-bit integer value
	*/
	template<> inline
	TimeValue::operator uint32_t() const { return static_cast<uint32_t>(data.secondsSince1970()); }
	/*!
		Get a 64-bit integer value
		@return A 64-bit integer value
	*/
	template<> inline
	TimeValue::operator int64_t() const 	{ return static_cast<int64_t>(data.secondsSince1970()); }
	/*!
		Get a double precision value
		@return A double precision value
	*/
	template<> inline
	TimeValue::operator double() const { return static_cast<double>(data.secondsSince1970()) + (static_cast<double>(data.microsecond()) / 1e6); }
	/*!
		Get a string value
		@return A string value
	*/
	template<> inline
	TimeValue::operator utility::String() const 	{
		utility::String string;
		utility::Time temp{data};
		serialise::xml::XMLDateTime{temp}.write(string);
		return string;

	}
	/*!
		Get a time value
		@return A time value
	*/
	template<> inline
	TimeValue::operator utility::Time() const {
		return data;
	}
	
	// MARK: - Functions (const)
	
	/*!
		Determine if the value is null, e.g. zero, empty, undefined
		@return True if the value is null
	*/
	template<> inline
	bool TimeValue::isNull() const { return false; }
	/*!
		Get the value type
		@return The value type
	*/
	template<> inline
	Value::Type TimeValue::getType() const { return timeType; };
	
}

#endif	//ACTIVE_SETTING_TIME_VALUE
