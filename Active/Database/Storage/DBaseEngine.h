#ifndef ACTIVE_DATABASE_DBASE_ENGINE
#define ACTIVE_DATABASE_DBASE_ENGINE

#include "Active/Container/Vector.h"
#include "Active/Utility/String.h"

namespace active::database {
	
	/*!
	 Interface for a database engine
	 
	 The role of an engine is to manage object content including storage, retrieval, deletion etc
	 @tparam Obj Interface for the stored object
	 @tparam Transport The transport mechanism for objects
	 @tparam DocID The document identifier type
	 @tparam ObjID The object identifier type
	 @tparam DBaseID The database identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename Obj, typename Transport, typename DocID = active::utility::Guid,
			typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class DBaseEngine {
	public:
		
		/*!
		 Get an object by index
		 @param index The object index
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return: The requested object (nullptr on failure)
		 */
		std::unique_ptr<Obj> getObject(const Obj::Index& index, std::optional<DocID> documentID = std::nullopt) const;
		/*!
		 Get all objects
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return: The requested objects (nullptr on failure)
		 */
		active::container::Vector<std::unique_ptr<Obj>> getObjects(std::optional<DocID> documentID = std::nullopt) const;
	};
	
}

#endif	//ACTIVE_DATABASE_DBASE_ENGINE
