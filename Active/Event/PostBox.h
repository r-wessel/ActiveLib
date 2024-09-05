/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_EVENT_POST_BOX
#define ACTIVE_EVENT_POST_BOX

#include "Active/Event/Message.h"

namespace active::event {

	/*!
		Base class to receive messages from event subscribers to the published event author
	 
		This is essentially an interface that makes no assumptions about the message content etc. and should be implemented as required
	*/
	class PostBox : public container::Vector<Message> {
	public:
		
		/*!
			Destructor
		*/
		virtual ~PostBox() = default;
		
		// MARK: - Functions (mutating)
		
		/*!
			Receive a message from an event subscriber for the published event author
			@param message The message incoming message
		*/
		virtual void receive(Message&& message) { emplace_back(std::move(message)); }
	};
	
}

#endif	//ACTIVE_EVENT_POST_BOX
