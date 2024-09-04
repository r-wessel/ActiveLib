#ifndef ACTIVE_DATABASE_RECORD
#define ACTIVE_DATABASE_RECORD

#include "Active/Database/Identity/Link.h"
#include "Active/Serialise/Package/Package.h"

namespace active::database {

	/*!
	 Base class for any object stored in a database
	 
	 The object has to support some form of portability and be clonable to support storage and database operations. Note that objects may have 2
	 identifiers:
	 - One for the ID within a document. This remains constant in copies when a document is duplicated, and is therefore not globally unique
	 - One as a gloablly unique identifier - this must not remain the same in copies
	 @tparam ObjID The object identifier type
	 @tparam DBaseID The database identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class Record : public active::serialise::Package, public active::utility::Cloner {
	public:

		// MARK: - Types
		
		using Unique = std::unique_ptr<Record>;
			///Shared pointer
		using Shared = std::shared_ptr<Record>;
			///Optional
		using Option = std::optional<Record>;
			///Record index type
		using Index = active::database::Index<ObjID, DBaseID, TableID>;
			///Record link type
		using Link = active::database::Link<ObjID, DBaseID, TableID>;

		// MARK: - Constructors
		
		/*!
		 Constructor
		 @param ID The object document identifier
		 */
		Record(const ObjID& ID) : m_ID{ID} {}
		Record(const Record&) = default;
		/*!
		 Destructor
		 */
		virtual ~Record() {}
		
		/*!
			Object cloning
			@return A clone of this object
		*/
		Record* clonePtr() const override { return new Record{*this}; }
		
		// MARK: - Functions (const)
		
		/*!
		 Get the object document identifier
		 @return The object document ID
		 */
		ObjID getID() const { return m_ID; }
		/*!
		 Get the object global identifier
		 @return The object global ID
		 */
		ObjID getGlobalID() const { return m_globalID; }
		/*!
		 Get the object document index
		 @return The object index
		 */
		virtual Index getIndex() const { return Index{m_ID, m_dbaseID, m_tableID}; }
		/*!
		 Get the object document link
		 @return The object link
		 */
		virtual Link getLink() const { return Link{getIndex()}; }
		
		// MARK: - Functions (mutating)
		
		/*!
		 Set the object document identifier
		 @param ID The object document ID
		 */
		virtual void setID(const ObjID& ID) { m_ID = ID; }
		/*!
		 Set the object global identifier
		 @param ID The object global ID
		 */
		virtual void setGlobalID(const ObjID& ID) { m_globalID = ID; }
		/*!
		 Get the object document index
		 @param index The object index
		 */
		virtual void setIndex(const Index& index) {
			m_ID = index;
			m_dbaseID = index.dbaseID;
			m_tableID = index.tableID;
		}
		
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
		Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override;
		/*!
			Set to the default package content
		*/
		void setDefault() override;
		/*!
			Validate the cargo data
			@return True if the data has been validated
		*/
		bool validate() override;
		
	private:
			///The object document identifier (NB: this may not be globally unique)
		ObjID m_ID;
			///The object global identifier (NB: intended to be globally unique)
		ObjID m_globalID;
			///The object database ID (nullopt = unspecified)
		std::optional<DBaseID> m_dbaseID;
			///The object database ID (nullopt = unspecified)
		std::optional<TableID> m_tableID;
			///The time the object wasor created
		active::utility::Time createTime = active::utility::Time{};
			///The time the object was last edited
		active::utility::Time editTime = active::utility::Time{};
	};

	/*--------------------------------------------------------------------
		Fill an inventory with the package items
		@param inventory The inventory to receive the package items
		@return True if the package has added items to the inventory
	  --------------------------------------------------------------------*/
	template<typename ObjID, typename DBaseID, typename TableID>
	bool Record<ObjID, DBaseID, TableID>::fillInventory(active::serialise::Inventory& inventory) const {
			//TODO: Complete
		return true;
	} //Record::fillInventory


	/*--------------------------------------------------------------------
		Get the specified cargo
		@param item The inventory item to retrieve
		@return The requested cargo (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename ObjID, typename DBaseID, typename TableID>
	active::serialise::Cargo::Unique Record<ObjID, DBaseID, TableID>::getCargo(const active::serialise::Inventory::Item& item) const {
			//TODO: Complete
		return nullptr;
	} //Record::getCargo


	/*--------------------------------------------------------------------
		Set to the default package content
	  --------------------------------------------------------------------*/
	template<typename ObjID, typename DBaseID, typename TableID>
	void Record<ObjID, DBaseID, TableID>::setDefault() {
		//TODO: Complete
	} //Record::setDefault


	/*--------------------------------------------------------------------
		Validate the cargo data
	 
		return: True if the data has been validated
	  --------------------------------------------------------------------*/
	template<typename ObjID, typename DBaseID, typename TableID>
	bool Record<ObjID, DBaseID, TableID>::validate() {
		//TODO: Complete
		return true;
	} //Record::validate
	
}

#endif	//ACTIVE_DATABASE_RECORD
