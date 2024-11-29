#ifndef ACTIVE_DATABASE_STORAGE
#define ACTIVE_DATABASE_STORAGE

#include "Active/Container/Vector.h"
#include "Active/Database/Content/Record.h"
#include "Active/Database/Storage/DBaseEngine.h"
#include "Active/Database/Storage/DBaseSchema.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/Package/Package.h"
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
	 @tparam TableType An identifier for a table type or group
	 */
	template<typename Obj, typename Transport, typename DocID = active::utility::Guid, typename ObjID = active::utility::Guid,
			typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid, typename TableType = int32_t>
	class Storage {
	public:
		
		// MARK: - Types
		
			///The engine driving dbase operations. NB: This is independent of the Storage wrapper, i.e. one wrapper could employ multiple engines
		using Engine = DBaseEngine<Obj, ObjID, DocID, TableID, TableType>;
			///The database schema, including tables
		using DBaseSchema = DBaseSchema<DBaseID, TableID>;
			///Tables within the database, including a full description of member fields (names/types) and primary index/content fields
		using TableSchema = DBaseSchema::TableSchemaType;
			///Unary predicate for filtering records
		using Filter = std::function<bool(const Obj&)>;
			///A serialisation wrapper for a Storage container
		class Wrapper : public active::serialise::Package {
		public:
				///A serialisation wrapper for a Storage table
			class Table : public active::serialise::Package {
			public:
				Table(const Storage& storage, const std::pair<TableID, std::unordered_set<ObjID>>& table) : m_storage{storage}, m_table(table) {}
				bool fillInventory(active::serialise::Inventory& inventory) const override;
				active::serialise::Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override;
			private:
				const Storage& m_storage;
				const std::pair<TableID, std::unordered_set<ObjID>>& m_table;
			};
			Wrapper(const Storage& storage) : m_storage{storage}, m_outline(storage.m_engine->getOutline()) {}
			bool fillInventory(active::serialise::Inventory& inventory) const override;
			active::serialise::Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override;
		private:
			const Storage& m_storage;
			Engine::Outline m_outline;
		};
		
		friend class Wrapper;
		
		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param engine The database engine
		 */
		Storage(std::shared_ptr<Engine> engine) : m_engine{engine} {}
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		Storage(const Storage& source) : m_engine{std::move(clone(*source.m_engine))} {}
		/*!
		 Destructor
		 */
		~Storage() {}
		
		// MARK: - Functions (const)
		
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
		/*!
		 Get all objects from a database table
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (filter for this document only - nullopt = all objects)
		 @return The requested objects (nullptr on failure)
		 */
		active::container::Vector<Obj> getObjects(const Filter& filter, std::optional<TableID> tableID = std::nullopt,
												  std::optional<DocID> documentID = std::nullopt) const {
			return m_engine->getObjects(tableID, documentID);
		}
		/*!
		 Write an object to the database
		 @param object The object to write
		 @param objID The object ID
		 @param objDocID The object document-specific ID (unique within a specific document - nullopt if not document-bound)
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 */
		void write(Obj& object, const ObjID& objID, std::optional<ObjID> objDocID = std::nullopt,
				   std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			m_engine->write(object, objID, objDocID, tableID, documentID);
		}
		/*!
		 Write a record-based object to the database
		 @param record The record-based object to writes
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 */
		template<typename T>
		void write(T& record, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			m_engine->write(record, ObjID{record.getGlobalID()}, ObjID{record.getID()}, tableID, documentID);
		}
		/*!
		 Erase an object by index
		 @param index The object index
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @throw Exception thrown on error
		 */
		void erase(const ObjID& index, std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			m_engine->erase(index, tableID, documentID);
		}
		/*!
		 Erase all objects
		 @param tableID Optional table ID (defaults to the first table)
		 @param documentID Optional document ID (when the object is bound to a specific document)
		 @throw Exception thrown on error
		 */
		void erase(std::optional<TableID> tableID = std::nullopt, std::optional<DocID> documentID = std::nullopt) const {
			m_engine->erase(tableID, documentID);
		}
		/*!
		 Get a serialisation wrapper for the storage container (to serialise the content)
		 @return A serialisation wrapper for the storage container
		 */
		std::unique_ptr<active::serialise::Cargo> wrapper() const { return std::make_unique<Wrapper>(*this); }

	private:
		std::shared_ptr<Engine> m_engine;
	};
	

	/*--------------------------------------------------------------------
		Fill an inventory with the cargo items
	 
		inventory: The inventory to receive the cargo items
	 
		return: True if items have been added to the inventory
	  --------------------------------------------------------------------*/
	template<typename Obj, typename Transport, typename DocID, typename ObjID, typename DBaseID, typename TableID, typename TableType>
	bool Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID, TableType>::Wrapper::fillInventory(active::serialise::Inventory& inventory) const {
		using enum active::serialise::Entry::Type;
			//Each table becomes a serialisation object using the table name
		int16_t index = 0;
		for (const auto& item : m_outline)
			inventory.merge({ item.first, index++, element });
		return true;
	} //Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID>::Wrapper::fillInventory
	
	
	/*--------------------------------------------------------------------
		Get the specified cargo
	 
		item: The inventory item to retrieve
	 
		return: The requested cargo (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename Transport, typename DocID, typename ObjID, typename DBaseID, typename TableID, typename TableType>
	active::serialise::Cargo::Unique Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID, TableType>::Wrapper::getCargo(const active::serialise::Inventory::Item& item) const {
		if (item.available >= m_outline.size())
			return nullptr;
		auto source = m_outline.begin();
		std::advance(source, item.available);
		return std::make_unique<Wrapper::Table>(m_storage, *source);
	} //Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID>::Wrapper::getCargo
	

	/*--------------------------------------------------------------------
		Fill an inventory with the cargo items
	 
		inventory: The inventory to receive the cargo items
	 
		return: True if items have been added to the inventory
	  --------------------------------------------------------------------*/
	template<typename Obj, typename Transport, typename DocID, typename ObjID, typename DBaseID, typename TableID, typename TableType>
	bool Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID, TableType>::Wrapper::Table::fillInventory(active::serialise::Inventory& inventory) const {
		using enum active::serialise::Entry::Type;
		inventory.merge({ m_table.first, 0, m_table.second.size(), std::nullopt });
		return true;
	} //Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID>::Wrapper::fillInventory
	
	
	/*--------------------------------------------------------------------
		Get the specified cargo
	 
		item: The inventory item to retrieve
	 
		return: The requested cargo (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename Transport, typename DocID, typename ObjID, typename DBaseID, typename TableID, typename TableType>
	active::serialise::Cargo::Unique Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID, TableType>::Wrapper::Table::getCargo(const active::serialise::Inventory::Item& item) const {
		if (item.available >= m_table.second.size())
			return nullptr;
		auto iter = m_table.second.begin();
		std::advance(iter, item.available);
		if (auto object = m_storage.m_engine->getObjectCargo(*iter); object)
			return std::move(object);
		return nullptr;
	} //Storage<Obj, Transport, DocID, ObjID, DBaseID, TableID>::Wrapper::getCargo
	
}

#endif	//ACTIVE_DATABASE_STORAGE
