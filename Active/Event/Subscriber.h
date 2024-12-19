/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_EVENT_SUBSCRIBER
#define ACTIVE_EVENT_SUBSCRIBER

#include "Active/Event/Participant.h"

#include <set>

namespace active::event {
	
	class Event;
	class Publisher;
		
	/*!
		A class representing an event subscriber
		
		A Subscriber registers an interest with a Publisher for one or more Events. Incoming Events will be issued to Subscribers with a registered
		interest in order of priority (defined by the Subscriber). Any Subscriber can signal that the Event has been closed, at which point
		distribution of the event to other Subscribers will cease (e.g. a speific tool has completed a requested task that can only be solved once).
	
		Subscribers are asked to fullfil several tasks:
		- Audit their environment to ensure normal function
		- Attach any components relevant to the app, e.g. UI additions
		- Initialise, e.g. allocate resources
		- Subscribe to at least one event
		- Shutdown
	 
		Note that many subscribers don't need to do anything to fullfil these requirements, e.g. a subscriber with no dependencies has nothing to
		audit. Generally an 'attach' function is only needed where a subscriber has to notity some other system of its presence in order to receive
		events, e.g. a tool acting on the selection of a menu in the UI might need to register the menu resources with the UI manager.
	 
		Subscribers should wait for the publisher to instigate tasks like attach, initialise or shutdown. This will allow a manager acting as a
		Publisher to coordinate the actions of all its subscribers (which may be critical in some contexts).
	*/
	class Subscriber : public Participant {
	public:
				
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Subscriber>;
			///Shared pointer
		using Shared = std::shared_ptr<Subscriber>;
			///Weak pointer
		using Weak = std::weak_ptr<Subscriber>;
			///An event subscription list
		using Subscription = std::set<NameID>;

			///The action awaited by the subscriber
		enum class Action {
			issue = 0,	///<Issue events
			suspend,	///<Suspend operation
			renew,	///<The subscriber has to be discontinued and restarted (audit, attach etc)
			discontinue,	///<The subscription is discontinued
		};

		// MARK: - Constructors
		
		/*!
			Default constructor
			@param priority The subscriber priority (determines the order in which subscribers receive events)
		*/
		Subscriber(int32_t priority = 0) { m_priority = priority; }
		/*!
			Constructor
			@param identity Name/ID for the subscriber
			@param priority The subscriber priority (determines the order in which subscribers receive events)
		*/
		Subscriber(const NameID& identity, int32_t priority = 0) : Participant{identity} { m_priority = priority; }
		/*!
			Destructor
		*/
		virtual ~Subscriber() = default;
		
		// MARK: - Operators

		/*!
			Equality operator
			@param ref The object to compare with this
			@return True if ref is equal to this
		*/
		bool operator== (const Subscriber& ref) const { return Participant::operator==(ref); }
		/*!
			Inequality operator
			@param ref The object to compare with this
			@return True if ref is not equal to this
		*/
		bool operator!= (const Subscriber& ref) const { return !(*this == ref); }
		
		// MARK: - Functions (const)
		
		/*!
			Get the subscriber priority
			@return The subscriber priority
		*/
		virtual bool getPriority() const { return m_priority; }
		/*!
			Get the subscriber action
			@return The subscriber action
		*/
		virtual Action action() const { return m_action; }
		/*!
			Get the event subscription list
			@return The subscription list (an empty list will put the subscriber into a suspended state)
		*/
		virtual Subscription subscription() const = 0;
		
		// MARK: - Functions (mutating)
		
		/*!
			Receive a subscribed event
			@param event The incoming event
			@return True if the event should be closed
		*/
		virtual bool receive(const Event& event) = 0;
		
	protected:
		friend class Publisher;
		
		/*!
			Get the subscriber action
			@param action The subscriber action
		*/
		void setAction(Action action) { m_action = action; }
		
	private:
			///The subscriber priority (higher number = higher priority)
		int32_t m_priority = 0;
			///The subscriber action
		Action m_action = Action::issue;
	};
	
}

#endif	//ACTIVE_EVENT_SUBSCRIBER
