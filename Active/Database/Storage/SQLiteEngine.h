#ifndef ACTIVE_DATABASE_SQLITE_ENGINE
#define ACTIVE_DATABASE_SQLITE_ENGINE

#include "Active/Database/Storage/DBaseEngine.h"
#include "Active/Database/Storage/SQLiteCore.h"
#include "Active/File/Path.h"
#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Package/PackageWrap.h"
#include "Active/Serialise/Transport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/Guid.h"
#include "SQLite/sqlite3.h"

namespace active::database {
	
	/*!
	 An SQLite database engine template
	 
	 @tparam Obj Interface for the stored object
	 @tparam Transport The transport mechanism for objects
	 @tparam DocID The document identifier type
	 @tparam ObjID The object identifier type
	 */
	template<typename Obj, typename ObjWrapper, typename Transport, typename DocID = utility::Guid, typename ObjID = utility::Guid>
	requires std::is_base_of_v<serialise::Cargo, Obj> && std::is_base_of_v<serialise::Cargo, ObjWrapper> && std::is_base_of_v<serialise::Transport, Transport>
	class SQLiteEngine : public SQLiteCore, public DBaseEngine<Obj, ObjID, DocID, utility::String>  {
	public:

		/*!
		 Constructor
		 @param path Path to the SQLite dbase
		 @param schema Database schema
		 */
		SQLiteEngine(file::Path path, SQLiteSchema&& schema) : SQLiteCore{path, std::move(schema)} {}
		/*!
			Object cloning
			@return A clone of this object
		*/
		virtual SQLiteEngine* clonePtr() const override { return new SQLiteEngine{*this}; }
		
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
		auto keyFieldIndex = documentID && table->documentIndex ? table->documentIndex : table->globalIndex;
		auto something = (*table)[keyFieldIndex];
		auto transaction = makeTransaction("SELECT * FROM " + table->ID + " WHERE " + (*table)[keyFieldIndex]->name + " = " + ID + ";");
		auto row = ++transaction;
		if (!row)
			return nullptr;
		if (row->size() < table->contentIndex)
			throw std::system_error(makeError(Status::contentNotFound));
		serialise::Cargo::Unique wrapper;
		if constexpr (std::is_same_v<ObjWrapper, Obj>)
			wrapper = std::make_unique<serialise::CargoHold<serialise::PackageWrap, Obj>>();
		else
			wrapper = std::make_unique<ObjWrapper>();
		Transport{}.receive(std::forward<serialise::Cargo&&>(*wrapper), serialise::Identity{}, (*row)[table->contentIndex]->string());
		if constexpr (std::is_same_v<ObjWrapper, Obj>)
			return std::make_unique<Obj>(dynamic_cast<serialise::CargoHold<serialise::PackageWrap, Obj>*>(wrapper.get())->get());
		else
			return wrapper.release();
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
		container::Vector<Obj> result;
		do {
			auto row = ++transaction;
			if (!row)
				break;
			if (row->size() < table->contentIndex)
				throw std::system_error(makeError(Status::contentNotFound));
			serialise::Cargo::Unique wrapper;
			if constexpr (std::is_same_v<ObjWrapper, Obj>)
				wrapper = std::make_unique<serialise::CargoHold<serialise::PackageWrap, Obj>>();
			else
				wrapper = std::make_unique<ObjWrapper>();
			Transport{}.receive(std::forward<serialise::Cargo&&>(*wrapper), serialise::Identity{}, (*row)[table->contentIndex]->string());
			if constexpr (std::is_same_v<ObjWrapper, Obj>)
				result.emplace_back(std::make_unique<Obj>(dynamic_cast<serialise::CargoHold<serialise::PackageWrap, Obj>*>(wrapper.get())->get()));
			else
				result.emplace(wrapper.release());
		} while (transaction);
		return result;
	} //SQLiteEngine<Obj, Transport, DocID, ObjID>::getObjects
	
	
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
