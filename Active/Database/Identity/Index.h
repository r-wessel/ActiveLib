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
	 */
	template<typename ObjID = active::utility::Guid>
	class Index : public ObjID {
	public:

		// MARK: - Constructors
		
		using ObjID::ObjID;
		
		/*!
		 Default constructor
		 @param object The object identifier
		 @param owner An optional runtime identifier for the object owner in memory
		 */
		Index(const ObjID& object = ObjID{}, std::any owner = std::any{}) : ObjID{object}, ownerID{owner} {}
		
		// MARK: - Public variables
		
			///An optional runtime identifier for the object owner in memory - can be used as required for a target application
		std::any ownerID;
	};
	
}

#endif	//ACTIVE_DATABASE_INDEX
