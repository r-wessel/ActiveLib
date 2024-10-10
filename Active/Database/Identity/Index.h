#ifndef ACTIVE_DATABASE_INDEX
#define ACTIVE_DATABASE_INDEX

#include "Active/Utility/Guid.h"

#include <any>

namespace active::database {
	
	/*!
	 Index to a database object
	 
	 A unique identifier for an object in a database. This can optionally include an identifier for a database (where multiple databases exist within
	 an app) or a table identifier (where the database is divided into tables). The specific form of these identifies is generic to support alternate
	 database engines. The database and table identifiers can be ignored for situations where this is not warranted
	 @tparam ObjID The object identifier type
	 @tparam TableID The source table identifier type
	 @tparam DocID The source document identifier type
	 */
	template<typename ObjID = active::utility::Guid, typename TableID = active::utility::Guid, typename DocID = active::utility::Guid>
	class Index : public ObjID {
	public:

		// MARK: - Constructors
		
		using ObjID::ObjID;
		
		/*!
		 Default constructor
		 */
		Index() {}
		/*!
		 Default constructor
		 @param objID The object identifier
		 @param tblID An optional table identifier
		 @param dcID An optional table identifier
		 */
		Index(const ObjID& objID, const TableID& tblID, const DocID& dcID = DocID{}) :
				ObjID{objID}, tableID{tblID}, docID{dcID} {}
		
		// MARK: - Public variables
		
			///An optional table identifier
		TableID tableID;
			///An optional document identifier
		DocID docID;
			///An optional runtime identifier for the object owner in memory - can be used as required for a target application
		std::any ownerID;
	};
	
}

#endif	//ACTIVE_DATABASE_INDEX
