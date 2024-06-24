/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Event/Publisher.h"

#include "Active/Event/Event.h"
#include "Active/Event/Subscriber.h"

#include <algorithm>
#include <mutex>

using namespace active::event;
using namespace active::utility;

using enum Subscriber::Action;

namespace {
	
	template<class T>
	class SubscriberList : public std::vector<T> {
	public:
		~SubscriberList() {}
		
		
		using base = std::vector<T>;
		using iterator = typename base::iterator;
		
		iterator findBySub(const Subscriber& sub) {
			return find_if(base::begin(), base::end(), [&](auto& subscriber) {
				if (auto locked = lock(subscriber); locked && (locked.get() == &sub))
					return true;
				return false;
			});
		}
		iterator findByID(const Guid& id) {
			return find_if(base::begin(), base::end(), [&](auto& subscriber){
				if (auto locked = lock(subscriber); locked && (locked->id == id))
					return true;
				return false;
			});
		}
		/*!
			Filter the list to remove redundant items and optionally find subscriptions to a specified event identify (from the remainder)
			@param subscriberPool A pool of active subscribers to the event
			@param eventID An event identifier to match (nullopt = pool all active subscribers)
			@return A list of subscribers that require renewal
		*/
		std::vector<T> filter(std::vector<Subscriber::Shared>& subscriberPool, const NameID::Option eventID = std::nullopt) {
			std::vector<T> toRenew;
			for (auto i = base::size(); i--; ) {
				auto subscriber = lock((*this)[i]);
				if (!subscriber || (subscriber->action() != issue)) {
					if (subscriber) {
						if (subscriber->action() == discontinue)
							continue;
						if (subscriber->action() == renew)
							toRenew.push_back((*this)[i]);
					}
					(*this)[i] = base::back();
					base::pop_back();
					continue;
				}
				if (subscriber->action() == suspend)
					continue;
				if (!eventID || subscriber->subscription().contains(*eventID))
					subscriberPool.push_back(subscriber);
			}
			return toRenew;
		}
		
	private:
		/*!
			Lock the subscriber (primarily for weak_ptrs)
			@return A shared_ptr to the item
		*/
		std::shared_ptr<Subscriber> lock(T& t) {
			return t;
		}
	};
	
	/*!
		Lock the subscriber (primarily for weak_ptrs)
		@return A shared_ptr to the item
	*/
	template<> inline
	std::shared_ptr<Subscriber> SubscriberList<std::weak_ptr<Subscriber>>::lock(std::weak_ptr<Subscriber>& t) {
		return t.lock();
	}

	class WeakList : public SubscriberList<std::weak_ptr<Subscriber>> {};
	class SharedList : public SubscriberList<std::shared_ptr<Subscriber>> {};
}  // namespace

namespace active::event {
	
	class Publisher::SubscriberList {
	public:
		/*!
			Default constructor
		*/
		SubscriberList() {
			m_weak = std::make_unique<WeakList>();
			m_shared = std::make_unique<SharedList>();
		}
		/*!
			Copy constructor
			source: The object to copy
		*/
		SubscriberList(const SubscriberList& source) {
			m_weak = std::make_unique<WeakList>(*source.m_weak);
			m_shared = std::make_unique<SharedList>(*source.m_shared);
		}
		/*!
			Destructor
		*/
		~SubscriberList() {}

		/*!
			Add a managed subscriber, i.e. the subscription continues until the publisher is explicitly instructed to end it
			@param subscriber The new subscriber
			@return True if subscriber was added
		*/
		bool add(std::shared_ptr<Subscriber> subscriber) {
			if (subscriber->id) {
				if (auto sub = m_shared->findByID(subscriber->id); sub != m_shared->end())
					return false;
			}
			m_shared->push_back(subscriber);
			return true;
		}
		/*!
			Add a casual subscriber, i.e. the subscription ends as soon as the subscriber disappears
			@param subscriber The new subscriber
			@return True if subscriber was added
		*/
		bool add(std::weak_ptr<Subscriber> subscriber) {
			auto lock = subscriber.lock();
			if (!lock)
				return false;	//The pointer has already expired
			if (lock->id) {
				if (auto sub = m_weak->findByID(lock->id); sub != m_weak->end())
					return false;
			}
			m_weak->push_back(subscriber);
			return true;
		}
		/*!
			Remove a subscriber (by memory address)
			@param subscriber A reference to the subscriber to remove
			@return True if subscriber was removed
		*/
		bool remove(const Subscriber& subscriber) {
			bool result = false;
			if (auto sub = m_shared->findBySub(subscriber); sub != m_shared->end()) {
				m_shared->erase(sub);
				result = true;
			}
			if (auto sub = m_weak->findBySub(subscriber); sub != m_weak->end()) {
				m_weak->erase(sub);
				result = true;
			}
			return result;
		}
		/*!
			Remove a subscriber by ID
			@param id The ID of the subscriber to remove
			@return True if subscriber was removed
		*/
		bool remove(const utility::Guid& id) {
			bool result = false;
			if (auto sub = m_shared->findByID(id); sub != m_shared->end()) {
				m_shared->erase(sub);
				result = true;
			}
			if (auto sub = m_weak->findByID(id); sub != m_weak->end()) {
				m_weak->erase(sub);
				result = true;
			}
			return result;
		}
		/*!
			Filter the subscribers to remove redundant items and optionally find subscriptions to a specified event (from the remainder)
			@param eventID An event identifier to filter the subscribers by (nullopt = pool all active subscribers)
			@return The filtered subscribers
		*/
		std::vector<std::shared_ptr<Subscriber>> filter(const utility::NameID::Option eventID = std::nullopt) {
			std::vector<Subscriber::Shared> subscribers;
			m_mutex.lock();
				//Filter subscribers for those expecting this event (and remove any expired)
			auto renewShared = m_shared->filter(subscribers, eventID);
			for (auto& subscriber : renewShared)
				add(subscriber);
			auto renewWeak = m_weak->filter(subscribers, eventID);
			for (auto& subscriber : renewWeak)
				add(subscriber);
			m_mutex.unlock();
			return subscribers;
		}
		
	private:
		std::unique_ptr<WeakList> m_weak;
		std::unique_ptr<SharedList> m_shared;
		std::mutex m_mutex;
	};
	
}  // namespace active::event

/*--------------------------------------------------------------------
	Default constructor
 
	identity: Optional name/ID for the subscriber
  --------------------------------------------------------------------*/
Publisher::Publisher(const utility::NameID& identity) : Participant{identity} {
	m_subscriber = std::make_unique<SubscriberList>();
} //Publisher::Publisher


/*--------------------------------------------------------------------
	Copy constructor
 
	source: The object to copy
  --------------------------------------------------------------------*/
Publisher::Publisher(const Publisher& source) {
	m_subscriber = std::make_unique<SubscriberList>(*source.m_subscriber);
} //Publisher::Publisher


/*--------------------------------------------------------------------
	Destructor
  --------------------------------------------------------------------*/
Publisher::~Publisher() {}


/*--------------------------------------------------------------------
	Assignment operator
 
	source: The object to copy
 
	return: A reference to this
  --------------------------------------------------------------------*/
Publisher& Publisher::operator= (const Publisher& source) {
	if (this != &source)
		m_subscriber = std::make_unique<SubscriberList>(*source.m_subscriber);
	return *this;
}


/*--------------------------------------------------------------------
	Publish an event to subscribers
 
	event: The event to publish (NB: this event can carry data and optionally a postbox to receive subscriber messages
 
	return: True if the event was closed by a subscriber
  --------------------------------------------------------------------*/
bool Publisher::publish(const Event& event) {
	auto subscribers = m_subscriber->filter(event);
		//Sort subscribers by priority
	std::sort(subscribers.begin(), subscribers.end(), [](auto& lhs, auto& rhs){ return lhs->getPriority() > rhs->getPriority(); });
		//Publish the event to subscribers - stop only if one signals the event is closed
	for (auto& subscriber : subscribers)
		if (subscriber->receive(event))
			return true;
	return false;
} //Publisher::publish


/*--------------------------------------------------------------------
	Add a managed subscriber, i.e. the subscription continues until the publisher is explicitly instructed to end it
 
	subscriber: The new subscriber
 
	return: True if subscriber was added
  --------------------------------------------------------------------*/
bool Publisher::add(std::shared_ptr<Subscriber> subscriber) {
	return m_subscriber->add(subscriber);
} //Publisher::add


/*--------------------------------------------------------------------
	Add a casual subscriber, i.e. the subscription ends as soon as the subscriber disappears
 
	subscriber: The new subscriber
 
	return: True if subscriber was added
  --------------------------------------------------------------------*/
bool Publisher::addWeak(std::shared_ptr<Subscriber> subscriber) {
	std::weak_ptr<Subscriber> weak = subscriber;
	return m_subscriber->add(weak);
} //Publisher::add


/*--------------------------------------------------------------------
	Remove a subscriber (by memory address)
 
	subscriber: A reference to the subscriber to remove
 
	return: True if subscriber was removed
  --------------------------------------------------------------------*/
bool Publisher::remove(const Subscriber& subscriber) {
	return m_subscriber->remove(subscriber);
} //Publisher::remove


/*--------------------------------------------------------------------
	Remove a subscriber by ID
 
	id: The ID of the subscriber to remove
 
	return: True if subscriber was removed
  --------------------------------------------------------------------*/
bool Publisher::remove(const Guid& id) {
	return m_subscriber->remove(id);
} //Publisher::remove


/*--------------------------------------------------------------------
	Audit the participant environment to ensure it can function
 
	return: True if the participant is able to function
  --------------------------------------------------------------------*/
bool Publisher::audit() {
	auto subscribers = m_subscriber->filter();
	bool allSuccessful = true;
	for (auto& subscriber : subscribers)
		if (!subscriber->audit())
			allSuccessful = false;
	return allSuccessful;
} //Publisher::audit


/*--------------------------------------------------------------------
	Attach participant components to the app (as required)
 
	return: True if the participant is able to function
  --------------------------------------------------------------------*/
bool Publisher::attach() {
	auto subscribers = m_subscriber->filter();
	bool allSuccessful = true;
	for (auto& subscriber : subscribers)
		if (!subscriber->attach())
			allSuccessful = false;
	return allSuccessful;
} //Publisher::audit


/*--------------------------------------------------------------------
	Initialise the participant operation
 
	return: True if the participant is able to continue
  --------------------------------------------------------------------*/
bool Publisher::start() {
	auto subscribers = m_subscriber->filter();
	bool allSuccessful = true;
	for (auto& subscriber : subscribers)
		if (!subscriber->start())
			allSuccessful = false;
	return allSuccessful;
} //Publisher::audit


/*--------------------------------------------------------------------
	Shut down event handling
  --------------------------------------------------------------------*/
void Publisher::stop() {
	auto subscribers = m_subscriber->filter();
	for (auto& subscriber : subscribers)
		subscriber->stop();
} //Publisher::audit
