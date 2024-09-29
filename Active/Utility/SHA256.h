/*!
@copyright Copyright 2024 Ralph Wessel and Hugh Wessel
@license Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_SHA256
#define ACTIVE_UTILITY_SHA256

#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"

#include <array>

namespace active::utility {
		
	/*!
		A class representing a SHA256 type
	*/
	class SHA256 {
	public:
		
		/*!
		 Default constructor
		 */
		SHA256();
		
		/*!
		 Write data to the hash (NB: This does not prevent additional data from being written to the hash)
		 @param source A data stream
		 @return A reference to this
		 */
		SHA256& operator<<(BufferIn&& source);
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
		
	private:
		using HashTable = std::array<uint32_t, 8>;
		
		/*!
		 Run the compression algorithm using the next chunk of 512 bits
		 @param chunk Pointer to chunk start
		 @param hashOutput The resulting hash
		 */
		void compress(const char* chunk, HashTable& hashOutput) const;
		/*!
		 Calculate the final hash consuming whatever overflow data remains
		 @return The final hash (NB: this value is not stored in the object and the overflow is not erased, so more data could be written)
		 */
		HashTable finaliseOverflow() const;
		/*!
		 Get the resulting hash
		 @return The hash (an array of bytes)
		 */
		Memory getHash() const;
			
			///Hash result
		HashTable m_hash;
			///Number of hashed bytes
		uint64_t m_hashedCount = 0;
			///Overflow from 512-byte chunk processing
		Memory m_overflow;
	};
	
}

#endif	//ACTIVE_UTILITY_SHA256
