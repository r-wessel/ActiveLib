/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Inventory/Inventory.h"

#include <algorithm>

using namespace active::serialise;
using namespace active::utility;

/*--------------------------------------------------------------------
	Count the number of attributes in the inventory
 
	isRequiredOnly: True if only required attributes should be counted
	
	return: The number of attributes
  --------------------------------------------------------------------*/
Inventory::size_type Inventory::attributeSize(bool isRequiredOnly) const {
	return std::count_if(begin(), end(), [&](const auto& item){
		return (isRequiredOnly && !item.required) ? false : item.isAttribute();
	});
} //Inventory::attributeSize


/*--------------------------------------------------------------------
	Count the number of required entries in the inventory
 
	return: The number of required entries
  --------------------------------------------------------------------*/
Inventory::size_type Inventory::countRequired() const {
	return std::count_if(base::begin(), base::end(), [](const auto& i) { return i.required; });
} //Inventory::countRequired


/*--------------------------------------------------------------------
	Get the inventory handling sequence, e.g. to package in the correct order for transport
 
	return: The inventory handling sequence (ordered by entry.index)
  --------------------------------------------------------------------*/
Inventory::Sequence Inventory::sequence() const {
	Inventory::Sequence sequenced;
	for (auto i = begin(); i != end(); ++i)
		sequenced.push_back(std::make_pair(i->index, i));
		//Order attributes first, then ordered by entry.index
	std::stable_sort(sequenced.begin(), sequenced.end(), [](const auto& left, const auto& right) {
		if (left.second->isAttribute() == right.second->isAttribute())
			return false;
		return left.second->isAttribute();	//Attributes come first
	});
	return sequenced;
} //Inventory::sequence


/*--------------------------------------------------------------------
	Set the owner type for the inventory entries
 
	ownerType: The owner type (typically used to link entries to objects within a hierarchical structure)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Inventory& Inventory::withType(const std::type_info* ownerType) {
	std::for_each(begin(), end(), [&](auto& i) { i.ownerType = ownerType; });
	return *this;
} //Inventory::withType


/*--------------------------------------------------------------------
	Merge another inventory with this
 
	inventory: The inventory to merge
 
	return: A reference to this
  --------------------------------------------------------------------*/
Inventory& Inventory::merge(const Inventory& inventory) {
	insert(end(), inventory.begin(), inventory.end());
		//If these are the first entries in the inventory, we also set any metadata it contains
	m_isFilled = true;
	return *this;
} //Inventory::merge


/*--------------------------------------------------------------------
	Merge an entry with this
 
	entry: The entry to merge
 
	return: A reference to this
  --------------------------------------------------------------------*/
Inventory& Inventory::merge(const Entry& entry) {
	insert(end(), entry);
	return *this;
} //Inventory::merge


/*--------------------------------------------------------------------
	Register an incoming item in the inventory
 
	identity: The item identity
 
 	return: The registered item (nullopt == not found)
  --------------------------------------------------------------------*/
Inventory::iterator Inventory::registerIncoming(const Identity& identity) {
		//Look for an existing inventory item
	if (auto item = findEntry(identity); item != base::end())
		return item;
	if (isEveryItemAccepted) {
		
	}
	return end();
} //Inventory::registerIncoming


/*--------------------------------------------------------------------
	Reset the availability per entry to zero (typically just prior to importing)
  --------------------------------------------------------------------*/
void Inventory::resetAvailable() {
	std::for_each(begin(), end(), [&](auto& i) { i.available = 0; });
} //Inventory::resetAvailable


/*--------------------------------------------------------------------
	Find an entry by identity and optionally owner type
 
	identity: The entry identity
	owner: The entry owner type
 
	return: An iterator at the requested entry (end() on failure)
  --------------------------------------------------------------------*/
Inventory::iterator Inventory::findEntry(const Identity& identity, const std::type_info* owner) {
	return std::find_if(begin(), end(), [&](auto& i) {
		return ((i.identity() == identity) && ((owner == nullptr) || (i.ownerType == owner)));
	});
} //Inventory::findEntry


/*--------------------------------------------------------------------
	Find a specified field by index and optionally owner type
 
	ind: The field index
	owner: The field owner type
 
	return: An iterator at the requested entry (end() on failure)
  --------------------------------------------------------------------*/
Inventory::iterator Inventory::findEntry(int16_t ind, const std::type_info* owner) {
	return std::find_if(begin(), end(), [&](auto& i) {
		return ((i.index == ind) && ((owner == nullptr) || (i.ownerType == owner)));
	});
} //Inventory::findEntry


/*--------------------------------------------------------------------
	Set the required state of a specified field (for export)
 
	mustHave: True if the field is required for export
	ind: The field index
	owner: The field owner type
 
	return: True if the field was found
  --------------------------------------------------------------------*/
bool Inventory::setRequired(bool mustHave, int16_t ind, const std::type_info* owner) {
	if (auto entry = findEntry(ind, owner); entry != end()) {
		entry->required = mustHave;
		return true;
	}
	return false;
} //Inventory::setRequired


/*--------------------------------------------------------------------
	Mark all the entries as 'required'
  --------------------------------------------------------------------*/
void Inventory::setAllRequired() {
	std::for_each(base::begin(), base::end(), [](auto& i){ i.required = true; });
} //Inventory::setAllRequired
