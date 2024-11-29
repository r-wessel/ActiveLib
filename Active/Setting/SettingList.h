/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_SETTING_LIST
#define ACTIVE_SETTING_SETTING_LIST

#include "Active/Container/Vector.h"
#include "Active/Setting/ValueSetting.h"

namespace active::setting {

		///A list of settings
	class SettingList : public container::Vector<Setting>, public utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Base storage class
		using base = container::Vector<Setting>;
			///Storage iterator
		using iterator = typename base::iterator;
			///Storage iterator
		using const_iterator = typename base::const_iterator;
			///Shared pointer
		using Shared = std::shared_ptr<SettingList>;
			///Shared pointer
		using Unique = std::unique_ptr<SettingList>;
			///Optional
		using Option = std::optional<SettingList>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		SettingList() {}
		/*!
			Constructor
			@param settings Settings to populate into the list (NB: for value types only using an initialiser list)
		*/
		SettingList(const std::initializer_list<ValueSetting>& settings);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		SettingList(const SettingList& source) : base(source) {}
		/*!
			Move constructor
			@param source The object to move
		*/
		SettingList(SettingList&& source) noexcept : base(std::move(source)) {}

		/*!
			Clone method
			@return A clone of this object
		*/
		virtual SettingList* clonePtr() const { return new SettingList{*this}; }
		
		// MARK: Operators
		
		/*!
			Assignment operator
			@param source The object to copy
		 	@return A reference to this
		*/
		virtual SettingList& operator=(const SettingList& source);
		/*!
			Equality operator
			@param ref The value to compare
		 	@return True if the values are identical
		*/
		virtual bool operator==(const SettingList& ref) const;

		// MARK: - Functions (const)

		/*!
			Determine if the list contains a setting with a specified ID
			@param nameID The ID to search for
			@return True if the list contains a matching entry
		*/
		bool contains(const utility::NameID& nameID) const { return find(nameID) != end(); }
		/*
			Find a match for a specified setting
			@param setting The setting to match (same ID and content)
			@return An iterator pointing to the first matching entry (end on failure)
		*/
		SettingList::const_iterator find(const Setting& setting) const;
		/*!
			Find a setting with a specified ID
			@param nameID The ID to search for
			@return An iterator pointing to the first matching entry (end on failure)
		*/
		const_iterator find(const utility::NameID& nameID) const;
		/*!
			Find a setting with a specified ID and attempt to cast as a specific type
			@param nameID The ID to search for
			@return The requested setting (nullptr if not found)
			@tparam T The expected setting type
		*/
		template<typename T>
		T* find(const utility::NameID& nameID) const;
		/*!
			Find a value setting with a specified ID
			@param nameID The ID to search for
			@return A pointer to the requested value setting (nullptr on failure)
		*/
		ValueSetting* findValue(const utility::NameID& nameID) const { return find<ValueSetting>(nameID); }

		// MARK: Functions (mutating)
		
		/*!
			Find a setting with a specified ID
			@param nameID The ID to search for
			@return An iterator pointing to the first matching entry (end on failure)
		*/
		iterator find(const utility::NameID& nameID);
		/*
			Find a match for a specified setting
			@param setting The setting to match (same ID and content)
			@return An iterator pointing to the first matching entry (end on failure)
		*/
		SettingList::iterator find(const Setting& setting);
	};
	
	
	/*--------------------------------------------------------------------
		Find a setting with a specified ID and attempt to cast as a specific type
	 
		nameID: The ID to search for
	 
		return The requested setting (nullptr if not found)
  	  --------------------------------------------------------------------*/
	template<typename T>
	T* SettingList::find(const utility::NameID& nameID) const {
		if (auto iter = find(nameID); iter != end())
			return dynamic_cast<T*>(iter->get());
		return nullptr;
	} //SettingList::find
	
}

#endif	//ACTIVE_SETTING_SETTING_LIST
