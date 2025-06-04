/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/NameID.h"

#include "Active/Utility/SHA256.h"

using namespace active::utility;

/*--------------------------------------------------------------------
	Conversion operator
 
	@return An unsigned 32-bit integer derived either (preferably) from the Guid or (alternatively) the name
  --------------------------------------------------------------------*/
NameID::operator uint32_t() const {
	uint64_t result = *this;
	return static_cast<uint32_t>((result >> 32) ^ result);
} //NameID::operator uint32_t


/*!
	Conversion operator
	@return An unsigned 64-bit integer derived either (preferably) from the Guid or (alternatively) the name
*/
NameID::operator uint64_t() const {
	if (id)
		return id.raw().first ^ id.raw().second;
	SHA256 hasher;
	hasher << name;
	auto hash = hasher.rawHash();
	uint64_t result = 0;
	for (auto i = 0; i < 8; i += 2)
		result ^= (static_cast<uint64_t>(hash[i]) << 32) | static_cast<uint64_t>(hash[i + 1]);
	return result;
} //NameID::operator uint64_t
