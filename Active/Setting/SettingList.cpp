/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include <algorithm>

#include "Active/Setting/SettingList.h"

using namespace active::setting;
using namespace active::utility;

/*--------------------------------------------------------------------
	Constructor
 
	settings: Settings to populate into the list
  --------------------------------------------------------------------*/
SettingList::SettingList(const std::initializer_list<ValueSetting>& settings) {
	for (const auto& setting : settings) {
		emplace_back(setting);
	}
} //SettingList::SettingList


/*--------------------------------------------------------------------
	Assignment operator
 
	source: The object to copy
 
	return: A reference to this
  --------------------------------------------------------------------*/
SettingList& SettingList::operator=(const SettingList& source) {
	if (this != &source) {
		base::operator=(source);
	}
	return *this;
} //SettingList::operator=


/*--------------------------------------------------------------------
	Equality operator
 
	ref: The value to compare
 
	return: True if the values are identical
  --------------------------------------------------------------------*/
bool SettingList::operator==(const SettingList& ref) const {
	if (size() != ref.size())
		return false;
	auto nullCount = 0;
	for (auto& setting : *this) {
		if (setting == nullptr) {
			++nullCount;
			continue;
		}
		for (auto& refSetting : ref) {
			if (*setting == *refSetting)
				break;
			
		}
	}
	return true;
} //SettingList::operator==


/*--------------------------------------------------------------------
	Find a setting with a specified ID
 
	nameID: The ID to search for
 
	return: An iterator pointing to the first matching entry (end on failure)
  --------------------------------------------------------------------*/
SettingList::const_iterator SettingList::find(const NameID& nameID) const {
	return const_cast<SettingList*>(this)->find(nameID);
} //SettingList::find


/*--------------------------------------------------------------------
	Find a match for a specified setting
 
	ref: The setting to match (same ID and content)
 
	return: An iterator pointing to the first matching entry (end on failure)
  --------------------------------------------------------------------*/
SettingList::const_iterator SettingList::find(const Setting& ref) const {
	return const_cast<SettingList*>(this)->find(ref);
} //SettingList::find


/*--------------------------------------------------------------------
	Find a setting with a specified ID
 
	nameID: The ID to search for
 
	return: An iterator pointing to the first matching entry (end on failure)
  --------------------------------------------------------------------*/
SettingList::iterator SettingList::find(const NameID& nameID) {
	return std::find_if(begin(), end(), [&](const auto& setting){ return setting->identity == nameID; });
} //SettingList::find


/*--------------------------------------------------------------------
	Find a match for a specified setting
 
	setting: The setting to match (same ID and content)
 
	return: An iterator pointing to the first matching entry (end on failure)
  --------------------------------------------------------------------*/
SettingList::iterator SettingList::find(const Setting& ref) {
	return std::find_if(begin(), end(), [&](const auto& setting){ return ((setting->identity == ref.identity) && (ref == *setting)); });
} //SettingList::find
