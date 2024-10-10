/*!
@copyright Copyright 2024 Ralph Wessel and Hugh Wessel
@license Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_GUID
#define ACTIVE_UTILITY_GUID

#include "Active/Utility/String.h"

namespace active::utility {
		
	/*!
		A class representing a GUID type
	*/
	class Guid {
	public:
		
		/*!
			Encode an integer into a guid - niche support for systems that use integer IDs rather than Guids (don't use otherwise)
			@param val The integer to encode. Note that this could be any lesser integer type, e.g. char, short etc.
			@return A guid - the lower 8 bytes are encoded from the integer and the remainder is zero (so guids from the same number are identical)
		*/
		static constexpr Guid fromInt(int64_t val) {
			return Guid{ Raw{0, val} };
		}
		/*!
			Decode an integer from a guid - niche support for systems that use integer IDs rather than Guids (don't use otherwise)
			@param guid The guid to decode (it is assumed that 'fromInt' has been used to generate thus guid)
			@return An integer decoded from the guid
		*/
		static constexpr int64_t toInt(const Guid& guid) { return guid.m_value.second; }
		
		// MARK: - Types
		
		using Raw = std::pair<uint64_t, uint64_t>;
			///Optional
		using Option = std::optional<Guid>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
			@param autoGenerate True to generate a guid value on construction
		*/
		Guid(bool autoGenerate = false);
		/*!
			Constructor
			@param uuidString The guid in string form
		*/
		explicit Guid(const String& uuidString);
		/*!
			constexpr constructor
			@param rawVal The guid raw value
		*/
		constexpr Guid(Raw rawVal) : m_value{rawVal} {}
		
		// MARK: - Operators

		friend auto operator<=>(const Guid&, const Guid&) = default;
		friend bool operator==(const Guid&, const Guid&) = default;
		friend bool operator!=(const Guid&, const Guid&) = default;
		friend bool operator<(const Guid&, const Guid&) = default;
		/*!
			Conversion operator
			@return The guid as a string
		*/
		operator String() const { return string(); }
		/*!
			Conversion operator
			@return The guid as a std::string
		*/
		operator std::string() const { return string(); }
		/*!
			Conversion operator
			@return True if the guid has a value (non-nil)
		*/
		operator bool() const { return !empty(); }
		
		// MARK: - Functions (const)
		
		/*!
			Return the raw (native) representation of the guid
			@return The raw native representation
		*/
		const Raw& raw() const { return m_value; }
		/*!
			Return a string representation of the guid
			@return A string representation
		*/
		String string() const;
		/*!
			Determine if the guid is an empty (nil) value
			@return True if the guid is empty
		*/
		bool empty() const { return (m_value.first == 0) && (m_value.second == 0); }
		
		// MARK: - Functions (mutating)
		
		/*!
			Reset the guid with a new value
		*/
		void reset();
		/*!
			Clear the guid value
		*/
		void clear() { m_value = {}; }
		
	private:
			//The guid value
		Raw m_value = {};
	};
	
}

	///Hashing for Guid class, e.g. to use as a key in unordered_map
template<>
struct std::hash<active::utility::Guid> {
	std::size_t operator() (const active::utility::Guid& guid) const {
		return static_cast<std::size_t>(guid.raw().first ^ guid.raw().second);
	}
};

#endif	//ACTIVE_UTILITY_GUID
