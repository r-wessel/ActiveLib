/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_IDENTITY
#define ACTIVE_SERIALISE_IDENTITY

#include "Active/Setting/Values/Value.h"
#include "Active/Utility/String.h"

namespace active::serialise {
	
	/*!
		Cargo identification class
		
		The primary forms of identifying cargo - a name and group. Used in an inventory to uniquely identify cargo during import/export
	*/
	struct Identity {

		// MARK: - Types

		enum class Role {
			attribute,
			element,
			array,
		};

		// MARK: - Constructors
		
		/*!
			Default constructor
			@param nameIn An identifying name
		*/
		Identity(const utility::String& nameIn = utility::String{}) {
			name = nameIn;
		}

		/*!
			Default constructor
			@param nameIn An identifying name
			@param groupIn An optional identifying group
		*/
		Identity(const utility::String& nameIn, const utility::String::Option groupIn) {
			name = nameIn;
			group = groupIn;
		}
		
		// MARK: - Variables
		
			///An identifying name
		utility::String name;
			///Optional named group (in which the name is unique, i.e. to avoid name clashes)
		utility::String::Option group = std::nullopt;
			///The expected inventory role (applicable when a new identity has been parsed)
		std::optional<Role> entryRole;
			///The value type (applicable when a new value has been parsed)
		std::optional<setting::Value::Type> valueType;
	};
		
	// MARK: - Operators
	
		///Less-than operator
	inline bool operator< (const Identity& lhs, const Identity& rhs) {
		return (lhs.group == rhs.group) ? lhs.name < rhs.name : lhs.group < rhs.group;
	}

		///Equality operator
	inline bool operator== (const Identity& lhs, const Identity& rhs) {
		return (lhs.name == rhs.name) && (lhs.group == rhs.group);
	}

		///Equality operator
	inline bool operator!= (const Identity& lhs, const Identity& rhs) {
		return !(lhs == rhs);
	}
	
}  // namespace active::serialise

	///Hashing for Identity class, e.g. to use as a key in unordered_map
template<>
struct std::hash<active::serialise::Identity> {
	std::size_t operator()(const active::serialise::Identity& identity) const noexcept {
		std::size_t h1 = std::hash<active::utility::String>{}(identity.name);
		if (!identity.group)
			return h1;
		std::size_t h2 = std::hash<active::utility::String>{}(*identity.group);
		return h1 ^ (h2 << 1);
	}
};

#endif	//ACTIVE_SERIALISE_IDENTITY
