/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_EVENT_PUBLISHER
#define ACTIVE_EVENT_PUBLISHER

#include "Active/Event/Participant.h"

namespace active::event {
	
	class Event;
	class Subscriber;
		
	/*!
		A class representing an event publisher
		
		Publishers issue Events to Subscribers (as requested). A Publisher typically represents some source of events, e.g. an App UI, a network
		connection, or even a single control in a UI view, and will publish events relevant to its operation. For example, an App might issue an
		Event to signal that it is about to shut down, giving Subscribers an opportunity to release resources etc.
	*/
	class Publisher : public Participant {
	public:
				
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Publisher>;
			///Shared pointer
		using Shared = std::shared_ptr<Publisher>;

		// MARK: - Constructors
		
		/*!
			Default constructor
			@param identity Optional name/ID for the subscriber
		*/
		Publisher(const utility::NameID& identity = utility::NameID{});
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Publisher(const Publisher& source);
		/*!
			Destructor
		*/
		virtual ~Publisher();
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Publisher& operator= (const Publisher& source);
		
		
		// MARK: - Functions (mutating)
		
		/*!
			Publish an event to subscribers
			@param event The event to publish (NB: this event can carry data and optionally a postbox to receive subscriber messages
			@return True if the event was closed by a subscriber
		*/
		bool publish(const Event& event);
		/*!
			Add a managed subscriber, i.e. the subscription continues until the publisher is explicitly instructed to end it
			@param subscriber The new subscriber
			@return True if subscriber was added
		*/
		bool add(std::shared_ptr<Subscriber> subscriber);
		/*!
			Add a casual subscriber, i.e. the subscription ends as soon as the subscriber disappears
			@param subscriber The new subscriber
			@return True if subscriber was added
		*/
		bool addWeak(std::shared_ptr<Subscriber> subscriber);
		/*!
			Construct and add a new subscriber
			@param args The constructor arguments
			@return The new subscriber on success, nullptr on failure
		*/
		template<class T, class... Args>
		std::shared_ptr<T> add(Args&&... args) {
			auto subscriber = std::make_shared<T>(std::forward<Args>(args)...);
			return add(subscriber) ? subscriber : nullptr;
		}
		/*!
			Remove a subscriber (by memory address)
			@param subscriber A reference to the subscriber to remove
			@return True if subscriber was removed
		*/
		bool remove(const Subscriber& subscriber);
		/*!
			Remove a subscriber by ID
			@param id The ID of the subscriber to remove
			@return True if subscriber was removed
		*/
		bool remove(const utility::Guid& id);
	
	protected:
		/*!
			Audit the participant environment to ensure it can function
			@return True if the participant is able to function
		*/
		bool audit() override;
		/*!
			Attach participant components to the app (as required)
			@return True if the participant is able to function
		*/
		bool attach() override;
		/*!
			Initialise the participant operation
			@return True if the participant is able to continue
		*/
		bool start() override;
		/*!
			Shut down event handling
		*/
		void stop() override;
		
	private:
		class SubscriberList;
		std::unique_ptr<SubscriberList> m_subscriber;
	};
	
}

#endif	//ACTIVE_EVENT_PUBLISHER
