/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_HEX_TRANSPORT
#define ACTIVE_SERIALISE_HEX_TRANSPORT

#include "Active/Utility/Memory.h"

namespace active::utility {
	
	class BufferIn;
	class BufferOut;
	class String;

}

namespace active::serialise {
	
	/*!
		A serialisation tool class to send/receive data encoded as hex characters
	*/
	class HexTransport {
	public:
		/*!
			Export specified data as hex chars
			@param source The data to send (NB: binary data is assumed to be big-endian - prepare the source accordingly)
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
		/*!
			Receive an integer from the specified hex chars
			@param source The hex stream source
			@return The received integer (nullopt on failure)
		*/
		std::optional<uint32_t> receive(const utility::String& source) const;
	};
	
	
}

#endif	//ACTIVE_SERIALISE_HEX_TRANSPORT
