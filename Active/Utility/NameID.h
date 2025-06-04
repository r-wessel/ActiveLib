/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_NAME_ID
#define ACTIVE_UTILITY_NAME_ID

#include "Active/Utility/Guid.h"

#include <optional>

namespace active::utility {
		
	/*!
		A utility class for a form of identity combining a name and guid
		
		Any combination of name and guid can be used, i.e. just name, just guid or both (depending on context). The intention is to provide a
		single container that can be used in a variety of contexts where names or IDs might both come into play in different combinations
	*/
	class NameID {
	public:
		
		// MARK: - Types
		
			///Optional
		using Option = std::optional<NameID>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		NameID() {}
		/*!
			Constructor
			@param str The identifying name
			@param guid The guid
		*/
		NameID(const String& str, const Guid& guid = Guid{}) : name{str}, id{guid} {}
		/*!
			Constructor
			@param str The identifying name
			@param guid The guid
		*/
		NameID(const char* str, const Guid& guid = Guid{}) : name{str}, id{guid} {}
		/*!
			Constructor
			@param guid The guid
			@param str The identifying name
		*/
		NameID(const Guid& guid, const String& str = String{}) : name{str}, id{guid} {}
		
		// MARK: - Variables
		
			//Optional identifying name (empty = unused)
		String name;
			//Optional guid (undefined = unused)
		Guid id;
		
		// MARK: - Operators

		/*!
			Equality operator
			@param ref The object to compare with this
			@return True if ref is equal to this
		*/
		bool operator== (const NameID& ref) const { return (id || ref.id) ? (id == ref.id) : (name == ref.name); }
		/*!
			Inequality operator
			@param ref The object to compare with this
			@return True if ref is not equal to this
		*/
		bool operator!= (const NameID& ref) const { return !(*this == ref); }
		/*!
			Less-than operator
			@param ref The object to compare with this
			@return True if this is less than ref
		*/
		bool operator< (const NameID& ref) const { return ((id || ref.id) && (id != ref.id)) ? (id < ref.id) : (name < ref.name); }
		/*!
			Conversion operator
			@return True if the guid has a value (non-nil)
		*/
		operator bool() const { return id || !name.empty(); }
		/*!
			Conversion operator
			@return An unsigned 32-bit integer derived either (preferably) from the Guid or (alternatively) the name
		*/
		operator uint32_t() const;
		/*!
			Conversion operator
			@return An unsigned 64-bit integer derived either (preferably) from the Guid or (alternatively) the name
		*/
		operator uint64_t() const;
		
		// MARK: - Functions (const)
		
		
		// MARK: - Functions (mutating)
		
		/*!
			Clear the guid value
		*/
		void clear() {
			name.clear();
			id.clear();
		}
	};
	
}

	///Hashing for NameID class, e.g. to use as a key in unordered_map
template<>
struct std::hash<active::utility::NameID> {
	std::size_t operator()(const active::utility::NameID& nameID) const noexcept {
		std::size_t h1 = std::hash<active::utility::String>{}(nameID.name);
		std::size_t h2 = std::hash<active::utility::Guid>{}(nameID.id);
		return h1 ^ (h2 << 1);
	}
};

#endif	//ACTIVE_UTILITY_NAME_ID
