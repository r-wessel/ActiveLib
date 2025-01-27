/*!
@copyright Copyright 2024 Ralph Wessel and Hugh Wessel
@license Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_SHA256
#define ACTIVE_UTILITY_SHA256

#include "Active/Utility/BufferIn.h"
#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"

#include <array>

namespace active::utility {
		
	/*!
	 A utility class providing SHA256 hashing
	 
	 See https://en.wikipedia.org/wiki/SHA-2#Pseudocode
	 This class can be used in a variety of ways:
	 - Hashing a block of data in a string, file or memory
	 - Hashing disparate fields together, e.g. for an object hash
	 - Any combination of the above
	 
	 The finalised hash can be obtained as hex or base64 by calling hexHash() or base64Hash(). Note that it is still possible to keep feeding data
	 into the hash even after a finalised hash has been obtained in order to obtain another hash. The method for finalising a result works on a
	 temporary copy of the hashing data and leaves the original untouched
	 */
	class SHA256 {
	public:
		
		// MARK: Constructors
		
		/*!
		 Default constructor
		 */
		SHA256();
		
		// MARK: Operators
		
		/*!
		 Write data to the hash (NB: This does not prevent additional data from being written to the hash)
		 @param source A data stream
		 @return A reference to this
		 */
		SHA256& operator<<(BufferIn&& source);
		/*!
		 Add a field to the hash (use this for data affected by endianness)
		 @param field The field to add (will be hashed as big endian)
		 @return A reference to this
		 */
		template<typename T> requires (std::is_arithmetic_v<T>)
		inline SHA256& operator<<(const T& field) {
			auto temp = field;
			return *this << BufferIn{Memory{Memory::toBigEndian(temp)}};
		}
		
		// MARK: Functions (const)
		
		/*!
		 Get the data hash (NB: This does not prevent additional data from being written to the hash)
		 @return The hash (as hex digits)
		 */
		String hexHash() const;
		/*!
		 Get the data hash
		 @return The hash (as hex digits)
		 */
		String base64Hash() const;
		
		// MARK: Functions (mutating)
		
		/*!
		 Reset the hashing data (as if starting from scratch)
		 */
		void reset();
		
	private:
		using HashTable = std::array<uint32_t, 8>;
		
		/*!
		 Run the compression algorithm using the next chunk of 512 bits
		 @param chunk Pointer to chunk start
		 @param hashOutput The resulting hash
		 */
		void compress(const char* chunk, HashTable& hashOutput) const;
		/*!
		 Calculate the final hash
		 @return The final hash (NB: this value is not stored in the object and the overflow is not erased, so more data could be written)
		 */
		HashTable finalise() const;
		/*!
		 Get the resulting hash
		 @return The hash (an array of bytes)
		 */
		Memory getHash() const;
			
			///Hash result
		HashTable m_hash;
			///Number of hashed bytes
		uint64_t m_hashedCount = 0;
			///Overflow from 512-bit chunk processing
		Memory m_overflow;
			///Number of bytes in overflow
		Memory::size_type m_overflowSize = 0;
	};
	
}

#endif	//ACTIVE_UTILITY_SHA256
