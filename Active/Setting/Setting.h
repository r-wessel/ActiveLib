/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_SETTING
#define ACTIVE_SETTING_SETTING

#include "Active/Utility/Cloner.h"
#include "Active/Utility/NameID.h"

namespace active::setting {

		///Base class for an identified setting for interprocess communication
	class Setting : public utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Setting>;
			///Shared pointer
		using Shared = std::shared_ptr<Setting>;
			///Optional
		using Option = std::optional<Setting>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param identifier An optional identifier for the setting
		*/
		Setting(utility::NameID::Option identifier = std::nullopt) : identity{identifier}	{}
		/*!
			Constructor
			@param name An optional identifying name for the setting
			@param guid An optional identifying guid for the setting
		*/
		Setting(utility::String::Option name, utility::Guid::Option guid = std::nullopt) {
			if (name || guid)
				identity = std::make_optional(utility::NameID{name.value_or(utility::String{}), guid.value_or(utility::Guid{})});
		}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Setting(const Setting& source) {
			if (source.identity)
				identity = std::make_optional<utility::NameID>(*source.identity);
		}
		
		/*!
			Destructor
		*/
		virtual ~Setting() = default;

		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Setting* clonePtr() const = 0;

		// MARK: - Public variables
		
			///Optional setting identity
		utility::NameID::Option identity;
		
		// MARK: Operators
		
		/*!
			Equality operator
			@param ref The value to compare
		 	@return True if the values are identical
		*/
		virtual bool operator==(const Setting& ref) const = 0;
		/*!
			Less-than operator
			@param ref The value to compare
		 	@return True if this is less than ref
		*/
		virtual bool operator<(const Setting& ref) const = 0;
		/*!
			Assignment operator
			@param source The object to copy
		 	@return A reference to this
		*/
		virtual Setting& operator=(const Setting& source) = 0;

		// MARK: - Functions (const)
		
			///True if the setting is empty
		virtual bool empty() const = 0;

			///True if the setting is empty
		virtual utility::String name() const { return identity.has_value() ? identity->name : utility::String{}; }

		// MARK: Functions (mutating)
		
		/*!
			Clear the setting (remove any values)
		*/
		virtual void clear() {}
		/*!
			Assign a default to the setting
		*/
		virtual void setDefault() = 0;
	};
	
}

#endif	//ACTIVE_SETTING_SETTING
