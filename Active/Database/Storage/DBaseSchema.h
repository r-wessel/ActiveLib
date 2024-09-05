#ifndef ACTIVE_DATABASE_DBASE_SCHEMA
#define ACTIVE_DATABASE_DBASE_SCHEMA

#include "Active/Database/Storage/TableSchema.h"

#include <vector>

namespace active::database {
	
	/*!
	 Class describing the schema of a database table including fields and primary indices
	 */
	template<typename DBaseID = active::utility::String, typename TableID = active::utility::String>
	class DBaseSchema : public std::vector<TableSchema<TableID>> {
	public:
		using TableSchema = TableSchema<TableID>;
		using base = std::vector<TableSchema>;
		
		DBaseSchema(const DBaseID& nm, const std::initializer_list<TableSchema> tables) : base{tables}, name{nm} {}
		
		DBaseID name;
	};
	
}

#endif	//ACTIVE_DATABASE_DBASE_SCHEMA
