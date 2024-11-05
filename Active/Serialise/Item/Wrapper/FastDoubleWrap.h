/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_FAST_DOUBLE_WRAP
#define ACTIVE_SERIALISE_FAST_DOUBLE_WRAP

#include "Active/Serialise/Item/Item.h"
#include "Active/Serialise/Item/Wrapper/ValueItem.h"
#include "Active/Setting/Values/Value.h"
#include "Active/Utility/Concepts.h"
#include "Active/Utility/Guid.h"
#include "Active/Utility/String.h"

#include <functional>

namespace active::serialise {
	
	/*!
		Interface for a lightweight wrapper for passing a double into (de)serialise functions
	 
		NB: This wrapper is for special cases where speed in writing double values is paramount, e.g. this will write a fixed precision and
	 	trailing zeros are not removed (and therefore results in larger output)
	*/
	class FastDoubleWrap : public virtual ValueItem, public Item, public std::reference_wrapper<double> {
	public:
		
		// MARK: - Types
		
			///Item reference base
		using base = std::reference_wrapper<double>;

		// MARK: - Constructors
		
		/*!
			Constructor
			@param val The item value
		*/
		FastDoubleWrap(double& val) : Item(), base(val) {}
		/*!
			Constructor
			@param val The item value
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		FastDoubleWrap(const double& val) : base(const_cast<double&>(val)) {}
		
		// MARK: - Functions (const)

		/*!
			Write the item to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override {
			return dest.assign(get(), 5);
		}
		
		// MARK: - Functions (mutating)

		/*!
			Read the item from a string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override {
			base::get() = double{source};
			return true;
		}
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		bool readSetting(const setting::Value& source) override {
			utility::String text = source;
			return read(text);	//Otherwise use a string as an intermediate value
		}
		/*!
			Set to the default package content
		*/
		void setDefault() override	{ base::get() = 0.0; }
		/*!
			Get the serialisation type for the item value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Cargo::Type> type() const override {
			return Cargo::Type::number;
		}	//Other types should specialise accordingly
	};

}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_FAST_DOUBLE_WRAP
