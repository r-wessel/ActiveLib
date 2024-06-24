/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/XML/Package/Wrapper/XMLValueSetting.h"

#include "Active/Serialise/Item/Wrapper/AnyValueWrap.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

#include <array>

using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::setting;
using namespace active::utility;

namespace {
		
		//Serialisation field IDs
	static std::array fieldID = {
		Identity{"tag"},
		Identity{"id"},
		Identity{"cols"},
		Identity{"type"},
		Identity{"class"},
		Identity{"data"},
	};

}  // namespace

/*--------------------------------------------------------------------
	Constructor
 
	setting The setting to wrap for (de)serialisation
	isTyped: True if the value is serialised as a specific type
	className: Optional class name for the value container
  --------------------------------------------------------------------*/
XMLValueSetting::XMLValueSetting(setting::ValueSetting& setting, bool isTyped, std::optional<utility::String> className) : base(setting) {
	if (setting.identity) {
		m_name = (*setting.identity).name;
		if ((*setting.identity).id)
			m_id = (*setting.identity).id;
	}
	m_columns = static_cast<int32_t>(setting.getColumnSize());
	m_isTyped = isTyped;
	if (m_isTyped && !get().empty())
		m_typeName = Value::nameForType(get().front()->getType());	//NB: We currently assume multiple values are a homogenous type
	if (className)
		m_className = *className;
} //XMLValueSetting::XMLValueSetting


/*--------------------------------------------------------------------
	Constructor
 
	setting The setting to wrap for (de)serialisation
	customTag A custom tag (overrides the default tag)
	isTyped: True if the value is serialised as a specific type
	className: Optional class name for the value container
  --------------------------------------------------------------------*/
XMLValueSetting::XMLValueSetting(setting::ValueSetting& setting, const utility::String& customTag,
								 bool isTyped, std::optional<utility::String> className) : XMLValueSetting{setting} {
	m_tag = customTag;
	m_columns = static_cast<int32_t>(setting.getColumnSize());
	m_isTyped = isTyped;
	if (m_isTyped && !get().empty())
		m_typeName = Value::nameForType(get().front()->getType());	//NB: We currently assume multiple values are a homogenous type
	if (className)
		m_className = *className;
} //XMLValueSetting::XMLValueSetting


/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool XMLValueSetting::fillInventory(Inventory& inventory) const {
	using enum Entry::Type;
	inventory.merge(Inventory{
		{
			{ fieldID[name], name, attribute, !m_name.empty() },
			{ fieldID[valGuid], valGuid, attribute, !m_id.empty() },
			{ fieldID[columns], columns, attribute, (m_columns > 1) },
			{ fieldID[FieldIndex::type], FieldIndex::type, attribute, m_isTyped && !m_typeName.empty() && (m_typeName != Value::nameForType(Value::stringType)) },
			{ fieldID[className], className, attribute, !m_className.empty() },
			{ fieldID[value], value, static_cast<uint32_t>(get().size()), std::nullopt, (get().size() > 0) },
		}
	}.withType(&typeid(XMLValueSetting)));
	return true;
} //XMLValueSetting::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique XMLValueSetting::getCargo(const Inventory::Item& item) const {
	if (item.ownerType != &typeid(XMLValueSetting))
		return nullptr;
	switch (item.index) {
		case FieldIndex::name:
			return std::make_unique<StringWrap>(m_name);
		case FieldIndex::valGuid:
			return std::make_unique<StringWrap>(m_id);
		case FieldIndex::columns:
			return std::make_unique<Int32Wrap>(m_columns);
		case FieldIndex::type:
			return std::make_unique<StringWrap>(m_typeName);
		case FieldIndex::className:
			return std::make_unique<StringWrap>(m_className);
		case FieldIndex::value:
			if (item.available < static_cast<uint32_t>(get().size()))
				return std::make_unique<AnyValueWrap>(*(get()[item.available]));
			return std::make_unique<AnyValueWrap>();
		default:
			return nullptr;	//Requested an unknown index
	}
} //XMLValueSetting::getCargo


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void XMLValueSetting::setDefault() {
	base::get().clear();	//The default state is an empty setting
	m_columns = 0;
	m_name.clear();
	m_typeName.clear();
	m_className.clear();
	m_id.clear();
} //XMLValueSetting::setDefault


/*--------------------------------------------------------------------
	Validate the cargo data
 
	return: True if the data has been validated
  --------------------------------------------------------------------*/
bool XMLValueSetting::validate() {
	Guid settingGuid{m_id};
	if (!m_name.empty() || settingGuid) {
		base::get().identity = std::make_optional<NameID>(m_name, settingGuid);
	} else
		base::get().identity = std::nullopt;
	return true;
} //XMLValueSetting::validate


/*--------------------------------------------------------------------
	Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
 
	cargo: The cargo to insert
	item: The inventory item linked with the cargo
 
	return: True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
  --------------------------------------------------------------------*/
bool XMLValueSetting::insert(Cargo::Unique&& cargo, const Inventory::Item& item) {
	if (item.ownerType != &typeid(XMLValueSetting))
		return true;
	switch (item.index) {
		case FieldIndex::value:
			if (auto* anyValue = dynamic_cast<AnyValueWrap*>(cargo.get()); anyValue != nullptr) {
				if (auto newValue = anyValue->releaseIncoming(); newValue)
					base::get().emplace_back(newValue);
			}
			break;
		default:
			break;
	}
	return true;
} //XMLValueSetting::insert
