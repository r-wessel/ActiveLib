#ifndef ACTIVE_DATABASE_TABLE_SCHEMA
#define ACTIVE_DATABASE_TABLE_SCHEMA

#include "Active/Setting/SettingList.h"
#include "Active/Setting/ValueSetting.h"

namespace active::database {
	
	/*!
	 Class describing the schema of a database table including fields and primary indices
	 */
	template<typename TableID = active::utility::String>
	class TableSchema : active::setting::SettingList {
	public:
		/*!
		 Constructor
		 @param tabID The table identifier
		 @param globIndex The column index of the primary index field (into the following field list)
		 @param contIndex The column index of the serialised object content
		 @param fields The table fields (currently only value types - blobs can be added separately)
		 */
		TableSchema(const TableID& tabID, size_t globIndex, size_t contIndex, const std::initializer_list<active::setting::ValueSetting> fields) :
			ID{tabID}, globalIndex{globIndex}, contentIndex{contIndex}, SettingList{fields} {}
		/*!
		 Constructor
		 @param tabID The table identifier
		 @param globIndex The column index of the primary index field (into the following field list)
		 @param contIndex The column index of the serialised object content
		 @param docIndex The column index of the document index field (only for document-based dbases, index of the object local to the document)
		 @param docID The column index of the document ID (only for document-based dbases, ID of the parent document)
		 @param fields The table fields
		 */
		TableSchema(const TableID& tabID, size_t globIndex, size_t contIndex, size_t docIndex, size_t docID,
					const std::initializer_list<active::setting::ValueSetting> fields) :
				ID{tabID}, globalIndex{globIndex}, contentIndex{contIndex}, documentIndex{docIndex}, documentID{docID}, SettingList{fields} {}


			//Table identifier
		TableID ID;
			//Index of the primary global index column
		size_t globalIndex;
			//Index of the serialised object content
		size_t contentIndex;
			//Index of the primary document index column (nullopt = no document index)
		std::optional<size_t> documentIndex;
			//Index of the document ID column (nullopt = no document column)
		std::optional<size_t> documentID;
	};
	
}

#endif	//ACTIVE_DATABASE_TABLE_SCHEMA
