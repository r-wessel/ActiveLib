/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/Guid.h"

#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/StackBufferOut.h"

#include <random>

using namespace active;
using namespace active::serialise;

namespace {
	
		///Maximum numeric value for a guid
	constexpr unsigned long long maxInt64 = std::numeric_limits<unsigned long long>::max();
		///Size of a guid as a string (without dashes)
	constexpr size_t guidHexLength = 32;
		///Size of a (half) guid as a string
	constexpr size_t guidHalfHexLength = 16;
		///Placeholder for an empty guid string
	static const string nullHex{"0000000000000000"};

	
	uint64_t randomInt64() {
		std::random_device rd;
		std::mt19937_64 gen(rd());
		std::uniform_int_distribution<unsigned long long> dis(0, maxInt64);
		return static_cast<uint64_t>(dis(gen));
	}


}  // namespace

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
Guid::Guid(const string& uuidString) {
	auto incoming{uuidString};
		//Remove all dashes to create a simple hex stream
	incoming.replace_all("-", string{});
		//We need 16 characters for a valid guid
	if (incoming.size() != guidHexLength)
		return;
		//Convert the incoming hex to 64-bit integers
	BufferIn source{incoming};
	HexTransport transport;
	if (!transport.receive(Memory{m_value.first}, source, guidHalfHexLength) ||
			!transport.receive(Memory{m_value.second}, source, guidHalfHexLength))
		return;
	m_value.first = Memory::fromBigEndian(m_value.first);
	m_value.second = Memory::fromBigEndian(m_value.second);
} //Guid::Guid


/*--------------------------------------------------------------------
	Return a string representation of the guid
 
	inCase: The digit case
 
	return: A string representation
  --------------------------------------------------------------------*/
string Guid::to_string(Case inCase) const {
	string first, second;
		//Convert the first half to hex and format
	auto val = Memory::toBigEndian(m_value.first);
	if (!HexTransport(inCase).send(Memory{val}, StackBufferOut<guidHalfHexLength>{first}) || (first.length() != guidHalfHexLength))
		first = nullHex;
	first.insert(8, "-");
	first.insert(13, "-");
		//Convert the second half to hex and format
	val = Memory::toBigEndian(m_value.second);
	if (!HexTransport().send(Memory{val}, StackBufferOut<guidHalfHexLength>{second}) || (second.length() != guidHalfHexLength))
		second = nullHex;
	second.insert(4, "-");
		//Join the halves for the result
	return first + "-" + second;
} //Guid::to_string


/*--------------------------------------------------------------------
	Reset the guid with a new value
  --------------------------------------------------------------------*/
void Guid::reset() {
	m_value.first = randomInt64();
	m_value.second = randomInt64();
} //Guid::reset
