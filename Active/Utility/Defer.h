/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_DEFER
#define ACTIVE_UTILITY_DEFER

#include <functional>
#include <utility>

namespace active::utility {

	///A class to hold an action to be deferred until its scope is exited
	/*!
		Used in cases where some action must be taken before a specific scope exits, often a 'closing' action counterbalancing some corresponding
		'opening' action, e.g. balancing unlocking a mutex with locking it, and particulary where there may be multiple paths or exit points within
		the context. This is sometimes handled by writing a block of exit code and jumping to it from every exit point with 'goto'. But this can be
		hard to read and it's far too easy to either omit a necessary action or fail to jump to the correct exit point.
	 
		Some languages incorporate an explicit 'defer' statement for this purpose, e.g.:
	
			opening statement
			defer {
				closing statement
			}
	
		This pattern ensures the closing statement is executed no matter how the scope is exited. The `Defer` class provides the same pattern
		in C++, e.g.:
	 
			auto fooResource = aquireFooResource();
			auto scope = defer([&]{
				releaseFooResource(fooResource);
			});
	 			//...more code in this scope with multiple exit points
	 
		NB: The variable name in this example (`scope`) has no significance. When the variable goes out of scope (including a thrown exception)
		the lambda will be executed. There can be any number of these in any given scope, so immediately follow any opening statement with a the
		corresponding `defer`. This will guarantee the required code is executed.
	*/
	class Defer {
	public:
		
			///Deferred action function
		using Action = std::function<void()>;
		
		/*!
			Constructor
			@param action The action to defer
		*/
		Defer(const Action& action) : m_action{action} {}
		/*!
			Constructor
			@param action The action to defer
		*/
		Defer(Action&& action) : m_action(std::move(action)) {}
		/*!
			Move constructor
			@param source The object to move
		*/
		Defer(Defer&& source) {
			m_action = std::exchange(source.m_action, nullptr);
		}
		Defer(const Defer&) = delete;	//No copy constructor
		Defer& operator=(const Defer&) = delete;	//No assignment operator
		/*!
			Destructor
		*/
		~Defer() {
			if (m_action)
				m_action();	//Execute the deferred action when destroyed
		}

	private:
			///The deferred action
		Action m_action;
	};


	/*!
		Factory function to instantiate a deferred action
	*/
	inline Defer defer(Defer::Action&& action) {
		return Defer{action};
	}
	
}

#endif	//ACTIVE_UTILITY_DEFER
