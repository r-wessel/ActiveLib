#ifndef ACTIVE_DATABASE_RECORD
#define ACTIVE_DATABASE_RECORD

#include "Active/Database/Identity/Link.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/XML/Item/XMLDateTime.h"

#include <any>

namespace active::database {
	
	namespace record {
		
			///Record serialisation fields
		enum FieldIndex {
			idIndex,
			globIndex,
			createIndex,
			editIndex,
		};
		
		
		/*!
			Get the identity of a specified field
			@param index the field index
			@return The field identity
		*/
		const serialise::Identity& getIdentity(FieldIndex index);
		
	}

	/*!
	 Base class for any object stored in a database
	 
	 The object has to support some form of portability and be clonable to support storage and database operations. Note that objects may have 2
	 identifiers:
	 - One for the ID within a document. This remains constant in copies when a document is duplicated, and is therefore not globally unique
	 - One as a globally unique identifier - this must not remain the same in copies
	 @tparam ObjID The object identifier type
	 */
	template<typename ObjID = active::utility::Guid>
	class Record : public active::serialise::Package, public active::utility::Cloner {
	public:

		// MARK: - Types
		
		using Unique = std::unique_ptr<Record>;
			///Shared pointer
		using Shared = std::shared_ptr<Record>;
			///Optional
		using Option = std::optional<Record>;
			///Record index type
		using Index = active::database::Index<ObjID>;
			///Record link type
		using Link = active::database::Link<ObjID>;
		
		// MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		Record() {}
		/*!
		 Constructor
		 @param ID The object document identifier
		 @param globID The global identifier
		 */
		Record(const ObjID& ID, const ObjID& globID) : m_ID{ID}, m_globalID(globID) {}
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
		virtual Index getIndex() const { return Index{m_ID, m_ownerID}; }
		/*!
		 Get the object document link
		 @return The object link
		 */
		virtual Link getLink() const { return Link{getIndex()}; }
		/*!
		 Get the last edit time
		 @return The edit time
		 */
		const active::utility::Time& getEdited() const { return m_editTime; }
		/*!
		 Get the created time
		 @return The created time
		 */
		const active::utility::Time& getCreated() const { return m_createTime; }
		
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
			m_ownerID = index.ownerID;
		}
		/*!
		 Set the last edit time
		 @param editTime The edit time
		 */
		void setEdited(const active::utility::Time& editTime) { m_editTime = editTime; }
		
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
		
	private:
			///The object document identifier (NB: this may not be globally unique)
		ObjID m_ID;
			///The object global identifier (NB: intended to be globally unique)
		ObjID m_globalID;
			///An optional runtime identifier for the object owner in memory - can be used as required for a target application
		std::any m_ownerID;
			///The time the object wasor created
		active::utility::Time m_createTime = active::utility::Time{};
			///The time the object was last edited
		active::utility::Time m_editTime = active::utility::Time{};
	};

	/*--------------------------------------------------------------------
		Fill an inventory with the package items
	 
		inventory: The inventory to receive the package items
	 
		return: True if the package has added items to the inventory
	  --------------------------------------------------------------------*/
	template<typename ObjID>
	bool Record<ObjID>::fillInventory(active::serialise::Inventory& inventory) const {
		using enum serialise::Entry::Type;
		using enum record::FieldIndex;
		inventory.merge(serialise::Inventory{
			{
				{ getIdentity(idIndex), idIndex, element },
				{ getIdentity(globIndex), globIndex, element },
				{ getIdentity(createIndex), createIndex, element },
				{ getIdentity(editIndex), editIndex, element },
			},
		}.withType(&typeid(Record<ObjID>)));
		return true;
	} //Record::fillInventory


	/*--------------------------------------------------------------------
		Get the specified cargo
		@param item The inventory item to retrieve
		@return The requested cargo (nullptr on failure)
	  --------------------------------------------------------------------*/
	template<typename ObjID>
	active::serialise::Cargo::Unique Record<ObjID>::getCargo(const active::serialise::Inventory::Item& item) const {
		if (item.ownerType != &typeid(Record<ObjID>))
			return nullptr;
		using namespace active::serialise;
		using enum record::FieldIndex;
		switch (item.index) {
			case idIndex:
				return std::make_unique<ValueWrap<ObjID>>(m_ID);
			case globIndex:
				return std::make_unique<ValueWrap<ObjID>>(m_globalID);
			case createIndex:
				return std::make_unique<xml::XMLDateTime>(m_createTime);
			case editIndex:
				return std::make_unique<xml::XMLDateTime>(m_editTime);
			default:
				return nullptr;	//Requested an unknown index
		}
	} //Record::getCargo


	/*--------------------------------------------------------------------
		Set to the default package content
	  --------------------------------------------------------------------*/
	template<typename ObjID>
	void Record<ObjID>::setDefault() {
		m_ID.clear();
		m_globalID.clear();
		m_createTime.resetDate();
		m_createTime.resetTime();
		m_editTime.resetDate();
		m_editTime.resetTime();
	} //Record::setDefault
	
}

#endif	//ACTIVE_DATABASE_RECORD
