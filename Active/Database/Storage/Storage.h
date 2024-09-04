#ifndef ACTIVE_DATABASE_STORAGE
#define ACTIVE_DATABASE_STORAGE

#include "Active/Container/Vector.h"
#include "Active/Database/Storage/DBaseEngine.h"
#include "Active/Utility/String.h"

namespace active::database {
	
	/*!
	 Interface for components providing database storage
	 @tparam Obj Interface for the stored object
	 @tparam Transport The transport mechanism for objects
	 @tparam DocID The document identifier type
	 @tparam ObjID The object identifier type
	 @tparam DBaseID The database identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename Obj, typename Transport, typename DocID = active::utility::Guid,
			typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class Storage {
	public:
		using Engine = DBaseEngine<Obj, DocID, ObjID, TableID>;
		using DBaseSchema = DBaseSchema<DBaseID, TableID>;
		using TableSchema = DBaseSchema::TableSchema;

		/*!
		 Constructor
		 @param engine The database engine
		 */
		Storage(std::unique_ptr<Engine> engine) : m_engine{std::move(engine)} {}
		
		Storage(const Storage& source) : m_engine{std::move(clone(*source.m_engine))} {}
		
		virtual ~Storage() {}
		
		/*!
		 Get an object by index
		 @param index The object index
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested object (nullptr on failure)
		 */
		template<typename T = Obj>
		std::unique_ptr<T> getObject(const Obj::Index& index, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			return m_engine->getObject(index, tableID, documentID);
		}
		/*!
		 Get all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The requested objects (nullptr on failure)
		 */
		template<typename T = Obj>
		active::container::Vector<T> getObjects(std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			return m_engine->getObjects(tableID, documentID);
		}

	private:
		std::unique_ptr<Engine> m_engine;
	};
	
}

#endif	//ACTIVE_DATABASE_STORAGE
