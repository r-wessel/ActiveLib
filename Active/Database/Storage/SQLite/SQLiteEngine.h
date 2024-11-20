#ifndef ACTIVE_DATABASE_SQLITE_ENGINE
#define ACTIVE_DATABASE_SQLITE_ENGINE

#include "Active/Database/Concepts.h"
#include "Active/Database/Storage/DBaseEngine.h"
#include "Active/Database/Storage/SQLite/SQLiteCore.h"
#include "Active/File/Path.h"
#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Concepts.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/Transport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/Guid.h"
#include "SQLite/sqlite3.h"

namespace active::database {
	
		///Concept for the ability to store objects in an SQLite database
	template<typename Obj, typename ObjWrapper, typename Transport>
	concept SQLiteStorable = (CanWrap<Obj, ObjWrapper> || FlatType<Obj, ObjWrapper>) &&
			serialise::IsCargo<Obj> &&
			std::is_base_of_v<serialise::Cargo, ObjWrapper> &&
			std::is_base_of_v<serialise::Transport, Transport>;

	/*!
	 An SQLite database engine template
	 
	 @tparam Obj Interface for the stored object. NB: This can be a base class for an object hierarchy, not necessarily a concrete class
	 @tparam ObjWrapper Wrapper type for (de)serialising objects
	 @tparam Transport The serialisation transport mechanism for objects
	 @tparam ObjID The object identifier type, e.g. Guid
	 @tparam DocID The document identifier type, e.g. Guid. The type is arbitrary if a document structure is not employed
	 */
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID = active::utility::Guid, typename ObjID = active::utility::Guid>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	class SQLiteEngine : public SQLiteCore, public DBaseEngine<Obj, ObjID, DocID, utility::String>  {
	public:
		
		// MARK: - Types
		
		using base = DBaseEngine<Obj, ObjID, DocID, utility::String>;
		using Filter = base::Filter;
		using Outline = base::Outline;
		using ObjIDList = base::ObjIDList;

		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param path Path to the SQLite dbase
		 @param schema Database schema
		 */
		SQLiteEngine(const file::Path& path, SQLiteSchema&& schema) : SQLiteCore{path, std::move(schema)} {}
		
		// MARK: - Functions (const)
		
		/*!
		 Find a filtered list of objects
		 @param filter The object filter (nullptr = find all objects)
		 @param subset A subset of the database content to search (specified by record ID)
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return A list containing IDs of found elements (empty if none found)
		 */
		virtual ObjIDList findObjects(const Filter* filter = nullptr, const ObjIDList& subset = {},
									  std::optional<utility::String> tableID = std::nullopt,
									  std::optional<DocID> documentID = std::nullopt) const override { return {}; }	//Implement when required
		/*!
		 Get an object by index
		 @param ID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested object (nullptr on failure)
		 */
		std::unique_ptr<Obj> getObject(const ObjID& ID, utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
		/*!
		 Get an object in a transportable form, e.g. packaged for serialisation
		 @param ID The object ID
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @return The requested wrapped cargo (nullptr on failure)
		 */
		active::serialise::Cargo::Unique getObjectCargo(const ObjID& ID, utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
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
		 Write an object to the database
		 @param object The object to write
		 @param objID The object ID
		 @param objDocID The object document-specific ID (unique within a specific document - nullopt if not document-bound)
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 */
		virtual void write(Obj& object, const ObjID& objID, std::optional<ObjID> objDocID = std::nullopt,
						   utility::String::Option tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const override;
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
		 Get the database outline
		 @return The database outline
		 */
		Outline getOutline() const override;
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
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	std::unique_ptr<Obj> SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObject(const ObjID& ID, utility::String::Option tableID,
																						   std::optional<DocID> documentID)  const {
		auto table = getTable(tableID);
		auto keyFieldIndex = documentID && table->documentIndex ? *table->documentIndex : table->globalIndex;
		auto transaction = makeTransaction("SELECT * FROM " + table->ID + " WHERE " + (*table)[keyFieldIndex]->name() + " = " + ID + ";");
		auto result = runTransaction(transaction, *table);
		return result.empty() ? nullptr : result.release(result.begin());
	} //SQLiteEngine<Obj, Transport, DocID, ObjID>::getObject
		
	
	/*--------------------------------------------------------------------
		Get an object in a transportable form, e.g. packaged for serialisation
	 
		index: The object index
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (when the object is bound to a specific document)
	 
		return: The requested wrapped cargo (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	active::serialise::Cargo::Unique SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObjectCargo(const ObjID& ID, utility::String::Option tableID,
																						   std::optional<DocID> documentID)  const {
		if (auto object = getObject(ID, tableID, documentID); object)
			return std::make_unique<active::serialise::CargoHold<ObjWrapper, Obj>>(std::move(object));
		return nullptr;
	} //SQLiteEngine<Obj, Transport, DocID, ObjID>::getObject
	
	
	/*--------------------------------------------------------------------
		Get all objects
	 
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (filter for this document only - nullopt = all objects)
	 
		return: The requested objects (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
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
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	container::Vector<Obj> SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObjects(const Filter& filter, utility::String::Option tableID,
																							  std::optional<DocID> documentID) const {
		auto table = getTable(tableID);
		auto transaction = makeTransaction("SELECT * FROM " + table->ID + ";");
		return runTransaction(transaction, *table, &filter);
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getObjects
	
	
	/*--------------------------------------------------------------------
		Write an object to the database
	 
		object: The object to write
		objID: The object ID
		objDocID: The object document-specific ID (unique within a specific document - nullopt if not document-bound)
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (when the object is bound to a specific document)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	void SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::write(Obj& object,
																	   const ObjID& objID, std::optional<ObjID> objDocID,
																	   utility::String::Option tableID, std::optional<DocID> documentID) const {
		utility::String content;
		Transport{}.send(serialise::PackageWrap(object), serialise::Identity{}, content);
		auto table = getTable(tableID);
		bool isDocIndexed = objDocID && table->documentIndex;
		utility::String statement{"INSERT INTO " + table->ID + " (" + (*table)[table->globalIndex]->name() + ", "};
		if (isDocIndexed)
			statement += (*table)[*table->documentIndex]->name() + ", ";
		statement += (*table)[table->contentIndex]->name() + ") VALUES (" + toSQLiteString(objID) + ", ";
		if (isDocIndexed)
			statement += toSQLiteString(*objDocID) + ", ";
		statement += toSQLiteString(content.data()) + ");";
		makeTransaction(statement).execute();
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::write
	
	
	/*--------------------------------------------------------------------
		Erase an object by index
	 
		objID: The object ID
		tableID: Optional table ID (defaults to the first table)
		documentID: Optional document ID (when the object is bound to a specific document)
	 
		return: True if the object was successfully erased
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
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
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	void SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::erase(utility::String::Option tableID, std::optional<DocID> documentID) const {
		auto table = getTable(tableID);
		makeTransaction("SELECT * FROM " + table->ID + ";").execute();
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::erase

	
	/*--------------------------------------------------------------------
		Get the database outline
	 
		return: The database outline
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
	SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::Outline SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getOutline() const {
		Outline result;
			//Iterate through each database table
		for (const auto& table : getSchema()) {
				//And extract the index column from the table
			auto indexField = table[table.globalIndex]->name();
			auto transaction = makeTransaction("SELECT " + indexField + " FROM " + table.ID + ";");
			std::unordered_set<ObjID> tableIDs;
			do {
				auto row = ++transaction;
				if (!row)
					break;
				if (auto idSetting = dynamic_cast<const setting::ValueSetting*>((*row)[0].get()); idSetting != nullptr) {
					ObjID temp = *idSetting;
					tableIDs.insert(temp);
				}
			} while (transaction);
				//Add the table name and index column to the outline
			result.emplace_back(std::make_pair(table.ID, tableIDs));
		}
		return result;
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::getOutline
	
	
	
	/*--------------------------------------------------------------------
		Run a transaction and collect the results
	 
		transaction: The transaction to run
		table: The target table
		filter: Filter for ecollected objects
	 
		return: The collected objects
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
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
				wrapper = std::make_unique<serialise::CargoHold<ObjWrapper, Obj>>();
			Transport{}.receive(std::forward<serialise::Cargo&&>(*wrapper), serialise::Identity{}, content->operator utility::String());
			if constexpr (std::is_same_v<ObjWrapper, Obj>)
				result.emplace_back(std::make_unique<Obj>(dynamic_cast<serialise::CargoHold<serialise::PackageWrap, Obj>*>(wrapper.get())->get()));
			else {
				if (auto incoming = wrapper.release(); incoming)
					result.emplace(incoming);
			}
		} while (transaction);
		return result;
	} //SQLiteEngine<Obj, ObjWrapper, Transport, DocID, ObjID>::runTransaction
	
	
	/*--------------------------------------------------------------------
		Get a table from the schema
	 
		tableID: Optional table ID (defaults to the first table)
	 
		return: An iterator pointing to the requested table
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID, typename ObjID>
	requires SQLiteStorable<Obj, ObjWrapper, Transport>
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
