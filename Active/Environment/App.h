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
	};

	/*!
	 Get an object representing the parent process/application
	 @return The active application instance (nullptr if the app is not running)
	 */
	 active::environment::App* app();

}

#endif	//ACTIVE_ENVIRONMENT_APP
