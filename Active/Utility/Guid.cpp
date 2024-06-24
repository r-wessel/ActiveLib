/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/Guid.h"

#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"

#include <random>

using namespace active::serialise;
using namespace active::utility;

namespace {
	
	constexpr unsigned long long maxInt64 = std::numeric_limits<unsigned long long>::max();
	
	static const String nullHex{"0000000000000000"};

	
	uint64_t randomInt64() {
		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<unsigned long long> dis(0, maxInt64);
		return static_cast<uint64_t>(dis(gen));
	}
	
}  // namespace


/*--------------------------------------------------------------------
	Encode an integer into a guid - niche support for systems that use integer IDs rather than Guids (don't use otherwise)
 
	val: The integer to encode. Note that this could be any lesser integer type, e.g. char, short etc.
 
	return: A guid - the lower 8 bytes are encoded from the integer and the remainder is zero (so guids from the same number are identical)
  --------------------------------------------------------------------*/
Guid Guid::fromInt(int64_t val) {
	Guid result;
	result.m_value.second = val;
	return result;
} //Guid::fromInt


/*--------------------------------------------------------------------
	Decode an integer from a guid - niche support for systems that use integer IDs rather than Guids (don't use otherwise)
 
	guid: The guid to decode (it is assumed that 'fromInt' has been used to generate this guid)
 
	return: An integer decoded from the guid
  --------------------------------------------------------------------*/
int64_t Guid::toInt(const Guid& guid) {
	return guid.m_value.second;
} //Guid::toInt


/*--------------------------------------------------------------------
	Default constructor
 
	autoGenerate: True to generate a guid value on construction
  --------------------------------------------------------------------*/
Guid::Guid(bool autoGenerate) {
	if (autoGenerate)
		reset();
} //Guid::Guid


/*--------------------------------------------------------------------
	Constructor
 
	uuidString: The guid in string form
  --------------------------------------------------------------------*/
Guid::Guid(const String& uuidString) {
	auto incoming{uuidString};
		//Remove all dashes to create a simple hex stream
	incoming.replaceAll("-", String{});
		//We need 16 characters for a valid guid
	if (incoming.size() != 32)
		return;
		//Convert the incoming hex to 64-bit integers
	BufferIn source{incoming};
	HexTransport transport;
	if (!transport.receive(Memory{m_value.first}, source, 16) || !transport.receive(Memory{m_value.second}, source, 16))
		return;
	m_value.first = Memory::fromBigEndian(m_value.first);
	m_value.second = Memory::fromBigEndian(m_value.second);
} //Guid::Guid


/*--------------------------------------------------------------------
	Return a string representation of the guid
 
	return: A string representation
  --------------------------------------------------------------------*/
String Guid::string() const {
	String first, second;
		//Convert the first half to hex and format
	auto val = Memory::toBigEndian(m_value.first);
	if (!HexTransport().send(Memory{val}, first) || (first.length() != 16))
		first = nullHex;
	first.insert(8, "-");
	first.insert(13, "-");
		//Convert the second half to hex and format
	val = Memory::toBigEndian(m_value.second);
	if (!HexTransport().send(Memory{val}, second) || (second.length() != 16))
		second = nullHex;
	second.insert(4, "-");
		//Join the halves for the result
	return first + "-" + second;
} //Guid::string


/*--------------------------------------------------------------------
	Reset the guid with a new value
  --------------------------------------------------------------------*/
void Guid::reset() {
	m_value.first = randomInt64();
	m_value.second = randomInt64();
} //Guid::reset
