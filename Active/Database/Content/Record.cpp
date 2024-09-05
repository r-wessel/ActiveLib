#include "Active/Database/Content/Record.h"

using namespace active::serialise;
using namespace active::database;

#include <array>

namespace {

		///Serialisation field IDs
	static std::array fieldID = {
		Identity{"id"},
		Identity{"globID"},
		Identity{"created"},
		Identity{"edited"},
	};

}

/*--------------------------------------------------------------------
	Get the identity of a specified field
 
	index: the field index
 
	return: The field identity
		const serialise::Identity& getIdentity(FieldIndex index);
  --------------------------------------------------------------------*/
const Identity& record::getIdentity(FieldIndex index) {
	return fieldID[index];
} //record::getIdentity
