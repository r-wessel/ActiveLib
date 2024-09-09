#ifndef ACTIVE_DATABASE_SQLITE_ENGINE
#define ACTIVE_DATABASE_SQLITE_ENGINE

#include "Active/Database/Storage/DBaseEngine.h"
#include "Active/Database/Storage/SQLite/SQLiteCore.h"
#include "Active/File/Path.h"
#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/Transport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/Guid.h"
#include "SQLite/sqlite3.h"

namespace active::database {
	
	/*!
	 An SQLite database engine template
	 
	 @tparam Obj Interface for the stored object. NB: This can be a base class for an object hierarchy, not necessarily a concrete class
	 @tparam Transport The serialisation transport mechanism for objects
	 @tparam DocID The document identifier type, e.g. Guid. The type is arbitrary if a document structure is not employed
	 @tparam ObjID The object identifier type, e.g. Guid
	 */
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID = utility::Guid, typename ObjID = utility::Guid>
	requires std::is_base_of_v<serialise::Cargo, Obj> &&
			std::is_base_of_v<serialise::Cargo, ObjWrapper> &&
			std::is_base_of_v<serialise::Transport, Transport>
	class SQLiteEngine : public SQLiteCore, public DBaseEngine<Obj, ObjID, DocID, utility::String>  {
	public:
		
		// MARK: - Types
		
		using base = DBaseEngine<Obj, ObjID, DocID, utility::String>;
		using Filter = base::Filter;
		
		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param path Path to the SQLite dbase
		 @param schema Database schema
		 */
		SQLiteEngine(const file::Path& path, SQLiteSchema&& schema) : SQLiteCore{path, std::move(schema)} {}
		/*!
			Object cloning
			@return A clone of this object
		*/
		SQLiteEngine* clonePtr() const override { return new SQLiteEngine{*this}; }
		
		// MARK: - Functions (const)
		
		/*!
		 Get an object by index
		 @param ID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested object (nullptr on failure)
		 */
		std::unique_ptr<Obj> getObject(const ObjID& ID, utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
		/*!
		 Get all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The requested objects (nullptr on failure)
		 */
		container::Vector<Obj> getObjects(utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
		/*!
		 Get a filtered list of objects
		 @param filter The object filter
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The filtered objects (nullptr on failure)
		 */
		active::container::Vector<Obj> getObjects(const Filter& filter, utility::String::Option tableID = std::nullopt,
														  std::optional<DocID> documentID = std::nullopt) const override;
		/*!
		 Erase an object by index
		 @param ID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @throw Exception thrown on SQL error
		 */
		void erase(const ObjID& ID, utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
		/*!
		 Erase all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @throw Exception thrown on SQL error
		 */
		void erase(utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
		/*!
		 Run a (multi-step) transaction and collect the results
		 @param transaction The transaction to run
		 @param table The target table
		 @param filter Filter for ecollected objects
		 @return The collected objects
		 @throw Exception thrown on SQL error
		 */
		container::Vector<Obj> runTransaction(SQLiteCore::Transaction& transaction, const TableSchema<utility::String>& table,
											  const Filter* filter = nullptr) const;
		
	private:
		/*!
		 Get a table from the schema
		 @param tableID Optional table ID (defaults to the first table)
		 @return An iterator pointing to the requested table
		 */
		SQLiteSchema::const_iterator getTable(utility::String::Option tableID) const;
	};
	
	
	
	/*--------------------------------------------------------------------
		Get an object by index
	 
		index: The object index
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (when the object is bound to a specific document)
	 
		return: The requested object (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	std::unique_ptr<Obj> SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObject(const ObjID& ID, utility::String::Option tableID,
																						   std::optional<DocID> documentID)  const {
		auto table = getTable(tableID);
		auto keyFieldIndex = documentID && table->documentIndex ? *table->documentIndex : table->globalIndex;
		auto transaction = makeTransaction("SELECT * FROM " + table->ID + " WHERE " + (*table)[keyFieldIndex]->name() + " = " + ID + ";");
		auto result = runTransaction(transaction, *table);
		return result.empty() ? nullptr : result.release(result.begin());
	} //SQLiteEngine<Obj, Transport, DocID, ObjID>::getObject
	
	
	/*--------------------------------------------------------------------
		Get all objects
	 
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (filter for this document only - nullopt = all objects)
	 
		return: The requested objects (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	container::Vector<Obj> SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObjects(utility::String::Option tableID,
																											   std::optional<DocID> documentID) const {
		auto table = getTable(tableID);
		auto transaction = makeTransaction("SELECT * FROM " + table->ID + ";");
		return runTransaction(transaction, *table);
	} //SQLiteEngine<Obj, Transport, DocID, ObjID>::getObjects
	
	
	/*--------------------------------------------------------------------
		Get all objects
	 
		filter: The object filter
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (filter for this document only - nullopt = all objects)
	 
		return: The requested objects (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	container::Vector<Obj> SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObjects(const Filter& filter, utility::String::Option tableID,
																							  std::optional<DocID> documentID) const {
		auto table = getTable(tableID);
		auto transaction = makeTransaction("SELECT * FROM " + table->ID + ";");
		return runTransaction(transaction, *table, &filter);
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObjects
	
	
	/*--------------------------------------------------------------------
		Erase an object by index
	 
		objID: The object ID
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (when the object is bound to a specific document)
	 
		return: True if the object was successfully erased
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	void SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::erase(const ObjID& ID, utility::String::Option tableID,
																	   std::optional<DocID> documentID) const {
		auto table = getTable(tableID);
		auto keyFieldIndex = documentID && table->documentIndex ? *table->documentIndex : table->globalIndex;
		makeTransaction("DELETE FROM " + table->ID + " WHERE " + (*table)[keyFieldIndex]->name() + " = " + ID + ";").execute();
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::erase
	
	
	/*--------------------------------------------------------------------
		Erase all objects
	 
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (filter for this document only - nullopt = all objects)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	void SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::erase(utility::String::Option tableID, std::optional<DocID> documentID) const {
		auto table = getTable(tableID);
		makeTransaction("SELECT * FROM " + table->ID + ";").execute();;
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::erase

	
	/*--------------------------------------------------------------------
		Run a transaction and collect the results
	 
		transaction: The transaction to run
		table: The target table
		filter: Filter for ecollected objects
	 
		return: The collected objects
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	container::Vector<Obj> SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::runTransaction(SQLiteCore::Transaction& transaction,
																								  const TableSchema<utility::String>& table,
																								  const Filter* filter) const {
		container::Vector<Obj> result;
		do {
				//Get the next row in the table
			auto row = ++transaction;
			if (!row)
				break;
			auto content = table.getContent(*row);
			if (content == nullptr)
				throw std::system_error(makeError(Status::contentNotFound));
			serialise::Cargo::Unique wrapper;
			if constexpr (std::is_same_v<ObjWrapper, Obj>)
				wrapper = std::make_unique<serialise::CargoHold<serialise::PackageWrap, Obj>>();
			else
				wrapper = std::make_unique<ObjWrapper>();
			Transport{}.receive(std::forward<serialise::Cargo&&>(*wrapper), serialise::Identity{}, content->operator utility::String());
			if constexpr (std::is_same_v<ObjWrapper, Obj>)
				result.emplace_back(std::make_unique<Obj>(dynamic_cast<serialise::CargoHold<serialise::PackageWrap, Obj>*>(wrapper.get())->get()));
			else
				result.emplace(wrapper.release());
		} while (transaction);
		return result;
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::runTransaction
	
	
	/*--------------------------------------------------------------------
		Get a table from the schema
	 
		tableID: Optional table ID (defaults to the first table)
	 
		return: An iterator pointing to the requested table
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	SQLiteSchema::const_iterator SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getTable(utility::String::Option tableID) const {
		const auto& schema = getSchema();
		SQLiteSchema::const_iterator table = schema.end();
			//Use the first table if none specified
		if (!tableID)
			table = schema.begin();
		else
			table = std::find_if(schema.begin(), schema.end(), [&tableID](const auto& table){ return table.ID == *tableID; });
		if (table == schema.end())
			throw std::system_error(makeError(Status::tableNotFound));
		return table;
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getTable
	
}

#endif	//ACTIVE_DATABASE_SQLITE_ENGINE
