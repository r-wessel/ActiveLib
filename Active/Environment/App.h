/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ENVIRONMENT_APP
#define ACTIVE_ENVIRONMENT_APP

#include "Active/Event/Publisher.h"

namespace active::database {
	class Transaction;
}

namespace active::environment {
		
	/*!
		A base class for an application
	*/
	class App : public event::Publisher {
	public:

		// MARK: - Constructors
		
		/*!
			Default constructor
			@param identity Optional name/ID for the subscriber
			@param isActiveInstance True if this object is the active instance (the primary application)
		*/
		App(const utility::NameID& identity = utility::NameID{}, bool isActiveInstance = false);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		App(const App& source) : Publisher{source} {}
		/*!
			Destructor
		*/
		virtual ~App();
		
		/*!
		 Make a (database) transaction
		 @param transaction The transaction
		 @return True if the transaction was successfully processed
		 */
		bool makeTransaction(database::Transaction& transaction);
		
	protected:
		/*!
		 Determine if a transaction can be started
		 @return True if a transaction can be started
		 */
		virtual bool canTransactionStart() const { return true; }
		/*!
		 Determine if a transaction is being performed
		 @return True if a transaction is being performed
		 */
		virtual bool isTransacting() const;
		/*!
		 Determine if a speciifc transaction is being performed
		 @return True if the specified transaction is being performed
		 */
		virtual bool isPerforming(const database::Transaction& transaction) const;
		/*!
		 Prepare a transaction for processing
		 @param transaction The transaction to prepare
		 @return True if a transaction can be processed
		 */
		virtual bool prepareTransaction(database::Transaction& transaction) const;
		/*!
		 Perform a transaction
		 @param transaction The transaction to perform
		 @return True if the transaction was successfully performed
		 */
		virtual bool performTransaction(database::Transaction& transaction) const;
		/*!
		 Finalise a transaction
		 @param transaction The transaction to be finalised
		 @param wasPerformedSuccessfully True if the transaction was successfully performed
		 */
		virtual void finaliseTransaction(database::Transaction& transaction, bool wasPerformedSuccessfully) const;
	};

	/*!
	 Get an object representing the parent process/application
	 @return The active application instance (nullptr if the app is not running)
	 */
	 active::environment::App* app();

}

#endif	//ACTIVE_ENVIRONMENT_APP
