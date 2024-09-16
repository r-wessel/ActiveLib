/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_VALUE_WRAP
#define ACTIVE_SERIALISE_VALUE_WRAP

#include "Active/Serialise/Item/Item.h"
#include "Active/Serialise/Item/Wrapper/ValueItem.h"
#include "Active/Setting/Values/Value.h"
#include "Active/Utility/Concepts.h"
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
	class ValueWrap : public virtual ValueItem, public Item, public std::reference_wrapper<T> {
	public:
		
		// MARK: - Types
		
			///Item reference base
		using base = std::reference_wrapper<T>;
			///Item value type
		using value_t = T;

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
			if constexpr (active::utility::Dereferenceable<T>) {
				return false;	//Should not be attempting to write a null value to a string (null != "")
			} else  if constexpr (std::is_convertible_v<T, active::utility::String>) {
				dest = base::get();
				return true;
			} else {
				dest = active::utility::String{base::get()};
				return true;
			}
		}
		
		// MARK: - Functions (mutating)

		/*!
			Read the item from a string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override {
				//If Value supports conversion to this type, assign directly
			if constexpr(requires (setting::Value& v) { base::get() = T{v}; }) {
				base::get() = T{source};
				return true;
			}
			if constexpr (active::utility::Dereferenceable<T>) {
				if (!isNull()) {
					base::get() = T{source};
					return true;
				}
			}
			return false;
		}
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		bool read(const setting::Value& source) override {
				//If Value supports conversion to this type, assign directly
			if constexpr(requires (setting::Value& v) { base::get() = v; }) {
				base::get() = source;
				return true;
			}
			if constexpr (active::utility::Dereferenceable<T>) {
				if (isNull())
					return false;	//Should not be attempting to read into a null value to a string (null != "")
			}
			utility::String text = source;
			return read(text);	//Otherwise use a string as an intermediate value
		}
		/*!
			Set to the default package content
		*/
		void setDefault() override	{}
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
	
	// MARK: - Specialisations for bool

	/*!
		Read a boolean value from the specified string
		@param source The string to read
		@return True if the data was successfully read
	*/
	inline std::pair<bool, bool> readBoolRefValue(const utility::String& source) {
		bool incoming = false;
		utility::String value = source.lowercase();
		if ((value == "true") || (value == "1"))
			incoming = true;
		else if ((value == "false") || (value == "0"))
			incoming = false;
		else
			return {incoming, false};
		return {incoming, true};
	} //ValueWrap<bool>::read


	/*!
		Import the object from the specified string (specialisation for bool)
		@param source The string to read
		@return True if the data was successfully read
	*/
	template<> inline
	bool ValueWrap<bool>::read(const utility::String& source) {
		auto result = readBoolRefValue(source);
		if (result.second)
			base::get() = result.first;
		return result.second;
	} //ValueWrap<bool>::read
	

	/*!
		Import the object from the specified string (specialisation for bool)
		@param source The string to read
		@return True if the data was successfully read
	*/
	template<> inline
		bool ValueWrap<std::optional<bool>>::read(const utility::String& source) {
		auto result = readBoolRefValue(source);
		if (result.second)
			base::get() = result.first;
		return result.second;
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
