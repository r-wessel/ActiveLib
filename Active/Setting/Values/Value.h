/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_VALUE
#define ACTIVE_SETTING_VALUE

#include "Active/Utility/Cloner.h"
#include "Active/Utility/Guid.h"
#include "Active/Utility/String.h"
#include "Active/Utility/Time.h"

namespace active::setting {
	
	/*!
		Base class for all single-item values
	 
		This base class essentially anonymises the value type, allowing a setting sender and receiver to work with the type it expects,
		e.g. a sender may have numeric value that the receiver will display as a string. This helps facilitate loose coupling between modules.
		Settings/parameters can share a common identity, but the senders/receivers of these values do not need a shared understanding
		of value types, formatting or purpose (which may vary if there are multiple receivers). This is particulary relevant to the UI,
		where the app business logic understands value types and formatting, but the UI must facilitate display/entry of values. The UI
		code can read/write to the value as a string irrespective of its internal type and allow the Value to validate user entry (meaning
		that no business logic for the value type is replicated or exposed in the UI code).
	 
		There are many other uses for this type abstraction, e.g. data passed with an Event to Subscribers.

		Subclasses should override the following functions as appropriate
	*/
	class Value : public active::utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Value>;
			///Shared pointer
		using Shared = std::shared_ptr<Value>;
			///Optional
		using Option = std::optional<Value>;
		
			///The value status (defines whether a value has been explicitly set and (if so) if it's meaningful
		enum class Status {
			undefined = 0,
			bad,
			good,
		};
		
			///Supported value types (broad groups, e.g. int32_t and int64_t are both intType)
		enum class Type {
			null = 0,
			boolType,
			idType,
			intType,
			floatType,
			stringType,
			timeType,
		};
		
		using enum Status;
		using enum Type;

		// MARK: - Static functions
		
		/*!
			Get a Value::Type enumerator from text
			@param text The incoming text
			@return The equivalent type (nullopt on failure)
		*/
		static std::optional<Type> typeFromName(const utility::String& text);
		/*!
			Get the text for a Value::Type
			@param type The incoming type
			@return The type name as text
		*/
		static utility::String nameForType(Type type);
		
		// MARK: - Constructors
		
		/*!
			Default constructor
			@param stat The value status
		*/
		Value(Status stat = good) { status = stat; }
		/*!
			Destructor
		*/
		virtual ~Value() = default;
		
		/*!
			Clone method
			@return A clone of this object
		*/
		Value* clonePtr() const override = 0;
		
		// MARK: - Public variables
		
			///The value status
		Status status;

		// MARK: - Operators
		
		/*!
			Equality operator
			@param ref A value to compare with this
			@return True if this is identical to ref
		*/
		virtual bool operator==(const Value& ref) const = 0;
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
		virtual bool operator<(const Value& ref) const = 0;
		/*!
			Assignment operator
			@param val A value to assign
			@return A reference to this
		*/
		virtual Value& operator=(const Value& val) = 0;
		/*!
			Assignment operator
			@param val A boolean value to assign
			@return A reference to this
		*/
		virtual Value& operator=(bool val) = 0;
		/*!
			Assignment operator
			@param val A 32-bit integer value to assign
			@return A reference to this
		*/
		virtual Value& operator=(int32_t val) = 0;
		/*!
			Assignment operator
			@param val An unsigned 32-bit integer value to assign
			@return A reference to this
		*/
		virtual Value& operator=(uint32_t val) = 0;
		/*!
			Assignment operator
			@param val A 64-bit integer value to assign
			@return A reference to this
		*/
		virtual Value& operator=(int64_t val) = 0;
		/*!
			Assignment operator
			@param val A double precision value to assign
			@return A reference to this
		*/
		virtual Value& operator=(double val) = 0;
		/*!
			Assignment operator
			@param val A string value to assign
			@return A reference to this
		*/
		virtual Value& operator=(const active::utility::String& val) = 0;
		/*!
			Assignment operator
			@param val A string value to assign
			@return A reference to this
		*/
		virtual Value& operator=(const char* val) { return operator=(utility::String{val}); }
		/*!
			Assignment operator
			@param val A guid value to assign
			@return A reference to this
		*/
		virtual Value& operator=(const active::utility::Guid& val) = 0;
		/*!
			Assignment operator
			@param val A time value to assign
			@return A reference to this
		*/
		virtual Value& operator=(const active::utility::Time& val) = 0;
		
		// MARK: - Conversion operators

		/*!
			Get a boolean value
			@return A boolean value
		*/
		virtual operator bool() const = 0;
		/*!
			Get a 32-bit integer value
			@return A 32-bit integer value
		*/
		virtual operator int32_t() const = 0;
		/*!
			Get an unsigned 32-bit integer value
			@return An unsigned 32-bit integer value
		*/
		virtual operator uint32_t() const = 0;
		/*!
			Get a 64-bit integer value
			@return A 64-bit integer value
		*/
		virtual operator int64_t() const = 0;
		/*!
			Get a double precision value
			@return A double precision value
		*/
		virtual operator double() const = 0;
		/*!
			Get a string value
			@return A string value
		*/
		virtual operator active::utility::String() const = 0;
		/*!
			Get a guid value
			@return A guid value
		*/
		virtual operator active::utility::Guid() const = 0;
		/*!
			Get a time value
			@return A time value
		*/
		virtual operator active::utility::Time() const = 0;
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the value has a good status
			@return True if the status is good
		*/
		virtual bool isGood() const { return (status == good); }
		/*!
			Determine if the value is null, e.g. zero, empty, undefined
			@return True if the value is null
		*/
		virtual bool isNull() const = 0;
		/*!
			Get the value type
			@return The value type
		*/
		virtual Type getType() const = 0;
		
		// MARK: - Functions (mutating)
		
		/*!	Set a default value */
		virtual void setDefault() = 0;
	};

	// MARK: - Value smart pointer comparisons
	
	/*!
		Equality operator
		@param lhs The left value to compare
		@param rhs The right value to compare
		@return True if the values are identical
	*/
	inline bool operator==(const Value::Unique& lhs, const Value::Unique& rhs) {
		return (!lhs || !rhs) ? !(!lhs ^ !rhs) : lhs->operator==(*rhs);
	}
	
	/*!
		Inequality operator
		@param lhs The left value to compare
		@param rhs The right value to compare
		@return True if the values are not identical
	*/
	inline bool operator!=(const Value::Unique& lhs, const Value::Unique& rhs) {
		return !(lhs == rhs);
	}
	
	/*!
		Less-than operator
		@param lhs The left value to compare
		@param rhs The right value to compare
		@return True if lhs is less than rhs
	*/
	inline bool operator<(const Value::Unique& lhs, const Value::Unique& rhs) {
		return (!lhs || !rhs) ? (!lhs && rhs) : lhs->operator<(*rhs);
	}
	
	/*!
		Equality operator
		@param lhs The left value to compare
		@param rhs The right value to compare
		@return True if the values are identical
	*/
	inline bool operator==(const Value::Shared& lhs, const Value::Shared& rhs) {
		return (!lhs || !rhs) ? !(!lhs ^ !rhs) : lhs->operator==(*rhs);
	}
	
	/*!
		Inequality operator
		@param lhs The left value to compare
		@param rhs The right value to compare
		@return True if the values are not identical
	*/
	inline bool operator!=(const Value::Shared& lhs, const Value::Shared& rhs) {
		return !(lhs == rhs);
	}
	
	/*!
		Less-than operator
		@param lhs The left value to compare
		@param rhs The right value to compare
		@return True if lhs is less than rhs
	*/
	inline bool operator<(const Value::Shared& lhs, const Value::Shared& rhs) {
		return (!lhs || !rhs) ? (!lhs && rhs) : lhs->operator<(*rhs);
	}
	
}

#endif	//ACTIVE_SETTING_VALUE
