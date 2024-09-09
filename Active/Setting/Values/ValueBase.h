/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_VALUE_BASE
#define ACTIVE_SETTING_VALUE_BASE

#include "Active/Setting/Values/Value.h"

#include <type_traits>

namespace active::setting {

	/*!
		Template class for all single-item setting values, implementing some of the base Value functions
	 
		Refer to the base Value class for more information about the purpose of this class
	*/
	template<class T> requires std::copyable<T>
	class ValueBase : public Value {
	public:
		
		// MARK: - Constructors

		/*!
			Default constructor
		*/
		ValueBase() : Value(undefined) {}
		/*!
			Constructor
			@param val The value to assign
			@param stat The value status
		*/
		ValueBase(const T& val, Status stat = good) : Value(stat) { data = val; }
		/*!
			Constructor
			@param value A value to assign
		*/
		ValueBase(const Value& value) { data = value; }
		/*!
			Constructor (for all non-string types)
			@param stringVal A string value to assign
		*/
		ValueBase(const utility::String& stringVal) requires (!std::is_same<T, utility::String>::value) : Value(bad) { *this = stringVal; }
		
		ValueBase(const ValueBase& source) = default;
		
		/*!
			Clone method
			@return A clone of this object
		*/
		Value* clonePtr() const override { return new ValueBase<T>{*this}; };
		
		// MARK: - Public variables
		
			///The value data
		T data = T();
		
		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref A value to compare with this
			@return True if this is identical to ref
		*/
		bool operator==(const Value& ref) const override { return data == ref.operator T(); }
		/*!
			Inequality operator
			@param ref A value to compare with this
			@return True if this is not identical to ref
		*/
		bool operator!=(const Value& ref) const { return !((*this) == ref); }
		/*!
			Less-than operator
			@param ref A value to compare with this
			@return True if this is less than ref
		*/
		bool operator<(const Value& ref) const override { return data < ref.operator T(); }
		/*!
			Assignment operator
			@param val A value to assign
			@return A reference to this
		*/
		Value& operator=(const Value& val) override {
			data = val;
			return *this;
		}
		/*!
			Assignment operator
			@param val A boolean value to assign
			@return A reference to this
		*/
		Value& operator=(bool val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val A 32-bit integer value to assign
			@return A reference to this
		*/
		Value& operator=(int32_t val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val An unsigned 32-bit integer value to assign
			@return A reference to this
		*/
		Value& operator=(uint32_t val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val A 64-bit integer value to assign
			@return A reference to this
		*/
		Value& operator=(int64_t val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val A double precision value to assign
			@return A reference to this
		*/
		Value& operator=(double val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val A string value to assign
			@return A reference to this
		*/
		Value& operator=(const active::utility::String& val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val A string value to assign
			@return A reference to this
		*/
		Value& operator=(const char* val) override { return Value::operator=(val); }
		/*!
			Assignment operator
			@param val A guid value to assign
			@return A reference to this
		*/
		Value& operator=(const active::utility::Guid& val) override { status = bad; return *this; }
		/*!
			Assignment operator
			@param val A time value to assign
			@return A reference to this
		*/
		Value& operator=(const active::utility::Time& val) override { status = bad; return *this; }
		
		// MARK: - Conversion operators

		/*!
			Get a boolean value
			@return A boolean value
		*/
		operator bool() const override { return false; }
		/*!
			Get a 32-bit integer value
			@return A 32-bit integer value
		*/
		operator int32_t() const override { return 0; }
		/*!
			Get an unsigned 32-bit integer value
			@return An unsigned 32-bit integer value
		*/
		operator uint32_t() const override { return 0; }
		/*!
			Get a 64-bit integer value
			@return A 64-bit integer value
		*/
		operator int64_t() const override { return 0; }
		/*!
			Get a double precision value
			@return A double precision value
		*/
		operator double() const override { return 0.0; }
		/*!
			Get a string value
			@return A string value
		*/
		operator active::utility::String() const override { return active::utility::String{}; }
		/*!
			Get a guid value
			@return A guid value
		*/
		operator active::utility::Guid() const override { return active::utility::Guid{}; }
		/*!
			Get a time value
			@return A time value
		*/
		operator active::utility::Time() const override { return active::utility::Time{}; }
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the value is null, e.g. zero, empty, undefined
			@return True if the value is null
		*/
		bool isNull() const override { return data == T{}; }
		/*!
			Get the value type
			@return The value type
		*/
		Type getType() const override { return null; }
		
		// MARK: - Functions (mutating)
		
		/*!	Set a default value */
		void setDefault() override	{ data = T{}; }
	};

	template<> inline
	Value& ValueBase<utility::Guid>::operator=(const Value& val) {
		data = val.operator utility::Guid();
		return *this;
	}

}

#endif	//ACTIVE_SETTING_VALUE_BASE
