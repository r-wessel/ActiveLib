/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Inventory/Entry.h"

#include "Active/Serialise/Inventory/Inventory.h"

using namespace active::serialise;

/*--------------------------------------------------------------------
	Get any management applied to this item (nullptr = unmanaged)
  --------------------------------------------------------------------*/
Management* Entry::management() const {
	return inventory == nullptr ? nullptr : inventory->management();
} //Entry::management


/*--------------------------------------------------------------------
	Determine if the entry is managed
 
	return: True if the entry is managed
  --------------------------------------------------------------------*/
bool Entry::isManaged() const {
	return inventory == nullptr ? false : inventory->isManaged();
} //Entry::isManaged
