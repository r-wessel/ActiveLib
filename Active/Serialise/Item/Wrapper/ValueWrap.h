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
#include "Active/string/string_utf8.h"

#include <functional>

namespace active::serialise {
	
	/*!
		Interface for a lightweight wrapper to pass a value item reference into (de)serialise functions
		Note: This class does not own the referenced data - the referenced variable must persist for the expected lifetime of the referencing instance, i.e. it will not be suitable for a stack-based value. This enables sending from or receiving into the referenced variable. Use `ValueHold` for sending temporary values
	 
		@tparam T Value native type
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
		bool write(string& dest) const override {
			if constexpr (requires (string& v) { v = base::get(); }) {
				dest = base::get();
				return true;
			}
			if constexpr (requires (string & v) { v = active::string{base::get()}; }) {
				dest = active::string{base::get()};
				return true;
			}
			if constexpr (active::Dereferenceable<T>) {
				return !isNull();	//Should not be attempting to write a null value to a string (null != "")
			}
			return false;
		}
		
		// MARK: - Functions (mutating)

		/*!
			Read the item from a string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const string& source) override {
				//If Value supports conversion to this type, assign directly
			if constexpr(requires (string& v) { base::get() = T{v}; }) {
				base::get() = T{source};
				return true;
			}
			if constexpr (active::Dereferenceable<T>) {
				if (!isNull()) {
						//TODO: Investigate if an object could use a string in some context
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
		bool readSetting(const setting::Value& source) override {
				//If Value supports conversion to this type, assign directly
			if constexpr(requires (const setting::Value& v) { base::get() = v; }) {
				base::get() = source;
				return true;
			}
			string text = source;
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
		std::optional<Cargo::Type> type() const override {
			if constexpr (std::is_base_of_v<string, T> || std::is_base_of_v<Guid, T> || std::is_enum_v<T>)
				return Cargo::Type::text;
			else if constexpr (std::is_same_v<bool, T>)
				return Cargo::Type::boolean;
			return Cargo::Type::number;
		}	//Other types should specialise accordingly
	};

	// MARK: - Specialisations for guid

	/*!
		Write the item to a string (specialisation for guid)
		@param dest The string to write the data to
		@return True if the data was successfully written
	*/
	template<> inline
	bool ValueWrap<Guid>::write(string& dest) const {
		dest = base::get();
		return true;
	}
	
	// MARK: - Specialisations for bool

	/*!
		Read a boolean value from the specified string
		@param source The string to read
		@return True if the data was successfully read
	*/
	inline std::pair<bool, bool> readBoolRefValue(const string& source) {
		bool incoming = false;
		string value = source.to_lower();
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
	bool ValueWrap<bool>::read(const string& source) {
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
	bool ValueWrap<std::optional<bool>>::read(const string& source) {
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
	bool ValueWrap<bool>::write(string& dest) const {
		dest = get() ? "true" : "false";
		return true;
	} //ValueWrap<bool>::write
	
		///Convenience wrapper names
	using BoolWrap = ValueWrap<bool>;
	using DoubleWrap = ValueWrap<double>;
	using FloatWrap = ValueWrap<float>;
	using Int32Wrap = ValueWrap<int32_t>;
	using Int64Wrap = ValueWrap<int64_t>;
	using StringWrap = ValueWrap<string>;
	using UInt32Wrap = ValueWrap<uint32_t>;

}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_VALUE_WRAP
