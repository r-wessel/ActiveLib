#ifndef ACTIVE_DATABASE_SQLITE_CORE
#define ACTIVE_DATABASE_SQLITE_CORE

#include "Active/File/Path.h"
#include "Active/Setting/SettingList.h"
#include "Active/Database/Storage/DBaseSchema.h"

namespace active::database {
	
	using SQLiteSchema = DBaseSchema<>;
	
	/*!
	 Core functionality and definitions for a SQLite database
	 */
	class SQLiteCore  {
	public:

		// MARK: - Types
		
			///Status of of the SQLite database
		enum class Status {
			nominal,	///<No errors logged
			error,	///<A non-specific error occurred
			internal,	///<An error occurred in the SQLite core library engine
			permissions,	///<The requested permissions could not be provided
			abortOp,	///<An operation was aborted, typically at the application request
			busy,	///<The database file could not be written, typically due to concurrent activities
			locked,	///<A write operation could not be continue due to a conflict with the came connection
			outOfMemory,	///<Memory exhausted
			readOnlyState,	///<Attempt to write to a read-only file was refused
			interrupt,	///<An operation was interrupted, e.g. by the UI
			ioErr,	///<The operating system reported an io error
			corrupt,	///<The database file has been corrupted
			full,	///<The drive is full
			cantOpen,	///<The file can't be opened
				//Continue as required - full error list is long
			tableNotFound = 0xF000,	///<The specified table can't be found
			contentNotFound,	///<The content column is missing
		};
		
			///An SQLite transaction - create using `makeTransaction`
		class Transaction {
		public:
			friend class SQLiteCore;

			/*!
			 Destructor
			 */
			~Transaction();

			/*!
			 Increment operator
			 @return The current transaction row (nullptr = engine is busy: https://sqlite.org/rescode.html#busy)
			 @throw Exception thrown on SQL error
			 */
			active::setting::SettingList::Unique operator++();
			/*!
			 Conversion operator
			 @return True if the transaction is not done
			 */
			operator bool() const { return !m_isDone; }
			
		private:
			/*!
			 Constructor
			 @param sql The parent SQLite database
			 @param statement The statement to be executed by the transaction
			 */
			Transaction(const SQLiteCore* sql, const active::utility::String& statement) : m_sqlite(sql), m_statement(statement) {}

				///The transaction statement to be executed
			active::utility::String m_statement;
				///Handle to the prepared transaction statement
			void* m_handle = nullptr;
				///The parent SQLite database
			const SQLiteCore* m_sqlite;
				///True if the transaction is done, i.e. don't increment again
			bool m_isDone = true;
		};
		
		friend class Transaction;

		// MARK: - Static functions
		
		/*!
		 Make an error code for SQLite processing
		 @return An STL error code
		 */
		static std::error_code makeError(SQLiteCore::Status code);

		// MARK: - Constructors
		
		/*!
		 Default constructor
		 @param path Path to the SQLite dbase
		 @param schema Database schema
		 */
		SQLiteCore(const active::file::Path& path, SQLiteSchema&& schema) : m_path(path), m_schema(schema) {}
		/*!
		 Destructor
		 */
		virtual ~SQLiteCore();
		
		// MARK: - Function (const)
		
		/*!
		 Get the database schema
		 @return The database schema
		 */
		const SQLiteSchema& getSchema() const { return m_schema; }
		
		// MARK: - Function (mutating)
		
		/*!
		 Make a SQLite transaction
		 @param statement The statement to be executed by the transaction
		 @return The SQLite handle (nullptr = failure to open database connection)
		 */
		Transaction makeTransaction(const active::utility::String& statement) const { return Transaction{this, statement}; }
		
	protected:
		/*!
		 Get the SQLite handle (ensuring the database connection is open if possible)
		 @return The SQLite handle (nullptr = failure to open database connection)
		 */
		void* getHandle() const;

	private:
			///The database
		SQLiteSchema m_schema;
			///Path to the database
		active::file::Path m_path;
			///The SQLite connection handle
		mutable void* m_handle = nullptr;
	};
	
}

#endif	//ACTIVE_DATABASE_SQLITE_CORE
