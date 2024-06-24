/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_VALUE_WRAP
#define ACTIVE_SERIALISE_VALUE_WRAP

#include "Active/Serialise/Item/Item.h"
#include "Active/Setting/Values/Value.h"
#include "Active/Utility/Guid.h"
#include "Active/Utility/String.h"

#include <functional>

namespace active::serialise {
	
	/*!
		Interface for a lightweight wrapper for passing a reference to item data into (de)serialise functions
	 
		Template parameters are:
		T: Value native type
	*/
	template<class T>
	class ValueWrap : public Item, public std::reference_wrapper<T> {
	public:
		
		// MARK: - Types
		
			///Item reference base
		using base = std::reference_wrapper<T>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param val The item value
		*/
		ValueWrap(T& val) : Item(), base(val) {}
		/*!
			Constructor
			@param val The item value
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		ValueWrap(const T& val) : base(const_cast<T&>(val)) {}
		
		// MARK: - Functions (const)

		/*!
			Write the item to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override {
			dest = utility::String{base::get()};
			return true;
		}
		
		// MARK: - Functions (mutating)

		/*!
			Read the item from a string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override	{ base::get() = T{source}; return true; }
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		bool read(const setting::Value& source) override {
				//If Value supports conversion to this type, assign directly
			if constexpr(requires (setting::Value& v) { v.operator T(); })
				base::get() = source;
			else {
				utility::String text = source;
				return read(text);	//Otherwise use a string as an intermediate value
			}
			return true;
		}
		/*!
			Set to the default package content
		*/
		void setDefault() override	{}
		/*!
			Get the serialisation type for the item value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Item::Type> type() const override  { return Item::Type::number; }	//Other types should specialise accordingly
	};
	
	// MARK: - Specialisations for string

	/*!
		Get the serialisation type for the item value (specialisation for string)
		@return The item value serialisation type (JSON encoding requires specific type representations)
	*/
	template<> inline
	std::optional<Item::Type> ValueWrap<utility::String>::type() const { return Item::Type::text; }
	
	// MARK: - Specialisations for guid

	/*!
		Write the item to a string (specialisation for guid)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueWrap<utility::Guid>::write(utility::String& dest) const {
		dest = base::get();
		return true;
	}

	/*!
		Get the serialisation type for the item value (specialisation for guid)
		@return The item value serialisation type (JSON encoding requires specific type representations)
	*/
	template<> inline
	std::optional<Item::Type> ValueWrap<utility::Guid>::type() const { return Item::Type::text; }
	
	// MARK: - Specialisations for bool

	/*!
		Import the object from the specified string (specialisation for bool)
		@param source The string to read
		@return True if the data was successfully read
	*/
	template<> inline
	bool ValueWrap<bool>::read(const utility::String& source) {
		utility::String value = source.lowercase();
		if ((value == "true") || (value == "1"))
			base::get() = true;
		else if ((value == "false") || (value == "0"))
			base::get() = false;
		else
			return false;
		return true;
	} //ValueWrap<bool>::read
	
	
	/*!
		Export the object to the specified string (specialisation for bool)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueWrap<bool>::write(utility::String& dest) const {
		dest = get() ? "true" : "false";
		return true;
	} //ValueWrap<bool>::write
	
	
	/*!
		Get the serialisation type for the item value (specialisation for bool)
		@return The item value serialisation type (JSON encoding requires specific type representations)
	*/
	template<> inline
	std::optional<Item::Type> ValueWrap<bool>::type() const { return Item::Type::boolean; }
	
		///Convenience wrapper names
	using BoolWrap = ValueWrap<bool>;
	using DoubleWrap = ValueWrap<double>;
	using FloatWrap = ValueWrap<float>;
	using Int32Wrap = ValueWrap<int32_t>;
	using Int64Wrap = ValueWrap<int64_t>;
	using StringWrap = ValueWrap<utility::String>;
	using UInt32Wrap = ValueWrap<uint32_t>;

}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_VALUE_WRAP
