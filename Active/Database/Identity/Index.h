#ifndef ACTIVE_DATABASE_INDEX
#define ACTIVE_DATABASE_INDEX

#include "Active/Utility/Guid.h"

namespace active::database {
	
	/*!
	 Index to a database object
	 
	 A unique identifier for an object in a database. This can optionally include an identifier for a database (where multiple databases exist within
	 an app) or a table identifier (where the database is divided into tables). The specific form of these identifies is generic to support alternate
	 database engines. The database and table identifiers can be ignored for situations where this is not warranted
	 @tparam ObjID The object identifier type
	 @tparam DBaseID The database identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class Index : public ObjID {
	public:

		// MARK: - Constructors
		
		using ObjID::ObjID;
		
		/*!
		 Default constructor
		 @param object The object identifier
		 @param dbase The database identifier (nullopt = undefined - can be valid where there is a single database or universal search is available)
		 @param table The table identifier (nullopt = undefined - can be valid where there is a single database or universal search is available)
		 */
		Index(const ObjID& object = ObjID{}, std::optional<DBaseID> dbase = std::nullopt, std::optional<TableID> table = std::nullopt) :
				ObjID{object}, databaseID{dbase}, tableID{table} {}
		
		// MARK: - Public variables
		
			///Optional database identifier
		std::optional<DBaseID> dbaseID;
			///Optional table identifier (where the source database is divided into tables)
		std::optional<TableID> tableID;
	};
	
}

#endif	//ACTIVE_DATABASE_INDEX
