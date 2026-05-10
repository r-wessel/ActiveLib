#ifndef ACTIVE_DATABASE_LINK
#define ACTIVE_DATABASE_LINK

#include "Active/Setting/SettingList.h"
#include "Active/Database/Identity/Index.h"

namespace active::database {

	/*!
	 Identifer for a database object coupled with essential metadata
	 
	 In essence, a link to an object is the same as an index. However, in some circumstances different subsystems may need to transmit key metadata
	 that can variously convey:
	 - internal information about the object
	 - information about related objects
	 - state information related to the object
	 @tparam ObjID The object identifier type
	 */
	template<typename ObjID = active::Guid, typename TableID = active::Guid, typename DocID = active::Guid>
	class Link : public active::database::Index<ObjID, TableID, DocID>, public active::setting::SettingList {
	public:

		// MARK: - Types
		
		using base = active::database::Index<ObjID, TableID, DocID>;

		// MARK: - Constructors
		
		using base::base;
		using active::setting::SettingList::SettingList;

		Link() = default;
		/*!
		 Constructor
		 @param index A database index
		 */
		Link(const base& index) : base{index} {}
	};
	
}

#endif	//ACTIVE_DATABASE_LINK
