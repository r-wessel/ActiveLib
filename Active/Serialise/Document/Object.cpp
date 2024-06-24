/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Document/Object.h"

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/Package/PackageWrap.h"
#include "Active/Serialise/XML/Package/Wrapper/XMLValueSetting.h"

#include <array>

using namespace active::serialise;
using namespace active::serialise::doc;
using namespace active::setting;
using namespace active::utility;

namespace {

		///Serialisation fields
	enum FieldIndex {
		typeID,
		tagID,
		val,
		obj,
	};

		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"class"},
		Identity{"tag"},
		Identity{"val"},
		Identity{"obj"},
	};

}  // namespace

/*--------------------------------------------------------------------
	Constructor
 
	type: The document object type - used to reconstruct the original object. NB: Can use type_info.name, but isn't consistent
	objTag: An optional serialisation tag (differentiating the object role when there are many of the same type for different purposes)
  --------------------------------------------------------------------*/
Object::Object(const String& type, String::Option objTag) {
	docType = type;
	if (objTag)
		tag = *objTag;
} //Object::Object


/*--------------------------------------------------------------------
	Get an object by tag
 
	tag: The required object tag
 
	return: The requested object (nullptr if not found)
  --------------------------------------------------------------------*/
const Object* Object::object(const String& tag) const {
	if (const auto& match = std::find_if(objects.begin(), objects.end(), [&](auto& object) { return object.tag == tag; }); match != objects.end())
		return &(*match);
	return nullptr;
} //Object::package


/*--------------------------------------------------------------------
	Get an object value by name
 
	name: The value name
 
	return: The requested value (nullopt on failure)
  --------------------------------------------------------------------*/
const ValueSetting::Option Object::value(const String& name) const {
	if (const auto& match = std::find_if(values.begin(), values.end(), [&](auto& value) { return value.name() == name; }); match != values.end())
		return *match;
	return std::nullopt;
} //Object::value


/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool Object::fillInventory(Inventory& inventory) const {
	using enum Entry::Type;
	inventory.merge(Inventory{
		{
			{ fieldID[typeID], typeID, attribute },
			{ fieldID[tagID], tagID, attribute, !tag.empty() },
			{ fieldID[val], val, static_cast<uint32_t>(values.size()), std::nullopt, values.size() > 0 },
			{ fieldID[obj], obj, static_cast<uint32_t>(objects.size()), std::nullopt, objects.size() > 0 },
		},
	}.withType(&typeid(Object)));
	return true;
} //Object::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
active::serialise::Cargo::Unique Object::getCargo(const active::serialise::Inventory::Item& item) const {
	if (item.ownerType != &typeid(Object))
		return nullptr;
	using namespace active::serialise;
	switch (item.index) {
		case typeID:
			return std::make_unique<StringWrap>(docType);
		case tagID:
			return std::make_unique<StringWrap>(tag);
		case val:
			if (item.available < values.size())
				return std::make_unique<xml::XMLValueSetting>(values[item.available]);
			return std::make_unique<xml::XMLValueSetting>(m_incoming);
		case obj:
			if (item.available < objects.size())
				return std::make_unique<PackageWrap>(objects[item.available]);
			return std::make_unique<Object>(String());
		default:
			return nullptr;	//Requested an unknown index
	}
} //Object::getCargo


/*--------------------------------------------------------------------
	Define the handler to reconstruct objects
 
	handler: The handler to reconstruct objects
 
	return: A reference to this
  --------------------------------------------------------------------*/
const Object& Object::usingHandler(std::shared_ptr<Handler> handler) const {
	m_handler = handler;
	for (const auto& package : objects)
		package.usingHandler(handler);
	return *this;
} //Object::usingHandler


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void Object::setDefault() {
	docType.clear();
	tag.clear();
	values.clear();
	objects.clear();
} //Object::setDefault


/*--------------------------------------------------------------------
	Validate the cargo data
 
	return: True if the data has been validated
  --------------------------------------------------------------------*/
bool Object::validate() {
	return !docType.empty();	//Can't construct an object without a type identifier
} //Object::validate


/*--------------------------------------------------------------------
	Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
 
	cargo: The cargo to insert
	item: The inventory item linked with the cargo
 
	return: True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
  --------------------------------------------------------------------*/
bool Object::insert(Cargo::Unique&& cargo, const active::serialise::Inventory::Item& item) {
	if (item.ownerType != &typeid(Object))
		return true;
	switch (item.index) {
		case FieldIndex::val:
			if (auto* value = dynamic_cast<xml::XMLValueSetting*>(cargo.get()); value != nullptr)
				values.emplace_back(*value);
			break;
		case FieldIndex::obj:
			if (auto* package = dynamic_cast<Object*>(cargo.get()); package != nullptr)
				objects.emplace_back(std::move(*package));
			break;
		default:
			break;
	}
	return true;
} //Object::insert
