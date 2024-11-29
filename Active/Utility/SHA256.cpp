/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/SHA256.h"

#include "Active/Serialise/Generic/Base64Transport.h"
#include "Active/Serialise/Generic/HexTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"

using namespace active::serialise;
using namespace active::utility;

namespace {
	
		///SHA256 hash keys
	const uint32_t keys[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
	
		///Chunk bit size for compressing incoming data
	constexpr uint32_t chunkBits = 512;
		///Number of bytes per chunk
	constexpr uint32_t chunkSize = chunkBits / 8;
		///Word size for compression schedule table
	constexpr uint32_t scheduleWordSize = sizeof(uint32_t);
		///Size of 32-bit word scheduling table for data compression
	constexpr uint32_t scheduleSize = 64;
		///Size of chunk populated into schedule table
	constexpr uint32_t messageSize = chunkSize / scheduleWordSize;
}  // namespace

/*--------------------------------------------------------------------
	Default constructor
  --------------------------------------------------------------------*/
SHA256::SHA256() {
	reset();
} //SHA256::SHA256


/*--------------------------------------------------------------------
	Write data to the hash
 
	source: A data stream
 
	return: A reference to this
  --------------------------------------------------------------------*/
SHA256& SHA256::operator<<(BufferIn&& source) {
		//First use (or extend) any accumulated overflow
	if (m_overflowSize != 0) {
		Memory::size_type offset = m_overflowSize, readSize = chunkSize - offset;
		source.read(m_overflow.data() + offset, readSize);
		m_overflowSize += readSize;
		if (m_overflowSize < chunkSize)
			return *this;
		compress(m_overflow.data(), m_hash);
		m_hashedCount += chunkSize;
		m_overflowSize = 0;
	}
		//Then compress as many full chunks as possible from the source
	while (source && (source.bufferMin(chunkSize) >= chunkSize)) {
		compress(source.data(), m_hash);
		m_hashedCount += chunkSize;
		source.skip(chunkSize);
	}
		//Finally assign any remainder to overflow
	m_overflowSize = chunkSize;
	source.read(m_overflow.data(), m_overflowSize);
	return *this;
} //SHA256::operator<<


/*--------------------------------------------------------------------
	Get the data hash
 
	return: The hash (as hex digits)
  --------------------------------------------------------------------*/
String SHA256::hexHash() const {
	String result;
	HexTransport().send(BufferIn{getHash()}, result);
	return result;
} //SHA256::hexHash


/*--------------------------------------------------------------------
	Get the data hash
 
	return: The hash (as hex digits)
  --------------------------------------------------------------------*/
String SHA256::base64Hash() const {
	String result;
	Base64Transport().send(BufferIn{getHash()}, result);
	return result;
} //SHA256::base64Hash


/*--------------------------------------------------------------------
	Reset the hashing data (as if starting from scratch)
  --------------------------------------------------------------------*/
void SHA256::reset() {
	m_hash =  {
		0x6a09e667,
		0xbb67ae85,
		0x3c6ef372,
		0xa54ff53a,
		0x510e527f,
		0x9b05688c,
		0x1f83d9ab,
		0x5be0cd19
	};
	m_overflow.resize(chunkSize);
	m_hashedCount = 0;
	m_overflowSize = 0;
} //SHA256::reset


/*--------------------------------------------------------------------
	Run the compression algorithm using the next chunk of 512 bits
 
	chunk: Pointer to chunk start (must be at least 512 bits/64 bytes)
	hashOutput: The resulting hash
  --------------------------------------------------------------------*/
void SHA256::compress(const char* chunk, HashTable& hashOutput) const {
	std::array<uint32_t, scheduleSize> schedule;
	Memory::copy(reinterpret_cast<char*>(&schedule[0]), chunk, chunkSize, chunkSize);
		//Numbers must be big-endian
	for (auto i = 0; i < messageSize; ++i)
		schedule[i] = Memory::toBigEndian(schedule[i]);
		//Extend the 16 message words into the remaining 48 words
	for (auto i = messageSize; i < scheduleSize; ++i) {
		auto val = schedule[i - 15];
		auto s0 = std::rotr(val, 7) ^ std::rotr(val, 18) ^ (val >> 3);
		val = schedule[i - 2];
		auto s1 = std::rotr(val, 17) ^ std::rotr(val, 19) ^ (val >> 10);
		schedule[i] = schedule[i - 16] + s0 + schedule[i - 7] + s1;
	}
	auto a = hashOutput[0];
	auto b = hashOutput[1];
	auto c = hashOutput[2];
	auto d = hashOutput[3];
	auto e = hashOutput[4];
	auto f = hashOutput[5];
	auto g = hashOutput[6];
	auto h = hashOutput[7];
		//The main compression algorithm
	for (auto i = 0; i < scheduleSize; ++i) {
		auto S1 = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
		auto ch = (e & f) ^ ((~e) & g);
		auto temp1 = h + S1 + ch + keys[i] + schedule[i];
		auto S0 = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
		auto maj = (a & b) ^ (a & c) ^ (b & c);
		auto temp2 = S0 + maj;
		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}
	hashOutput[0] += a;
	hashOutput[1] += b;
	hashOutput[2] += c;
	hashOutput[3] += d;
	hashOutput[4] += e;
	hashOutput[5] += f;
	hashOutput[6] += g;
	hashOutput[7] += h;
} //SHA256::compress


/*--------------------------------------------------------------------
	Calculate the final hash
 
	return: The final hash (NB: this value is not stored in the object and the overflow is not erased, so more data could be written)
  --------------------------------------------------------------------*/
SHA256::HashTable SHA256::finalise() const {
		//NB: This process allows for the possibility that more data will be compressed into the hash, so copies are made of the hash etc
	HashTable result{m_hash};
		//Calculate the total number of bits in the hash (as big endian)
	auto totalBytes = Memory::toBigEndian(8 * (m_hashedCount + m_overflowSize));
		//Copy the overflow data and append a single '1' bit
	Memory overflow{m_overflow};
	auto overflowSize = m_overflowSize;
	overflow[overflowSize++] = '\x80';
		//Zero the remaining overflow
	for (auto i = overflowSize; i < chunkSize; ++i)
		overflow[i] = '\x00';
		//Add the total number of bits at the end of the overflow (if there is enough space)
	constexpr uint32_t bitCountStart = chunkSize - sizeof(uint64_t);
	if (overflowSize <= bitCountStart)
		Memory::copy(overflow.data() + bitCountStart, reinterpret_cast<const char*>(&totalBytes), sizeof(totalBytes), sizeof(totalBytes));
		//Compress the overflow into the result
	compress(overflow.data(), result);
		//If the bit count could be included in the overflow, we're done
	if (overflowSize <= bitCountStart)
		return result;
		//Zero the overflow
	Memory::fill(overflow.data(), chunkSize);
		//Add the total number of bits at the end
	Memory::copy(overflow.data() + bitCountStart, reinterpret_cast<const char*>(&totalBytes), sizeof(totalBytes), sizeof(totalBytes));
		//Compress the overflow into the result
	compress(overflow.data(), result);
	return result;
} //SHA256::finalise


/*--------------------------------------------------------------------
	Get the resulting hash
 
	return: The hash (an array of bytes)
  --------------------------------------------------------------------*/
Memory SHA256::getHash() const {
	auto finalHash = finalise();
	Memory hash;
	BufferOut buffer{hash};
	for (auto i = 0; i < finalHash.size(); ++i)
		buffer.writeBinary(Memory::toBigEndian(finalHash[i]));
	buffer.flush();
	return hash;
} //SHA256::getHash
