/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/ValueSetting.h"

#include "Active/Setting/Values/BoolValue.h"
#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Setting/Values/GuidValue.h"
#include "Active/Setting/Values/Int32Value.h"
#include "Active/Setting/Values/Int64Value.h"
#include "Active/Setting/Values/StringValue.h"
#include "Active/Setting/Values/TimeValue.h"
#include "Active/Setting/Values/UInt32Value.h"

using namespace active::setting;
using namespace active::utility;

/*--------------------------------------------------------------------
	Constructor
 
	value: A value to populate into the setting
	rows: The number of rows of this value
	cols: The number of columns of this value
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(const Value& value, size_t rows, size_t cols) {
	m_columns = cols;
	for (auto index = rows * cols; index--; )
		emplace_back(value);
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	value: A value to populate into the setting
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(Value&& value, utility::NameID::Option nameID) : setting::Setting(nameID) {
	emplace_back(value);
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Copy constructor
 
	source: The object to copy
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(ValueSetting&& source) noexcept : Setting(source), base(source) {}


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(bool val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(BoolValue{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(int32_t val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(Int32Value{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(uint32_t val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(UInt32Value{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(int64_t val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(Int64Value{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(double val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(DoubleValue{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(const Guid& val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(GuidValue{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(const String& val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(StringValue{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	val: The value
	nameID: The setting identifier
  --------------------------------------------------------------------*/
ValueSetting::ValueSetting(const Time& val, utility::NameID::Option nameID) : setting::Setting{nameID}	{
	emplace_back(TimeValue{val});
} //ValueSetting::ValueSetting


/*--------------------------------------------------------------------
	Equality operator
 
	ref: The value to compare
 
	return: True if the values are identical
  --------------------------------------------------------------------*/
bool ValueSetting::operator==(const Setting& ref) const {
	if (const auto* refValues = dynamic_cast<const ValueSetting*>(&ref); (refValues != nullptr)) {
		if (refValues == this)
			return true;
		if (size() != refValues->size())
			return false;
		for (auto index = size(); index--; )
			if ((*this)[index] != (*refValues)[index])
				return false;
		return true;
	}
	return false;
} //ValueSetting::operator==


/*--------------------------------------------------------------------
	Less-than operator
 
	ref: The value to compare
 
	return: True if this is less than ref
  --------------------------------------------------------------------*/
bool ValueSetting::operator<(const Setting& ref) const {
	if (const auto* refValues = dynamic_cast<const ValueSetting*>(&ref); (refValues != nullptr)) {
		if (refValues == this)
			return true;
		if (size() != refValues->size())
			return false;
		for (auto index = size(); index--; )
			if ((*this)[index] < (*refValues)[index])
				return false;
		return true;
	}
	return false;
} //ValueSetting::operator<


/*--------------------------------------------------------------------
	Assignment operator
 
	source: The object to copy
 
	return: A reference to this
  --------------------------------------------------------------------*/
Setting& ValueSetting::operator=(const Setting& source) {
	if (const auto* sourceValues = dynamic_cast<const ValueSetting*>(&source); (sourceValues != nullptr) && (sourceValues != this)) {
		base::operator=(*sourceValues);
		m_columns = sourceValues->m_columns;
	} else
		clear();	//If an unknown setting is assigned, existing content is removed to reflect that nothing is copied
	return *this;
} //ValueSetting::operator=


/*--------------------------------------------------------------------
	Assign a default to the setting
  --------------------------------------------------------------------*/
void ValueSetting::setDefault() {
	for (auto& item : *this)
		if (item)
			item->setDefault();
} //ValueSetting::setDefault
