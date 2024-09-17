#include "Active/Database/Storage/RecordCache.h"

using namespace active::serialise;
using namespace active::database;

#include <array>

namespace {

		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"object"},
	};

}

/*--------------------------------------------------------------------
	Get the identity of a specified field
 
	index: the field index
 
	return: The field identity
		const serialise::Identity& getIdentity(FieldIndex index);
  --------------------------------------------------------------------*/
const Identity& active::database::cache::getIdentity(FieldIndex index) {
	return fieldID[index];
} //active::database::cache::getIdentity
