/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Environment/App.h"


using namespace active::environment;

namespace {
	
		//An object representing the parent process/application
	active::environment::App* m_appInstance = nullptr;

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
	Get an object representing the parent process/application
 
	return: The active application instance
 --------------------------------------------------------------------*/
active::environment::App* active::environment::app() {
	return m_appInstance;
} //active::environment::app
