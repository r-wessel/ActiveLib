#ifndef ACTIVE_DATABASE_STORAGE
#define ACTIVE_DATABASE_STORAGE

#include "Active/Container/Vector.h"
#include "Active/Utility/String.h"

namespace active::database {
	
	/*!
	 Interface for components providing database storage
	 @tparam Obj Interface for the stored object
	 @tparam Trans The transport mechanism for objects
	 @tparam DocID The document identifier type
	 @tparam ObjID The object identifier type
	 @tparam DBaseID The database identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename Obj, typename Transport, typename DocID = active::utility::Guid,
			typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class Storage {
	public:
		using Engine = DBaseEngine<Obj, Transport, DocID, ObjID, DBaseID, TableID>;
		
		/*!
		 Get an object by index
		 @param index The object index
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return: The requested object (nullptr on failure)
		 */
		template<typename T = Obj>
		std::unique_ptr<T> getObject(const Object::Index& index, std::optional<DocID> documentID = std::nullopt) const;
		/*!
		 Get all objects
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return: The requested objects (nullptr on failure)
		 @paramt: The record type
		 */
		template<typename T = Obj>
		active::container::Vector<std::unique_ptr<T>> getObjects(std::optional<DocID> documentID = std::nullopt) const;

	private:
		std::unique_ptr<Engine> m_engine;
	};
	
}

#endif	//ACTIVE_DATABASE_STORAGE
