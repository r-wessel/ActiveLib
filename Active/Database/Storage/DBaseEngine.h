#ifndef ACTIVE_DATABASE_DBASE_ENGINE
#define ACTIVE_DATABASE_DBASE_ENGINE

#include "Active/Container/Vector.h"
#include "Active/Utility/Cloner.h"
#include "Active/Utility/Guid.h"

namespace active::database {
	
	/*!
	 Interface for a database engine
	 
	 The role of an engine is to manage object content including storage, retrieval, deletion etc
	 @tparam Obj Interface for the stored object
	 @tparam ObjID The object identifier type
	 @tparam DocID The document identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename Obj, typename ObjID = active::utility::Guid, typename DocID = active::utility::Guid, typename TableID = active::utility::Guid>
	class DBaseEngine : public utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Unary predicate for filtering objects
		using Filter = std::function<bool(const Obj&)>;
		
		// MARK: - Constructors
		
		/*!
		 Destructor
		 */
		virtual ~DBaseEngine() {}
		/*!
			Object cloning
			@return A clone of this object
		*/
		virtual DBaseEngine* clonePtr() const = 0;
		
		// MARK: - Functions (const)
		
		/*!
		 Get an object by index
		 @param objID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return: The requested object (nullptr on failure)
		 */
		virtual std::unique_ptr<Obj> getObject(const ObjID& objID, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return: The requested objects (nullptr on failure)
		 */
		virtual active::container::Vector<Obj> getObjects(std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const = 0;
		/*!
		 Get a filtered list of objects
		 @param filter The object filter
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return: The filtered objects (nullptr on failure)
		 */
		virtual active::container::Vector<Obj> getObjects(const Filter& filter, std::optional<TableID> tableID = std::nullopt,
														  std::optional<DocID> documentID = std::nullopt) const = 0;
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
	};
	
}

#endif	//ACTIVE_DATABASE_DBASE_ENGINE
