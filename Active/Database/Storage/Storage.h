#ifndef ACTIVE_DATABASE_STORAGE
#define ACTIVE_DATABASE_STORAGE

#include "Active/Container/Vector.h"
#include "Active/Database/Storage/DBaseEngine.h"
#include "Active/Utility/String.h"

namespace active::database {
	
	/*!
	 Template interface for components providing database storage. NB: This defines storage behaviour without prescribing an engine or schema
	 @tparam Obj Interface for the stored object. NB: This can be a base class for an object hierarchy, not necessarily a concrete class
	 @tparam Transport The serialisation transport mechanism for objects
	 @tparam DocID The document identifier type, e.g. Guid. The type is arbitrary if a document structure is not employed
	 @tparam ObjID The object identifier type, e.g. Guid
	 @tparam DBaseID The database identifier type, e.g. Guid (or String for named dbases)
	 @tparam TableID The table identifier type, e.g. Guid (or String for named tables)
	 */
	template<typename Obj, typename Transport, typename DocID = active::utility::Guid,
			typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class Storage {
	public:
			///The engine driving dbase operations. NB: This is independent of the Storage wrapper, i.e. one wrapper could employ multiple engines
		using Engine = DBaseEngine<Obj, DocID, ObjID, TableID>;
			///The database schema, including tables
		using DBaseSchema = DBaseSchema<DBaseID, TableID>;
			///Tables within the database, including a full description of member fields (names/types) and primary index/content fields
		using TableSchema = DBaseSchema::TableSchema;

		/*!
		 Constructor
		 @param engine The database engine
		 */
		Storage(std::unique_ptr<Engine> engine) : m_engine{std::move(engine)} {}
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		Storage(const Storage& source) : m_engine{std::move(clone(*source.m_engine))} {}
		/*!
		 Destructor
		 */
		virtual ~Storage() {}
		
		/*!
		 Get an object by index
		 @param index The object index
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested object (nullptr on failure)
		 @tparam T The type of the expected result. Intended to save repetitive casting code - could be used as a filter, but not the most efficient
		 */
		template<typename T = Obj>
		std::unique_ptr<T> getObject(const ObjID& index, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			auto result = m_engine->getObject(index, tableID, documentID);
			if constexpr (std::is_same_v<T, Obj>)
				return result;
			else {
				auto object = dynamic_cast<T*>(result.get());
				if (object == nullptr)
					return nullptr;
				result.release();
				return std::unique_ptr<T>(object);
			}
		}
		/*!
		 Get all objects from a database table
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The requested objects (nullptr on failure)
		 */
		active::container::Vector<Obj> getObjects(std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			return m_engine->getObjects(tableID, documentID);
		}

	private:
		std::unique_ptr<Engine> m_engine;
	};
	
}

#endif	//ACTIVE_DATABASE_STORAGE
