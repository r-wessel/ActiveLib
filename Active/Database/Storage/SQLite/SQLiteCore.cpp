#include "Active/Database/Storage/SQLite/SQLiteCore.h"

#include "Active/Database/Storage/DBaseSchema.h"
#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Setting/Values/Int64Value.h"
#include "Active/Setting/Values/StringValue.h"
#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"
#include "SQLite/sqlite3.h"

using namespace active::database;
using namespace active::setting;
using namespace active::utility;

using enum SQLiteCore::Status;

namespace {
	
		///Category for SQLite processing errors
	class SQLiteCategory : public std::error_category {
	public:
			///Category name
		const char* name() const noexcept override {
			return "active::database::sqlite::category";
		}
		/*!
			Get a message for a specified error code
			@param errorCode A SQLite processing code
			@return The error message for the specified code
		*/
		std::string message(int errorCode) const override {
			switch (static_cast<SQLiteCore::Status>(errorCode)) {
				case nominal:
					return "";
				case error:
					return "A non-specific error occurred";
				case internal:
					return "An error occurred in the SQLite core library engine";
				case permissions:
					return "The requested permissions could not be provided";
				case abortOp:
					return "An operation was aborted, typically at the application request";
				case busy:
					return "The database file could not be written, typically due to concurrent activities";
				case locked:
					return "A write operation could not be continue due to a conflict with the came connection";
				case outOfMemory:
					return "Memory exhausted";
				case readOnlyState:
					return "ttempt to write to a read-only file was refused";
				case interrupt:
					return "An operation was interrupted, e.g. by the UI";
				case ioErr:
					return "The operating system reported an io error";
				case corrupt:
					return "The database file has been corrupted";
				case full:
					return "The drive is full";
				case cantOpen:
					return "The file can't be opened";
				default:
					return "Unknown/invalid error";
			}
		}
	};

		///SQLite processing category error instance
	static SQLiteCategory instance;
	
}

/*--------------------------------------------------------------------
	Destructor
 --------------------------------------------------------------------*/
SQLiteCore::Transaction::~Transaction() {
		//Ensure an open statement is closed
	if (m_handle != nullptr)
		sqlite3_finalize((sqlite3_stmt*) m_handle);
} //SQLiteCore::Transaction::~Transaction


/*--------------------------------------------------------------------
	Increment operator
 
	return: The current transaction row (nullptr = transaction successfully completed)
 --------------------------------------------------------------------*/
SettingList::Unique SQLiteCore::Transaction::operator++() {
		//First check if we need to prepare the statement
	if (m_handle == nullptr) {
		auto dbaseHandle = m_sqlite->getHandle();
		const char* dataTail = nullptr;
		if (auto status = sqlite3_prepare_v2((sqlite3*) dbaseHandle, m_statement.data(), m_statement.dataSize() + 1,
											 (sqlite3_stmt**) &m_handle, &dataTail); (status != SQLITE_OK) && (dataTail != nullptr))
			throw std::system_error(makeError(static_cast<Status>(status)));
		m_isDone = false;
	}
	auto status = sqlite3_step((sqlite3_stmt*) m_handle);
	switch (status) {
		case SQLITE_DONE:
			m_isDone = true;
			break;
		case SQLITE_ROW: case SQLITE_OK: {
			auto row = std::make_unique<SettingList>();
			bool isMore = true;
			for (int i = 0; isMore; ++i) {
					//Build an identifying name from the table and column names
				NameID identifier;
				auto tableName = sqlite3_column_table_name((sqlite3_stmt*)m_handle, i);
				if (tableName != nullptr)
					identifier.name = String{tableName} + "::";
				auto columnName = sqlite3_column_origin_name((sqlite3_stmt*)m_handle, i);
				if (columnName != nullptr)
					identifier.name += columnName;
				switch (sqlite3_column_type((sqlite3_stmt*) m_handle, i)) {
					case SQLITE_INTEGER:
						row->emplace_back(std::make_unique<ValueSetting>(Int64Value{sqlite3_column_int64((sqlite3_stmt*) m_handle, i)}, identifier));
						break;
					case SQLITE_FLOAT:
						row->emplace_back(std::make_unique<ValueSetting>(DoubleValue{sqlite3_column_double((sqlite3_stmt*) m_handle, i)}, identifier));
						break;
					case SQLITE_TEXT:
						row->emplace_back(std::make_unique<ValueSetting>(StringValue{(const char*) sqlite3_column_text((sqlite3_stmt*) m_handle, i)}, identifier));
						break;
					default:
						isMore = false;
						break;
				}
			}
			return row;
		}
		case SQLITE_BUSY:
			break;	//This will return nullptr, allowing the caller to retry
		default:
			throw std::system_error(makeError(static_cast<Status>(status)));
	}
	return nullptr;
} //SQLiteCore::Transaction::operator++


/*--------------------------------------------------------------------
	Execute a single-step process, e.g. erase, insert etc
 --------------------------------------------------------------------*/
void SQLiteCore::Transaction::execute() const {
	
} //SQLiteCore::Transaction::execute


/*--------------------------------------------------------------------
	Make an error code for SQLite processing
 
	return: An STL error code
 --------------------------------------------------------------------*/
std::error_code SQLiteCore::makeError(SQLiteCore::Status code) {
	return std::error_code(static_cast<int>(code), instance);
} //SQLiteCore::makeError


/*--------------------------------------------------------------------
	Convert a string to an SQLite string literal (escape single-quotes, e.g. ' -> '')
 
	text: The string to convert
 
	return: An SQLite string literal
 --------------------------------------------------------------------*/
String SQLiteCore::toSQLiteString(const String& text) {
	String result{text};
	result.replaceAll("'", "''");
	return result;
} //SQLiteCore::toSQLiteString


/*--------------------------------------------------------------------
	Destructor
 --------------------------------------------------------------------*/
SQLiteCore::~SQLiteCore() {
		//Ensure an open connection is closed
	if (m_handle != nullptr)
		sqlite3_close((sqlite3*) m_handle);
} //SQLiteCore::~SQLiteCore


/*--------------------------------------------------------------------
	Get the SQLite handle (ensuring the database connection is open if possible)
 
	return: The SQLite handle (nullptr = failure to open database connection)
--------------------------------------------------------------------*/
void* SQLiteCore::getHandle() const {
	if (m_handle != nullptr)
		return m_handle;
	uint32_t flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
	if (auto status = sqlite3_open_v2(String{m_path}.data(), (sqlite3**) &m_handle, flags, nullptr); status != SQLITE_OK)
		throw std::system_error(makeError(static_cast<Status>(status)));
		//TODO: Ensure dbase schema is applied to newly created files, and ensure schema field order matches file order
	return m_handle;
} //SQLiteCore::getHandle
