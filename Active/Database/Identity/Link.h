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
	 @tparam DBaseID The database identifier type
	 @tparam TableID The table identifier type
	 */
	template<typename ObjID = active::utility::Guid, typename DBaseID = active::utility::Guid, typename TableID = active::utility::Guid>
	class Link : public active::database::Index<ObjID, DBaseID, TableID>, public active::setting::SettingList {
	public:

		// MARK: - Types
		
		using base = active::database::Index<ObjID, DBaseID, TableID>;
			///Unique pointer
		using Unique = std::unique_ptr<Link>;
			///Shared pointer
		using Shared = std::shared_ptr<Link>;
			///Optional
		using Option = std::optional<Link>;

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