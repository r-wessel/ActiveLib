/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_VALUE_OPTION_WRAP
#define ACTIVE_SERIALISE_VALUE_OPTION_WRAP

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

namespace active::serialise {
	
	/*!
		Interface for a lightweight wrapper for passing a reference to optional item data into (de)serialise functions
	 
		Template parameters are:
		T: Value native type
	*/
	template<class T>
	class ValueOptionWrap : public ValueWrap<std::optional<T>> {
	public:
		
		// MARK: - Types
		
			///Base type
		using base = ValueWrap<std::optional<T>>;
			///Optional value type
		using value_t = base::value_t;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param val The optional item value
		*/
		ValueOptionWrap(value_t& val) : base(val) {}
		/*!
			Constructor
			@param val The item value
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		ValueOptionWrap(const value_t& val) : base(const_cast<value_t&>(val)) {}
		
		// MARK: - Functions (const)

		/*!
			Determine if the cargo has a null value (undefined)
			@return True if the cargo data is undefined
		*/
		bool isNull() const override { return !base::get().operator bool(); }
		/*!
			Write the item to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override {
			if (isNull())
				return false;	//Should not be attempting to write a null value to a string (null != "")
			dest = utility::String{*base::get()};
			return true;
		}
		
		// MARK: - Functions (mutating)

		/*!
			Read the item from a string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override {
			if (isNull())
				base::get() = T{};	//If we're reading into an unassigned optional, we need to assign a value so it doesn't register as null
			return base::read(source);
		}
		/*!
			Set to the default package content
		*/
		void setDefault() override	{
			base::get().reset();	//The default for an optional value is null
		}
		/*!
			Get the serialisation type for the item value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Item::Type> type() const override {
			if constexpr (std::is_base_of_v<utility::String, T> || std::is_base_of_v<utility::Guid, T>)
				return Item::Type::text;
			else if constexpr (std::is_same_v<bool, T>)
				return Item::Type::boolean;
			return Item::Type::number;
		}	//Other types should specialise accordingly
	};

	// MARK: - Specialisations for String

	/*!
		Write the item to a string (specialisation for guid)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueOptionWrap<utility::Guid>::write(utility::String& dest) const {
		if (isNull())
			return false;	//Should not be attempting to write a null value to a string (null != "")
		dest = *base::get();
		return true;
	}
	
	// MARK: - Specialisations for bool

	/*!
		Export the object to the specified string (specialisation for bool)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueOptionWrap<bool>::write(utility::String& dest) const {
		if (isNull())
			return false;	//Should not be attempting to write a null value to a string (null != "")
		dest = *get() ? "true" : "false";
		return true;
	} //ValueOptionWrap<bool>::write
	
		///Convenience wrapper names
	using BoolOptWrap = ValueOptionWrap<bool>;
	using DoubleOptWrap = ValueOptionWrap<double>;
	using FloatOptWrap = ValueOptionWrap<float>;
	using Int32OptWrap = ValueOptionWrap<int32_t>;
	using Int64OptWrap = ValueOptionWrap<int64_t>;
	using StringOptWrap = ValueOptionWrap<utility::String>;
	using UInt32OptWrap = ValueOptionWrap<uint32_t>;

}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_VALUE_OPTION_WRAP
