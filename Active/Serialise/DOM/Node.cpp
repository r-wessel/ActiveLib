/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/DOM/Node.h"

#include "Active/Serialise/Null.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Setting/Values/BoolValue.h"
#include "Active/Setting/Values/Int64Value.h"
#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Setting/Values/NullValue.h"
#include "Active/Setting/Values/StringValue.h"

using namespace active::serialise;
using namespace active::serialise::dom;
using namespace active::setting;
using namespace active::utility;

namespace {

	/*!
	 Allocate a default node based on the identified entry/value type (defaulting to string value as a catch-all)
	 @param identity The specified node identity
	 @return A default node reflecting the specified identity
	 */
	Node makeNode(const Identity& identity) {
		using enum active::setting::Value::Type;
		if (identity.entryRole == Identity::Role::array)
			return Array{};
		else {
				//Allocate a default value based on the identified value type (defaulting to string as a catch-all)
			switch (identity.valueType.value_or(stringType)) {
				case boolType:
					return active::serialise::dom::Value{false};
				case intType:
					return active::serialise::dom::Value{0};
				case floatType:
						return active::serialise::dom::Value{0.0};
				default:
					return active::serialise::dom::Value{String{}};
			}
		}
	} //makeNode

	
	/*!
	 Wrap a node for transport
	 @param node The node to be wrapped
	 @return Cargo for transport referencing the node
	 */
	active::serialise::Cargo::Unique wrapNode(Node& node) {
		return std::make_unique<PackageWrap>(node);
	}
	
}

/*--------------------------------------------------------------------
	Constructor
 
	nodes: Nodes to populate an array
  --------------------------------------------------------------------*/
Node::Node(const std::initializer_list<Node>& nodes) {
	base::operator=(Array{});
	for (const auto& node : nodes)
		push_back(node);
} //Node::Node


/*--------------------------------------------------------------------
	Get the value setting (allowing for anonymous conversion to a variety of value types)
 
	return: The value setting
  --------------------------------------------------------------------*/
ValueSetting dom::Value::setting() const {
	using enum Index;
	switch (index()) {
		case boolType:
			return ValueSetting(get<bool>(*this));
		case intType:
			return ValueSetting(get<int64_t>(*this));
		case floatType:
			return ValueSetting(get<double>(*this));
		case stringType:
			return ValueSetting(get<String>(*this));
		default:
			break;
	}
	return ValueSetting{NullValue{}};
} //Value::setting


/*--------------------------------------------------------------------
	Determine if the node is empty (undefined, containing a default/undefined value or an empty object/array)
 
	return: True if the node is empty
  --------------------------------------------------------------------*/
bool Node::empty() const {
	switch (index()) {
		case Index::value:
			using enum Value::Index;
			switch (value().index()) {
				case boolType:
					return !get<bool>(value());
				case intType:
					return get<int64_t>(value()) == 0;
				case floatType:
					return math::isZero(get<double>(value()));
				case stringType:
					return get<String>(value()).empty();
				default:
					break;
			}
			return true;
		case Index::object:
			return object().empty();
		case Index::array:
			return array().empty();
		default:
			break;
	}
	return true;
} //Node::empty


/*--------------------------------------------------------------------
	Get the index of a named item in the node
 
	name: The name to search for
 
	return: The type index of the named item (nullopt if not found)
  --------------------------------------------------------------------*/
std::optional<Node::Index> Node::index(const utility::String& name) const {
	if (!isObject())
		return std::nullopt;
	if (auto iter = object().find(name); iter != object().end())
		return iter->second.index();
	return std::nullopt;
} //Node::index


/*--------------------------------------------------------------------
	Write the item to a string
 
	dest: The string to write the data to
 
	return: True if the data was successfully written
  --------------------------------------------------------------------*/
bool Node::write(utility::String& dest) const {
	using enum Value::Index;
	switch (value().index()) {
		case boolType:
			return BoolWrap(get<bool>(value())).write(dest);
		case intType:
			return Int64Wrap(get<int64_t>(value())).write(dest);
		case floatType:
			return DoubleWrap(get<double>(value())).write(dest);
		case stringType:
			dest = get<utility::String>(value());
			break;
		default:
			dest.clear();
			break;
	};
	return true;
} //Node::write


/*--------------------------------------------------------------------
	Get the serialisation type for the cargo value
 
	return: The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
  --------------------------------------------------------------------*/
std::optional<Cargo::Type> Node::type() const {
	using enum Cargo::Type;
	if (index() == Index::value) {
		using enum Value::Index;
		switch (value().index()) {
			case boolType:
				return boolean;
			case intType: case floatType:
				return number;
			case stringType:
				return text;
			default:
				break;
		}
	}
	return package;
} //Node::type


/*--------------------------------------------------------------------
	Get an object value setting by name
 
	name: The value name
 
	return: The requested value setting (nullopt on failure)
  --------------------------------------------------------------------*/
ValueSetting::Option Node::setting(const String& name) const {
	if (!isObject())
		return std::nullopt;
	if (auto iter = object().find(name); (iter != object().end()) && (iter->second.index() == Index::value)) {
		using enum Value::Index;
		switch (iter->second.value().index()) {
			case boolType:
				return ValueSetting(get<bool>(iter->second.value()), name);
			case intType:
				return ValueSetting(get<int64_t>(iter->second.value()), name);
			case floatType:
				return ValueSetting(get<double>(iter->second.value()), name);
			case stringType:
				return ValueSetting(get<String>(iter->second.value()), name);
			default:
				break;
		}
	}
	return std::nullopt;
} //Node::setting


/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool Node::fillInventory(Inventory& inventory) const {
	using enum Identity::Role;
	inventory.isEveryItemAccepted = true;
	switch (index()) {
		case Index::object: {
			int16_t index = 0;
			for (auto node = object().begin(); node != object().end(); ++node)
				inventory.merge({ node->first, index++, element });
			break;
		}
		case Index::array:
			inventory.merge({ this->array().itemTag, 0, this->array().size(), std::nullopt });
			break;
		default:
			break;
	}
	return true;
} //Node::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
active::serialise::Cargo::Unique Node::getCargo(const active::serialise::Inventory::Item& item) const {
	switch (index()) {
		case Index::value:
			return wrapNode(const_cast<Node&>(*this));
		case Index::object: {
			auto iter = object().find(item.identity().name);
			return iter == object().end() ? nullptr : wrapNode(const_cast<Node&>(iter->second));
		}
		case Index::array: {
			auto sourceArray = const_cast<Array*>(&array());
			while (item.available >= sourceArray->size())
				sourceArray->push_back(makeNode(item.identity()));
			return wrapNode((*sourceArray)[item.available]);
		}
		default:
			break;
	}
	return nullptr;
} //Node::getCargo


/*--------------------------------------------------------------------
	Read the item from a string
 
	source: The string to read
 
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool Node::read(const utility::String& source) {
	base::operator=(Value{source});
	return true;
} //Node::read


/*--------------------------------------------------------------------
	Read the cargo data from the specified setting
 
	source: The setting to read
 
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool Node::readSetting(const setting::Value& source) {
	switch (source.getType()) {
		case setting::Value::boolType:
			base::operator=(Value{source.operator bool()});
			break;
		case setting::Value::intType:
			base::operator=(Value{source.operator int64_t()});
			break;
		case setting::Value::floatType:
			base::operator=(Value{source.operator double()});
			break;
		default:
			base::operator=(Value{source.operator utility::String()});
	};
	return true;
} //Node::readSetting


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void Node::setDefault() {
	base::operator=(std::monostate{});
} //Node::setDefault


/*--------------------------------------------------------------------
	Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
 
	cargo: The cargo to insert
	item: The inventory item linked with the cargo
 
	return: True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
  --------------------------------------------------------------------*/
bool Node::insert(Cargo::Unique&& cargo, const active::serialise::Inventory::Item& item) {
	return true;
} //Node::insert


/*--------------------------------------------------------------------
	Allocate inventory for new (incoming) cargo
 
	inventory: The inventory to extend
	identity: The cargo identity
	enclosing: The enclosing cargo identity
 
	return: An iterator pointing to the allocated item (returns end() if inventory cannot be allocated)
  --------------------------------------------------------------------*/
Inventory::iterator Node::allocate(Inventory& inventory, const Identity& identity, const Identity& enclosing) {
	switch (index()) {
			//If the node is undefined, we're at the root level - apply the identity to this node
		case Index::undefined:
			if (enclosing.entryRole == Identity::Role::array) {
				base::operator=(Array{}.withItemTag(identity.name));	//Allocate an empty array
				return inventory.merge({ identity.name, static_cast<int16_t>(inventory.size()), 0, std::nullopt });
			}
			base::operator=(Object{});
			break;
		case Index::array: {
				//Assume this node should actually be an object - first capture the existing content
			auto child = *this;
			base::operator=(Object{});
				//We can only retain the existing node in an object if it's named
			if (!child.array().itemTag.empty()) {
				auto name = child.array().itemTag;
				object()[name] = child;
				object()[name].array().itemTag.clear();
			}
			break;
		}
		case Index::value:
			base::operator=(Object{});	///If the node is a value, assume that it should transform to an object
			break;
		default:
			break;
	}
	object()[identity.name] = makeNode(identity);
	return inventory.merge({ identity.name, static_cast<int16_t>(inventory.size()), 0 });
} //Node::allocate


/*--------------------------------------------------------------------
	Allocate an existing inventory item as an array
 
	inventory: The parent inventory
	item: The inventory item to reallocate as an array
 
	return: An iterator pointing to the reallocated item (returns end() if the item cannot be reallocated as an array)
  --------------------------------------------------------------------*/
Inventory::iterator Node::allocateArray(Inventory& inventory, Inventory::iterator item) {
		//Only objects have named members and can contain an array
	if (index() != Index::object)
		return inventory.end();
		//Only a child value can be transformed to an array
	auto child = object().find(item->identity().name);
	if ((child == object().end()) || (child->second.index() != Index::value))
		return inventory.end();
		//Cache the value already in the child node
	Node cached(Value{child->second.value()});
		//If there's only one item in the object, assume for the moment that the entire node is an array
	if (object().size() == 1) {
			//Reallocate this node an an array
		base::operator=(Array{}.withItemTag(item->identity().name));
			//Push the existing value into the new array
		array().push_back(cached);
	} else {
			//Reallocate the node as an array
		child->second.base::operator=(Array{}.withItemTag(item->identity().name));
			//Push the existing value into the new array
		child->second.push_back(cached);
	}
		//Clear the maximum items allowed in this node (i.e. no limit)
	item->setMaximum();
	return item;
} //Node::allocateArray
