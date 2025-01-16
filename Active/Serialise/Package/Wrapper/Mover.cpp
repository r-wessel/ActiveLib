/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Package/Wrapper/Mover.h"

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

#include <array>

using namespace active::serialise;
using namespace active::utility;

using enum Mover::TransportPhase;

namespace {
	
		///Serialisation fields
	enum FieldIndex {
		objectTypeID,
	};
	
}

/*--------------------------------------------------------------------
	Constructor (when it will be used for serialisation only on an existing object)
 
	package: A reference to the member variable
	handler: A package handler to tag outgoing packages
  --------------------------------------------------------------------*/
Mover::Mover(const Package& package, Handler::Shared handler) : m_handler{handler}, m_package{const_cast<Package*>(&package)} {
	m_typeName = handler->findTagFor(typeid(package)).value_or(String{});
} //Mover::Mover


/*--------------------------------------------------------------------
	Constructor (when a reference to a unique_ptr member package variable is supplied)
 
	package: A reference to a unique_ptr member variable
	handler: A package handler to tag outgoing packages (nullptr = fixed type, use the wrapper object maker)
  --------------------------------------------------------------------*/
Mover::Mover(PackageUniqueWrap&& package, Handler::Shared handler) : m_handler{handler}, m_unique{package} {
	m_package = m_unique->get();
	if ((m_package != nullptr) && m_handler)
		m_typeName = handler->findTagFor(typeid(*m_package)).value_or(String{});
} //Mover::Mover


/*--------------------------------------------------------------------
	Constructor (primarily for a list of polymorphic objects)
 
	handler: A package handler to reconstruct incoming packages
  --------------------------------------------------------------------*/
Mover::Mover(Handler::Shared handler) : m_handler{handler} {
} //Mover::Mover


/*--------------------------------------------------------------------
	Fill an inventory with the package items
 
	inventory: The inventory to receive the package items
 
	return: True if the package has added items to the inventory
  --------------------------------------------------------------------*/
bool Mover::fillInventory(Inventory& inventory) const {
	using enum Identity::Role;
	if (m_handler && ((m_transportPhase == writingEverything) || (m_transportPhase == readingAttributes))) {
		inventory.merge(Inventory{
			{
				{ m_handler->attributeTag(), objectTypeID, attribute },
			},
		}.withType(&typeid(Mover)));
	}
	if (!isNull())
		m_package->fillInventory(inventory);
	return true;
} //Mover::fillInventory


/*--------------------------------------------------------------------
	Get the specified cargo
 
	item: The inventory item to retrieve
 
	return: The requested cargo (nullptr on failure)
  --------------------------------------------------------------------*/
Cargo::Unique Mover::getCargo(const Inventory::Item& item) const {
		//If the item isn't from this wrapper, pass the request to the wrapper for the target subclass
	if (item.ownerType != &typeid(Mover)) {
		if (m_package)
			return m_package->getCargo(item);
		return nullptr;
	}
	switch (item.index) {
		case objectTypeID:
			return std::make_unique<StringWrap>(m_typeName);
	}
	return nullptr;
} //Mover::getCargo


/*--------------------------------------------------------------------
	Set to the default package content
  --------------------------------------------------------------------*/
void Mover::setDefault() {
	m_transportPhase = readingAttributes;
		//If we have a handler, we need to interpret the object type from the deserialised data
	if (m_handler) {
		m_wrapper.reset();	//This will be populated once the type and guid are deserialised
		m_typeName.clear();
		if (m_package != nullptr)
			m_package->setDefault();
	} else if ((m_package == nullptr) && m_unique && m_unique->canMake()) {
			//Otherwise, we must be dealing with a fixed type and can get the wrapper to make an object (if we don't have one)
		if (!m_wrapper)
			m_wrapper = m_unique->make();
		m_package = m_wrapper.get();
	}
} //Mover::setDefault


/*--------------------------------------------------------------------
	Validate the cargo data
 
	return: True if the data has been validated
  --------------------------------------------------------------------*/
bool Mover::validate() {
	if (!m_wrapper)
		return true;
	if (!m_wrapper->validate())
		return false;
	if (m_unique)
		m_unique->set(std::move(m_wrapper));
	m_transportPhase = writingEverything;
	return true;
} //Mover::validate


/*--------------------------------------------------------------------
	Finalise the package attributes (called when isAttributeFirst = true and attributes have been imported)
 
	return: True if the attributes have been successfully finalised (returning false will cause an exception to be thrown)
  --------------------------------------------------------------------*/
bool Mover::finaliseAttributes() {
	if (!m_handler || (m_transportPhase != readingAttributes))
		return true;	//Finalise not applicable
	m_transportPhase = readingElements;
		//Attempt to construct a new package based on the deserialised type name
	m_wrapper.reset(m_handler->reconstruct(m_typeName));
	if (!m_wrapper)
		return false;
	m_package = m_wrapper.get();
	return true;
} //Mover::finaliseAttributes


/*--------------------------------------------------------------------
	Read the cargo data from the specified string
 
	source: The string to read
 
	return: True if the data was successfully read
  --------------------------------------------------------------------*/
bool Mover::read(const utility::String& source) {
	if (source.empty() || isNull())
		return true;
	return m_package->read(source);
} //Mover::read


/*--------------------------------------------------------------------
	Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
 
	cargo: The cargo to insert
	item: The inventory item linked with the cargo
  --------------------------------------------------------------------*/
bool Mover::insert(Cargo::Unique&& cargo, const Inventory::Item& item) {
	return (isNull()) ? true : m_package->insert(std::move(cargo), item);
} //Mover::insert
