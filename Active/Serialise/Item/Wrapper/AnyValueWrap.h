/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_ANY_VALUE_WRAP
#define ACTIVE_SERIALISE_ANY_VALUE_WRAP

#include "Active/Serialise/Item/Item.h"
#include "Active/Setting/Values/NullValue.h"

#include <functional>

namespace active::serialise {
	
	/*!
		Interface for a lightweight wrapper for a serialisable value of a potentially unspecified type
	
		This is primarily intended to support classes like XMLValueSetting, which may contain an array of values of different types.
		On export, the type of value is established by each outgoing value, but on import the value type may not be known until it is read,
		e.g. a JSON parser may detect text/number/bool/null. XML does not have an explicit format for value types, so this will need to be
		post-processed by the receiving package (if it has explicit type specifications)
	*/
	class AnyValueWrap : public Item {
	public:
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		AnyValueWrap() : Item(), m_value(m_placeholder) {}
		/*!
			Constructor
			@param val The item value
		*/
			//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
		AnyValueWrap(const setting::Value& val) : Item(), m_value(const_cast<setting::Value&>(val)) {}
		
		// MARK: - Functions (const)

		/*!
			Write the item to a string
			@param dest The string to write the data to
			@return True if the data was successfully written
		*/
		bool write(utility::String& dest) const override;
		
		// MARK: - Functions (mutating)

		/*!
			Release the incoming value held by this wrapper
			@return The incoming value (nullptr if none received)
		*/
		setting::Value::Unique releaseIncoming() { return std::move(m_incoming); }
		/*!
			Read the item from a string
			@param source The string to read
			@return True if the data was successfully read
		*/
		bool read(const utility::String& source) override;
		/*!
			Read the cargo data from the specified setting
			@param source The setting to read
			@return True if the data was successfully read
		*/
		bool readSetting(const setting::Value& source) override;
		/*!
			Set to the default package content
		*/
		void setDefault() override;
		/*!
			Get the serialisation type for the item value
			@return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		*/
		std::optional<Cargo::Type> type() const override;
		
	private:
		/*!
			Get the target value for reading/writing
			@return The target value
		*/
		const setting::Value& getTargetValue() const;

			///A concrete value, primarily where the sender has specified an outgoing value (can be used for incoming if the type is preknown)
		std::reference_wrapper<setting::Value> m_value;
			///A null placeholder for cases where a concrete value is no specified
		setting::NullValue m_placeholder;
			///An optional incoming value (captured if no concrete value is specified)
		setting::Value::Unique m_incoming;
	};
	
}

#endif	//ACTIVE_SERIALISE_ANY_VALUE_WRAP
