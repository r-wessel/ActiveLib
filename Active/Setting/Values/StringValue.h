/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_STRING_VALUE
#define ACTIVE_SETTING_STRING_VALUE

#include "Active/Setting/Values/ValueBase.h"
#include "Active/Serialise/XML/Item/XMLDateTime.h"

namespace active::setting {
	
		///A single string value
	using StringValue = ValueBase<utility::String>;

	// MARK: - Operators
	
	/*!
		Assignment operator
		@param val A boolean value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(bool val) {
		data = val ? "true" : "false";
		return *this;
	}
	/*!
		Assignment operator
		@param val A 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(int32_t val) {
		data = utility::String{val};
		return *this;
	}
	/*!
		Assignment operator
		@param val An unsigned 32-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(uint32_t val) {
		data = utility::String{val};
		return *this;
	}
	/*!
		Assignment operator
		@param val A 64-bit integer value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(int64_t val) {
		data = utility::String{val};
		return *this;
	}
	/*!
		Assignment operator
		@param val A double precision value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(double val) {
		data = utility::String{val};
		return *this;
	}
	/*!
		Assignment operator
		@param val A string value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(const utility::String& val) {
		data = val;
		return *this;
	}
	/*!
		Assignment operator
		@param val A guid value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(const utility::Guid& val) {
		data = val;
		return *this;
	}
	/*!
		Assignment operator
		@param val A time value to assign
		@return A reference to this
	*/
	template<> inline
	Value& StringValue::operator=(const utility::Time& val) {
		data.clear();
		utility::Time temp{val};
		serialise::xml::XMLDateTime{temp}.write(data);
		return *this;
	}
	
	// MARK: - Conversion operators

	/*!
		Get a boolean value
		@return A boolean value
	*/
	template<> inline
	StringValue::operator bool() const { return ((data == "true") || (data == "1")); }
	/*!
		Get a 32-bit integer value
		@return A 32-bit integer value
	*/
	template<> inline
	StringValue::operator int32_t() const { return data.operator int32_t(); }
	/*!
		Get an unsigned 32-bit integer value
		@return An unsigned 32-bit integer value
	*/
	template<> inline
	StringValue::operator uint32_t() const { return data.operator uint32_t(); }
	/*!
		Get a 64-bit integer value
		@return A 64-bit integer value
	*/
	template<> inline
	StringValue::operator int64_t() const 	{ return data.operator int64_t(); }
	/*!
		Get a double precision value
		@return A double precision value
	*/
	template<> inline
	StringValue::operator double() const { return data.operator double(); }
	/*!
		Get a string value
		@return A string value
	*/
	template<> inline
	StringValue::operator utility::String() const 	{ return data; }
	/*!
		Get a guid value
		@return A guid value
	*/
	template<> inline
	StringValue::operator utility::Guid() const { return utility::Guid{data}; }
	/*!
		Get a time value
		@return A time value
	*/
	template<> inline
	StringValue::operator utility::Time() const {
			//First attempt to read the time as xs:dateTime format
		utility::Time time;
		serialise::xml::XMLDateTime xmlParser{time};
		if (xmlParser.read(data))
			return time;
			//Otherwise attempt to convert as seconds from 1970
		if (auto seconds = data.toInt64(); seconds)
			return utility::Time{*seconds};
		return utility::Time{};
	}
	
	// MARK: - Functions (const)
	
	/*!
		Determine if the value is null, e.g. zero, empty, undefined
		@return True if the value is null
	*/
	template<> inline
	bool StringValue::isNull() const { return !data.empty(); }
	/*!
		Get the value type
		@return The value type
	*/
	template<> inline
	Value::Type StringValue::getType() const { return stringType; };
	
}

#endif	//ACTIVE_SETTING_STRING_VALUE
