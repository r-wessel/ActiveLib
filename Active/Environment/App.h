/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_ENVIRONMENT_APP
#define ACTIVE_ENVIRONMENT_APP

#include "Active/Event/Publisher.h"

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
		*/
		App(const utility::NameID& identity = utility::NameID{}) : Publisher{identity} {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		App(const App& source) : Publisher{source} {}
		/*!
			Destructor
		*/
		virtual ~App() = default;
	};
	
}

	//An object representing the parent process/application
extern active::environment::App* app;

#endif	//ACTIVE_ENVIRONMENT_APP
