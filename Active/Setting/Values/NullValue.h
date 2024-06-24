/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_NULL_VALUE
#define ACTIVE_SETTING_NULL_VALUE

#include "Active/Setting/Values/Value.h"

namespace active::setting {
	
	/*!
		A class to represent a setting for a void (non-existant) value
	 
		This can be used in cases like JSON parsing where a value can be explicity assigned a 'null' (for example). The object will discard all
		assigned values without signalling an error, and returns null/empty responses for every value request
	*/
	class NullValue : public Value {
	public:
		
		/*!
			Default constructor
		*/
		NullValue() {}

		/*!
			Clone method
			@return A clone of this object
		*/
		Value* clonePtr() const override { return new NullValue(); }

		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref A value to compare with this
			@return True if this is identical to ref
		*/
		bool operator==(const Value& ref) const override { return ref.isNull(); }
		/*!
			Less-than operator
			@param ref A value to compare with this
			@return True if this is less than ref
		*/
		bool operator<(const Value& ref) const override { return !ref.isNull(); }	//Null is less than anything except another null
		/*!
			Assignment operator
			@param val A value to assign
			@return A reference to this
		*/
		Value& operator=(const Value& val) override { return *this; }
		/*!
			Assignment operator
			@param val A boolean value to assign
			@return A reference to this
		*/
		Value& operator=(bool val) override { return *this; }
		/*!
			Assignment operator
			@param val A 32-bit integer value to assign
			@return A reference to this
		*/
		Value& operator=(int32_t val) override { return *this; }
		/*!
			Assignment operator
			@param val An unsigned 32-bit integer value to assign
			@return A reference to this
		*/
		Value& operator=(uint32_t val) override { return *this; }
		/*!
			Assignment operator
			@param val A 64-bit integer value to assign
			@return A reference to this
		*/
		Value& operator=(int64_t val) override { return *this; }
		/*!
			Assignment operator
			@param val A double precision value to assign
			@return A reference to this
		*/
		Value& operator=(double val) override { return *this; }
		/*!
			Assignment operator
			@param val A string value to assign
			@return A reference to this
		*/
		Value& operator=(const active::utility::String& val) override { return *this; }
		/*!
			Assignment operator
			@param val A guid value to assign
			@return A reference to this
		*/
		Value& operator=(const active::utility::Guid& val) override { return *this; }
		/*!
			Assignment operator
			@param val A time value to assign
			@return A reference to this
		*/
		Value& operator=(const active::utility::Time& val) override { return *this; }
		
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
		operator active::utility::String() const override { return utility::String(); }
		/*!
			Get a guid value
			@return A guid value
		*/
		operator active::utility::Guid() const override { return utility::Guid(); }
		/*!
			Get a time value
			@return A time value
		*/
		operator active::utility::Time() const override  { return utility::Time(); }
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the value is null, e.g. zero, empty, undefined
			@return True if the value is null
		*/
		bool isNull() const override { return true; }
		/*!
			Get the value type
			@return The value type
		*/
		Type getType() const override { return null; }
		
		// MARK: - Functions (mutating)
		
		/*!	Set a default value */
		void setDefault() override {}
	};
	
}

#endif	//ACTIVE_SETTING_NULL_VALUE
