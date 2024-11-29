/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Environment/App.h"

#include "Active/Database/Transaction.h"

#include <mutex>

using namespace active::environment;
using namespace active::utility;

namespace {
	
		//An object representing the parent process/application
	active::environment::App* m_appInstance = nullptr;
		///The ID of the active transaction (under processing - nullopt if no transaction is being processed)
	std::optional<Guid> m_activeTransactionID;
		///Mutex to control access to transaction ID
	std::mutex m_transactionIDLock;

}

/*--------------------------------------------------------------------
	Default constructor
 
	identity: Optional name/ID for the subscriber
	isActiveInstance: True if this object is the active instance (the primary application)
 --------------------------------------------------------------------*/
App::App(const utility::NameID& identity, bool isActiveInstance) : Publisher{identity} {
	if (isActiveInstance) {
		if (m_appInstance != nullptr)
			throw;	//There can't be multiple active instances
		m_appInstance = this;
	}
} //App::App


/*--------------------------------------------------------------------
	Destructor
 --------------------------------------------------------------------*/
App::~App() {
	if (m_appInstance == this)
		m_appInstance = nullptr;
} //App::~App


/*--------------------------------------------------------------------
	Make a (database) transaction
 
	transaction: The transaction
 
	return: True if the transaction was successfully processed
 --------------------------------------------------------------------*/
bool App::makeTransaction(database::Transaction& transaction) {
	if (!canTransactionStart())
		return false;
	if (!prepareTransaction(transaction))
		return true;
	{
		const std::lock_guard<std::mutex> lock{m_transactionIDLock};
		if (!m_activeTransactionID)
			m_activeTransactionID = transaction.getID();
	}
	bool result = performTransaction(transaction);
	{
		const std::lock_guard<std::mutex> lock{m_transactionIDLock};
		if (m_activeTransactionID == transaction.getID())
			m_activeTransactionID.reset();
	}
	finaliseTransaction(transaction, result);
	return result;
} //App::makeTransaction


/*--------------------------------------------------------------------
	Determine if a transaction is being performed
 
	return: True if a transaction is being performed
 --------------------------------------------------------------------*/
bool App::isTransacting() const {
	const std::lock_guard<std::mutex> lock{m_transactionIDLock};
	return m_activeTransactionID.operator bool();
} //App::isTransacting


/*--------------------------------------------------------------------
	Determine if a speciifc transaction is being performed
 
	return: True if the specified transaction is being performed
 --------------------------------------------------------------------*/
bool App::isPerforming(const database::Transaction& transaction) const {
	const std::lock_guard<std::mutex> lock{m_transactionIDLock};
	return m_activeTransactionID == transaction.getID();
} //App::isPerforming


/*--------------------------------------------------------------------
	Prepare a transaction to be performed
 
	transaction: The transaction to prepare
 
	return: True if a transaction can be performed
 --------------------------------------------------------------------*/
bool App::prepareTransaction(database::Transaction& transaction) const {
	return transaction.prepare();
} //App::prepareTransaction


/*--------------------------------------------------------------------
	Perform a transaction
 
	transaction: The transaction to perform
 
	return: True if the transaction was successfully performed
 --------------------------------------------------------------------*/
bool App::performTransaction(database::Transaction& transaction) const {
	return transaction.perform();
} //App::performTransaction


/*--------------------------------------------------------------------
	Finalise a transaction
 
	transaction: The transaction to be finalised
	wasPerformedSuccessfully: True if the transaction was successfully performed
 --------------------------------------------------------------------*/
void App::finaliseTransaction(database::Transaction& transaction, bool wasPerformedSuccessfully) const {
	return transaction.finalise(wasPerformedSuccessfully);
} //App::finaliseTransaction


/*--------------------------------------------------------------------
	Get an object representing the parent process/application
 
	return: The active application instance
 --------------------------------------------------------------------*/
active::environment::App* active::environment::app() {
	return m_appInstance;
} //active::environment::app
