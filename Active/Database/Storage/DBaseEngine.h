#ifndef ACTIVE_DATABASE_DBASE_ENGINE
#define ACTIVE_DATABASE_DBASE_ENGINE

#include "Active/Container/Vector.h"
#include "Active/Serialise/Cargo.h"
#include "Active/Utility/Guid.h"

#include <unordered_set>

namespace active::database {
	
	/*!
	 Interface for a database engine
	 
	 The role of an engine is to manage object content including storage, retrieval, deletion etc
	 @tparam Obj Interface for the stored object
	 @tparam ObjID The object identifier type
	 @tparam DocID The document identifier type
	 @tparam TableID The table identifier type
	 @tparam TableType An identifier for a table type or group
	 */
	template<typename Obj, typename ObjID = active::utility::Guid, typename DocID = active::utility::Guid,
			typename TableID = active::utility::Guid, typename TableType = int32_t>
	class DBaseEngine {
	public:
		
		// MARK: - Types
		
			///Unary predicate for filtering objects
		using Filter = std::function<bool(const Obj&)>;
			///A list of object identifiers
		using ObjIDList = std::unordered_set<ObjID>;
			///A list of table identifiers
		using TableIDList = std::unordered_set<TableID>;
			///Outline structure of the database (pairs a table identifier with an array of object identifiers for the table content)
		using Outline = std::vector<std::pair<TableID, ObjIDList>>;
		
		// MARK: - Constructors
		
		/*!
		 Destructor
		 */
		virtual ~DBaseEngine() {}
		
		// MARK: - Functions (const)
		
		/*!
		 Get the available dbase tables
		 @param targetType An optional filtr for table type/group to retrieve
		 @return A list of available tables
		 */
		virtual TableIDList getTables(std::optional<TableType> targetType) const { return {}; }	//Most dbases are a single table - exceptions override
		/*!
		 Find a filtered list of objects
		 @param filter The object filter (nullptr = find all objects)
		 @param subset A subset of the database content to search (specified by record ID)
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return A list containing IDs of found elements (empty if none found)
		 */
		virtual ObjIDList findObjects(const Filter* filter = nullptr, const ObjIDList& subset = {}, std::optional<TableID> tableID = std::nullopt,
									  std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get an object by index
		 @param objID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested object (nullptr on failure)
		 */
		virtual std::unique_ptr<Obj> getObject(const ObjID& objID, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get an object in a transportable form, e.g. packaged for serialisation
		 @param objID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested wrapped cargo (nullptr on failure)
		 */
		virtual active::serialise::Cargo::Unique getObjectCargo(const ObjID& objID, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The requested objects (nullptr on failure)
		 */
		virtual active::container::Vector<Obj> getObjects(std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get a filtered list of objects
		 @param filter The object filter
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The filtered objects (nullptr on failure)
		 */
		virtual active::container::Vector<Obj> getObjects(const Filter& filter, std::optional<TableID> tableID = std::nullopt,
														  std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Write an object to the database
		 @param object The object to write
		 @param objID The object ID (globally unique)
		 @param objDocID The object document-specific ID (unique within a specific document - nullopt if not document-bound)
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 */
		virtual void write(Obj& object, const ObjID& objID, std::optional<ObjID> objDocID = std::nullopt,
						   std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Erase an object by index
		 @param objID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @throw Exception thrown on error
		 */
		virtual void erase(const ObjID& objID, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Erase all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @throw Exception thrown on error
		 */
		virtual void erase(std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get the database outline
		 @return The database outline
		 */
		virtual Outline getOutline() const = 0;
	};
	
}

#endif	//ACTIVE_DATABASE_DBASE_ENGINE
