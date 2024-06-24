/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Generic/HexTransport.h"

#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/String.h"

using namespace active::serialise;
using namespace active::utility;

namespace {
	
		///The base hex numerals
	const char* hexNumerals = "0123456789ABCDEF";
		///The number of hex numerals required for a 32-bit integer
	const String::size_type hexIntLength = 8;

	/*!
		Get the next value from a hex numeral stream
		@param source The hex stream
		@return The next value from the stream (nullopt on failure)
	*/
	inline std::optional<unsigned char> getHexValue(const BufferIn& source) {
		unsigned char incoming;
		if (!source.good() || !source.get(incoming))
			return std::nullopt;
		if (incoming <= hexNumerals[9]) {
			if (incoming < hexNumerals[0])
				return std::nullopt;
			return incoming - hexNumerals[0];
		}
		incoming = std::toupper(incoming);
		if ((incoming < hexNumerals[10]) || (incoming > hexNumerals[15]))
			return std::nullopt;
		return incoming - hexNumerals[10] + 10;
	} //getHexValue
	
}  // namespace

/*--------------------------------------------------------------------
	Export specified data as hex chars
 
	source: The data to send
	destination: The destination buffer for the exported hex chars
	howMany: The number of bytes to export (nullopt = to eof)
 
	return: True if no errors occurred
  --------------------------------------------------------------------*/
bool HexTransport::send(const BufferIn& source, const BufferOut& destination, Memory::sizeOption howMany) const {
	bool isOpen = (howMany == std::nullopt);
	unsigned char incoming;
	while (!source.eof() && (isOpen || ((*howMany)-- > 0))) {
		if (!source.get(incoming))
			break;
		destination.write(hexNumerals[incoming >> 4]);
		destination.write(hexNumerals[incoming & 0x0F]);
	}
	return (destination.good() && (isOpen || ((*howMany + 1) == 0))) && destination.flush();
} //HexTransport::send
	

/*--------------------------------------------------------------------
	Receive specified data from a stream of hex characters
 
	destination: The buffer to receive the data
	source: The hex stream source
	howMany: The number of bytes to import (nullopt = to eof)
 
	return: True if the import was successful
  --------------------------------------------------------------------*/
bool HexTransport::receive(const BufferOut& destination, const BufferIn& source, Memory::sizeOption howMany) const {
	bool isUnlimited = howMany == std::nullopt;
	if (!isUnlimited) {
		if (*howMany == 0)
			return true;
		if (*howMany % 2 != 0) {
			if (auto firstNibble = getHexValue(source); firstNibble)
				destination.write(*firstNibble);
			else
				return false;
		}
	}
	while (!source.eof() && destination && (isUnlimited || (*howMany > 1))) {
		if (auto firstNibble = getHexValue(source), secondNibble = getHexValue(source); firstNibble && secondNibble)
			destination.write((*firstNibble << 4) | *secondNibble);
		else
			break;
		if (!isUnlimited)
			*howMany -= 2;
	}
	return (destination.good() && (isUnlimited || (*howMany == 0))) && destination.flush();
} //HexTransport::receive


/*--------------------------------------------------------------------
	Receive an integer from the specified hex chars
 
	source: The hex stream source
 
	return: The received integer (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<uint32_t> HexTransport::receive(const utility::String& source) const {
	const auto* target = &source;
	String cache;
		//The incoming hex stream is padded out to 8 numerals (if required) to be interpreted as a 32-bit integer
	if (source.size() < hexIntLength) {
		cache = source;
		cache.padRight(hexIntLength, "0");
		target = &cache;
	}
		//Read the hex stream directly into the memory for the int result
	uint32_t result = 0;
	if (!receive(BufferOut{Memory(result)}, BufferIn(*target), hexIntLength))
		return std::nullopt;
	result = Memory::fromBigEndian(result);
	return result;
} //HexTransport::receive
