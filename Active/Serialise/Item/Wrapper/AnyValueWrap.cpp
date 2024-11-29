/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Item/Wrapper/AnyValueWrap.h"

using namespace active::serialise;
using namespace active::utility;
using namespace active::setting;

/*--------------------------------------------------------------------
	Write the item to a string
 
	dest: The string to write the data to
 
	return: True if the data was successfully written
  --------------------------------------------------------------------*/
bool AnyValueWrap::write(utility::String& dest) const {
	dest = getTargetValue();
	return true;
} //AnyValueWrap::read


/*--------------------------------------------------------------------
	Read the item from a string
 
	source: The string to read
 
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool AnyValueWrap::read(const utility::String& source) {
		//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
	const_cast<Value&>(getTargetValue()) = source;
	return true;
} //AnyValueWrap::read


/*--------------------------------------------------------------------
	Read the cargo data from the specified setting
 
	source: The setting to read
 
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool AnyValueWrap::readSetting(const Value& source) {
	m_incoming = clone(source);	//If we receive an incoming value type, we use this in preference to any specified concrete type
	read(source);	//But we still read the value as usual in case this is preferred
	return true;
} //AnyValueWrap::readSetting


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void AnyValueWrap::setDefault() {
		//NB: Value is only mutated within import processes, in which case the object must be mutable (i.e. const discard is safe)
	const_cast<Value&>(getTargetValue()).setDefault();
} //AnyValueWrap::setDefault


/*--------------------------------------------------------------------
	Get the serialisation type for the item value
 
	return: The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
  --------------------------------------------------------------------*/
std::optional<Cargo::Type> AnyValueWrap::type() const  {
	using enum Cargo::Type;
	using enum Value::Type;
	switch (getTargetValue().getType()) {
		case Value::Type::null:
			return std::nullopt;
		case Value::Type::boolType:
			return boolean;
		case Value::Type::idType: case Value::Type::stringType: case Value::Type::timeType:
			return Cargo::Type::text;
		default:
			return Cargo::Type::number;
	}
} //AnyValueWrap::type


/*--------------------------------------------------------------------
	Get the target value for reading/writing
 
	return: The target value
  --------------------------------------------------------------------*/
const Value& AnyValueWrap::getTargetValue() const {
	if (m_incoming)
		return *m_incoming;
	return m_value.get();
} //AnyValueWrap::getTargetValue
