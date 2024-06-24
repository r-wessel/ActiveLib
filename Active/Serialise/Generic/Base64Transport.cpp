/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Generic/Base64Transport.h"

#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/String.h"

using namespace active::serialise;
using namespace active::utility;

namespace {
	
		///The base64 numerals
	std::string base64Numerals{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
	
	std::vector<int8_t> base64Lookup;

} // namespace

/*--------------------------------------------------------------------
	Export specified data as base64 chars
 
	source: The data to send
	destination: The destination buffer for the exported base64 chars
	howMany: The number of bytes to export (nullopt = to eof)
 
	return: True if no errors occurred
  --------------------------------------------------------------------*/
bool Base64Transport::send(const BufferIn& source, const BufferOut& destination, Memory::sizeOption howMany) const {
	bool isOpen = (howMany == std::nullopt);
	Memory::size_type written = 0;
	while (!source.eof() && (isOpen || ((*howMany)-- > 0))) {
		uint64_t incoming = 0;
		Memory::size_type incomingSize = 6;
		if (!source.read(reinterpret_cast<char*>(&incoming), incomingSize) || (incomingSize == 0))
			break;
		incomingSize = ((incomingSize * 8) + 5) / 6; //The number of outgoing base64 numerals (number of outgoing bits rounded to blocks of 6 bits)
		incoming = Memory::toBigEndian(incoming);
		uint64_t mask = 0xFC00000000000000;
		uint8_t shift = 58;
		while (incomingSize-- > 0) {
			if (!destination.write(base64Numerals[static_cast<unsigned char>((incoming & mask) >> shift)]))
				return false;
			++written;
			mask >>= 6;
			shift -= 6;
		}
	}
	written %= 4;
	if (written > 0) {
		if (written > 1)
			destination.write(String{"=="}.substr(written - 2));
		else
			return false;
	}
	return (destination.good() && (isOpen || ((*howMany + 1) == 0))) && destination.flush();
} //Base64Transport::send
	

/*--------------------------------------------------------------------
	Receive specified data from a stream of base64 characters
 
	destination: The buffer to receive the data
	source: The base64 stream source
	howMany: The number of bytes to import (nullopt = to eof)
 
	return: True if the import was successful
  --------------------------------------------------------------------*/
bool Base64Transport::receive(const BufferOut& destination, const BufferIn& source, Memory::sizeOption howMany) const {
	bool isOpen = howMany == std::nullopt;
	if (!isOpen && (*howMany == 0))
		return true;	//Nothing to read
	if (base64Lookup.empty()) {
		base64Lookup.resize(256, -1);
		for (auto i = 0; i < base64Numerals.size(); ++i)
			base64Lookup[base64Numerals[i]] = i;
		base64Lookup['='] = -2;
	}
		//Read the incoming base64 in chunks of (up to) 8 bytes - should result in up to (exactly) 6 bytes of incoming decoded data
	constexpr int bufferInSize = 8;
	while (!source.eof() && destination.good() && (isOpen || (*howMany > 1))) {
		Memory::size_type incomingSize = bufferInSize, outgoingSize = 0;
		unsigned char incoming[bufferInSize];
		uint64_t outgoing = 0;
		if (!source.read(reinterpret_cast<char*>(incoming), incomingSize) || (incomingSize == 0))
			break;
		uint64_t mask = 0xFF00000000000000;
		uint8_t shift = 56;
		for (auto i = 0; i < incomingSize; ++i) {
			auto next = base64Lookup[incoming[i]];
			if (next < 0) {
				if (next == -1)
					return false;	//Found an invalid base64 numeral
				break;
			}
			outgoing <<= 6;
			outgoing |= next;
			outgoingSize += 6;
		}
		if (outgoingSize < 8)
			return false;	//Not enough bits to decode even a single byte (can't be valid)
		auto byteCount = outgoingSize / 8;
		outgoing >>= outgoingSize % 8;
		shift = static_cast<uint8_t>(8 * (byteCount - 1));
		mask = 0x00000000000000FF;
		mask <<= shift;
		while ((byteCount-- > 0) && destination.write(static_cast<unsigned char>((outgoing & mask) >> shift))) {
			mask >>= 8;
			shift -= 8;
		}
	}
	return (destination.good() && (isOpen || ((*howMany + 1) == 0))) && destination.flush();
} //Base64Transport::receive
