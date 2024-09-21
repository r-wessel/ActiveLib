#ifndef ACTIVE_DATABASE_RECORD_CACHE
#define ACTIVE_DATABASE_RECORD_CACHE

#include "Active/Container/Map.h"
#include "Active/Database/Concepts.h"
#include "Active/Database/Content/Record.h"
#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"

#include <algorithm>
#include <ranges>

namespace active::database {
	
	namespace cache {
		
			///Record serialisation fields
		enum FieldIndex {
			object,
		};
		
		
		/*!
			Get the identity of a specified field
			@param index the field index
			@return The field identity
		*/
		const serialise::Identity& getIdentity(FieldIndex index);
	}

		///Concept for the ability to store objects in a document
	template<typename Obj, typename ObjWrapper, typename ObjID>
	concept IsRecordType = std::is_base_of_v<Record<ObjID>, Obj> && (CanWrap<Obj, ObjWrapper> || FlatType<Obj, ObjWrapper>);

	/*!
	 Interface for a in-memory record cache indexed by the primary key
	 
	 This is primarily intended to work as a cache for long-term storage that doesn't facilitate operations on individual records. Subclassed from
	 Record for 2 purposes:
	 1. To allow the storage to be individually tracked/indexed/stored as a record (independent of its content)
	 2. To support the edited/created characteristics of a record (used for record merging in collaborative workflows)
	 @tparam Obj Interface for the stored record
	 @tparam ObjID The record identifier type (the primary index key)
	 */
	template<typename Obj, typename ObjWrapper, typename ObjID = utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	class RecordCache : public Record<ObjID>, private container::Map<ObjID, Obj> {
	public:
		
		// MARK: - Types
		
		using base = container::Map<ObjID, Obj>;
			///Unary predicate for filtering records
		using Filter = std::function<bool(const Obj&)>;
		
		// MARK: - Constructors
		
		using base::base;
		
		/*!
		 Destructor
		 */
		virtual ~RecordCache() {}
		/*!
			Record cloning
			@return A clone of this record
		*/
		virtual RecordCache* clonePtr() const override { return new RecordCache(*this); };
		
		// MARK: - Functions (const)
		
		/*!
		 Read a record by index
		 @param objID The record ID
		 @return The requested record (nullptr on failure). NB: The returned record is a clone of the original in storage
		 */
		virtual std::unique_ptr<Obj> read(const ObjID& objID) const;
		/*!
		 Read all records
		 @return The requested records (nullptr on failure). NB: The returned records are cloned from storage
		 */
		virtual active::container::Vector<Obj> read() const;
		/*!
		 Read a filtered list of records
		 @param filter The record filter
		 @return The filtered records (nullptr on failure). NB: The returned records are cloned from storage
		 */
		virtual active::container::Vector<Obj> read(const Filter& filter) const;
		/*!
		 Get the cached object keys
		 @return An array containing the cache keys
		 */
		auto keys() const { return base::keys(); }
		
		
		/*!
		 Write a record to storage
		 @param record The record to write (adds if new, or overwrites any record with the same ID)
		 */
		virtual void write(const Obj& record) { base::emplace(record.getID(), clone(record)); }
		/*!
		 Erase an record by index
		 @param objID The record ID
		 @throw Exception thrown on error
		 */
		virtual void erase(const ObjID& objID) { base::erase(objID); }
		/*!
		 Erase all records
		 @throw Exception thrown on error
		 */
		virtual void erase() { base::clear(); }
		/*!
		 Merge another store into this (on the basis of created/edited record and cache time-stamps, most recent data wins)
		 @param store The store to merge (NB: the objects in this container are assumed to be expendable)
		 @return True if the record was accepted
		 */
		virtual RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>& merge(RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>&& store);
		
		// MARK: - Serialisation
		
		/*!
			Fill an inventory with the package items
			@param inventory The inventory to receive the package items
			@return True if the package has added items to the inventory
		*/
		bool fillInventory(active::serialise::Inventory& inventory) const override;
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		serialise::Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override;
		/*!
			Set to the default package content
		*/
		void setDefault() override { base::clear(); }
		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
			@return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		*/
		bool insert(serialise::Cargo::Unique&& cargo, const serialise::Inventory::Item& item) override;
	};

	
	/*--------------------------------------------------------------------
		Read a record by index
	 
		objID: The record ID
	 
		return: The requested record (nullptr on failure). NB: The returned record is a clone of the original in storage
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	typename std::unique_ptr<Obj> RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::read(const ObjID& objID) const {
		if (auto iter = base::find(objID); iter != base::end())
			return clone(*iter->second);
		return nullptr;
	} //RecordCache::<Obj, ObjID>::read
	
	
	/*--------------------------------------------------------------------
		Read all records
	 
		return: The requested records (nullptr on failure). NB: The returned records are cloned from storage
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	typename active::container::Vector<Obj> RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::read() const {
		active::container::Vector<Obj> result;
		std::for_each(base::begin(), base::end(), [&result](const auto& item){ result.emplace_back(clone(*item.second)); });
		return result;
	} //RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::read
	
	
	/*--------------------------------------------------------------------
		Read a filtered list of records
	 
		filter: The record filter
	 
		return: The filtered records (nullptr on failure). NB: The returned records are cloned from storage
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	typename active::container::Vector<Obj> RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::read(const Filter& filter) const {
		active::container::Vector<Obj> result;
		std::for_each(base::begin(), base::end(), [&](const auto& item){
			if (filter(*item.second))
				result.emplace_back(clone(*item.second));
		});
		return result;
	} //RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::read
	
	
	/*--------------------------------------------------------------------
		Merge another store into this (on the basis of created/edited record and cache time-stamps, most recent data wins)
	 
		store: The store to merge (NB: the objects in this container are assumed to be expendable)
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>& RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::merge(RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>&& store) {
		auto startKeys = base::keys();
		auto endKeys = store.keys();
		std::set<ObjID> startSet{startKeys.begin(), startKeys.end()},
				endSet{endKeys.begin(), endKeys.end()};
			//Find (potentially) edited records
		std::set<ObjID> edited;
		std::set_intersection(startSet.begin(), startSet.end(), endSet.begin(), endSet.end(), std::inserter(edited, edited.begin()));
			//Find (potentially) added records
		std::set<ObjID> added;
		std::set_difference(endSet.begin(), endSet.end(), edited.begin(), edited.end(), std::inserter(added, added.begin()));
			//Find (potentially) removed records
		std::set<ObjID> removed;
		std::set_difference(startSet.begin(), startSet.end(), edited.begin(), edited.end(), std::inserter(removed, removed.begin()));
			//Compare edited records to retain the most recent version in this cache
		std::for_each(edited.begin(), edited.end(), [this, &store](const auto& key) {
			auto& mine = (*this)[key];
			auto& theirs = store[key];
				//If their record is more recent than mine, swap in the new data
			if (theirs->getEdited() > mine->getEdited())
				mine.reset(theirs.release());
		});
			//Merge added records, but only if they haven't been previously deleted from this cache
		std::for_each(added.begin(), added.end(), [this, &store](const auto& key) {
			auto theirs = store.find(key);
				//If this record was created after the last refresh of this data, add it to this
			if (theirs->second->getEdited() > this->getEdited())
				this->emplace(key, store.release(theirs));
		});
			//Erase removed records, but only if they haven't been added in this container
		std::for_each(removed.begin(), removed.end(), [this](const auto& key) {
			auto& mine = (*this)[key];
				//If their record is more recent than mine, swap in the new data
			if (mine->getCreated() < this->getEdited())
				this->erase(key);
			else
				mine->setEdited(active::utility::Time{});	//Mark the time this record has merged
		});
		return *this;
	} //RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::merge
	

	/*--------------------------------------------------------------------
		Fill an inventory with the package items
	 
		inventory: The inventory to receive the package items
	 
		return: True if the package has added items to the inventory
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	bool RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::fillInventory(active::serialise::Inventory& inventory) const {
		using enum serialise::Entry::Type;
		using enum cache::FieldIndex;
		inventory.merge(serialise::Inventory{
			{
				{ cache::getIdentity(object), object, base::size(), std::nullopt },
			},
		}.withType(&typeid(RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>)));
		Record<ObjID>::fillInventory(inventory);
		return true;
	} //RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::fillInventory
	
	
	/*--------------------------------------------------------------------
		Get the specified cargo
	 
		item: The inventory item to retrieve
	 
		return: The requested cargo (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	serialise::Cargo::Unique RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::getCargo(const active::serialise::Inventory::Item& item) const {
		if (item.ownerType != &typeid(RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>))
			return Record<ObjID>::getCargo(item);
		using namespace active::serialise;
		using enum cache::FieldIndex;
		switch (item.index) {
			case object:
				if (item.available < base::size()) {
					auto iter = base::begin();
					std::advance(iter, item.available);
					return std::make_unique<active::serialise::CargoHold<ObjWrapper, Obj>>(*iter->second);
				}
				if constexpr (FlatType<Obj, ObjWrapper>)
					return std::make_unique<active::serialise::CargoHold<active::serialise::PackageWrap, Obj>>();
				else
					return std::make_unique<active::serialise::CargoHold<ObjWrapper, Obj>>();
			default:
				return nullptr;	//Requested an unknown index
		}
	} //RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::getCargo

	
	/*--------------------------------------------------------------------
		Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
	 
		cargo: The cargo to insert
		item: The inventory item linked with the cargo
	 
		return: True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
	  --------------------------------------------------------------------*/
	template<typename Obj, typename ObjWrapper, typename ObjID, typename DBaseID, typename TableID>
	requires IsRecordType<Obj, ObjWrapper, ObjID>
	bool RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::insert(serialise::Cargo::Unique&& cargo, const serialise::Inventory::Item& item) {
		if (item.ownerType != &typeid(RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>))
			return true;
		using namespace active::serialise;
		using enum cache::FieldIndex;
		switch (item.index) {
			case object:
				auto source = dynamic_cast<active::serialise::CargoHold<ObjWrapper, Obj>*>(cargo.get());
				if (source != nullptr) {
					auto incoming = source->releaseIncoming();
					if (auto record = dynamic_cast<Obj*>(incoming.get()); record != nullptr) {
						incoming.release();
						base::emplace(record->getID(), std::unique_ptr<Obj>{record});
					}
				}
				break;
		}
		return true;
	} //RecordCache<Obj, ObjWrapper, ObjID, DBaseID, TableID>::insert
	
}

#endif	//ACTIVE_DATABASE_RECORD_CACHE
