/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_DATABASE_TRANSACTION
#define ACTIVE_DATABASE_TRANSACTION

#include "Active/Utility/Guid.h"
#include "Active/Utility/String.h"

#include <memory>

namespace active::environment {
	class App;
}

namespace active::database {
	
	/*!
	 A database transaction
	 
	 Operations requiring write access to a document/database should be wrapped in a transaction and performed through the app
	 to support sequenced operations like undo.
	 
	 Note that only Transaction::perform() must be implemented - prepare() and finalise() are optional. Ideally carry out as much work
	 as possible outside the perform() phase to minimise the time the action holds write access to databases etc. Use prepare() and
	 finalise() accordingly where write access is not required
	 */
	class Transaction {
	public:

		//MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Transaction>;
			///Shared pointer
		using Shared = std::shared_ptr<Transaction>;
			///Optional
		using Option = std::optional<Transaction>;

		//MARK: - Constructors
		
		/*!
		 Constructor
		 @param name The transaction name (for displaying undo etc)
		 */
		Transaction(const utility::String& name) : m_name(name)	{}
		/*!
		 Destructor
		 */
		virtual ~Transaction()	{}

		//MARK: - Functions (const)
		
		/*!
		 Get the transaction name
		 @return The transaction name
		 */
		const utility::String& getName() const	{ return m_name; }
		/*!
		 Get the transaction ID
		 @return The transaction ID
		 */
		utility::Guid getID() const	{ return m_ID; }
		
	protected:

		friend class environment::App;
		
		//MARK: - Functions (App management)
		
		/*!
		 Prepare the transaction, e.g. collect data and check viability etc
		 @return True if the transaction can be performed (returning false will cause the transaction to be abandoned)
		 */
		virtual bool prepare()	{ return true; }
		/*!
		 Perform the transaction. Database writes can be performed. NB: this function is not called if the prepare phase was unsuccessful)
		 @return True if the transaction was successfully performed
		 */
		virtual bool perform() = 0;
		/*!
		 Finalise the transaction (database access is now closed - use this phase to clean up or follow up with other actions)
		 @param isActioned True if the transaction was successfully performed
		 */
		virtual void finalise(bool isActioned)	{}
		
	private:
			///The transaction name (for logging and user feedback, i.e. should be easily readable)
		utility::String m_name;
			///The transaction guid
		utility::Guid m_ID = utility::Guid{true};
	};
	
}

#endif	//ACTIVE_DATABASE_TRANSACTION
