/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_BASE64_TRANSPORT
#define ACTIVE_SERIALISE_BASE64_TRANSPORT

#include "Active/Utility/Memory.h"

#include <string>

namespace active::utility {
	
	class BufferIn;
	class BufferOut;
	class String;

}

namespace active::serialise {
	
		///Standard base64 numerals
	inline std::string standardEncoding{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
		///IFC guid base64 numerals
	inline std::string ifcGuidEncoding{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_$"};
	
	/*!
		A serialisation tool class to send/receive data encoded as base64 characters
	*/
	class Base64Transport {
	public:
		/*!
		 Set an alternative base64 encoding table
		 @param table The alternative table
		 @return A reference to this
		 */
		Base64Transport& withEncodingTable(const std::string& table) {
			m_encodingTable = table;
			return *this;
		}
		
		/*!
			Export specified data as hex chars
			@param source The data to send
			@param destination The destination buffer for the exported hex chars
			@param howMany The number of bytes to export (nullopt = to eof)
			@return True if no errors occurred
		*/
		bool send(const utility::BufferIn& source, const utility::BufferOut& destination, utility::Memory::sizeOption howMany = std::nullopt) const;
		/*!
			Receive specified data from a stream of hex characters
			@param destination The buffer to receive the data
			@param source The hex stream source
			@param howMany The number of bytes to import (nullopt = to eof)
			@return True if the import was successful
		*/
		bool receive(const utility::BufferOut& destination, const utility::BufferIn& source, utility::Memory::sizeOption howMany = std::nullopt) const;
		
	private:
			//Optional encoding table to support different standards
		std::optional<std::string> m_encodingTable;
	};
	
	
}

#endif	//ACTIVE_SERIALISE_BASE64_TRANSPORT
