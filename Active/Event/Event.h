/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_EVENT_EVENT
#define ACTIVE_EVENT_EVENT

#include "Active/Event/PostBox.h"
#include "Active/Setting/SettingList.h"
#include "Active/Utility/NameID.h"

namespace active::event {
	
	class Message;
		
	/*!
		A class representing an event
		
		Events are issued through a Publisher object to interested Subscriber objects. Subscribers will notify a relevant Publisher of the type of
		events is is interested in receiving.
		An event has a two-fold purpose:
		1) Notification that something has happened
		2) Transporting data relevant to the event
		For example, a service might need to sign out of an online account when an app quits. It will ask for notification from the app signaling
		that (for whatever reason) the app is about to quit. The event might carry information about the reason for the quit.
	*/
	class Event : public utility::NameID, public setting::SettingList {
	public:
				
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Event>;
			///Shared pointer
		using Shared = std::shared_ptr<Event>;
			///Optional
		using Option = std::optional<Event>;
		
		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		Event() {}
		/*!
			Constructor
			@param nameID The event identity
			@param postBox Optional postbox for the published event author to receive messages from subscribers
		*/
		Event(const utility::NameID& nameID, PostBox* postBox = nullptr) : utility::NameID{nameID}, setting::SettingList{} { m_postBox = postBox; }
		/*!
			Destructor
		*/
		virtual ~Event() {}
		
		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Event* clonePtr() const { return new Event{*this}; }
		
		// MARK: - Operators

		/*!
			Equality operator
			@param ref The object to compare with this
			@return True if ref is equal to this
		*/
		bool operator== (const Event& ref) const { return utility::NameID::operator==(ref); }	//Events are matched by identity
		/*!
			Equality operator
			@param ref The object to compare with this
			@return True if ref is equal to this
		*/
		bool operator== (const NameID& ref) const { return utility::NameID::operator==(ref); }
		/*!
			Inequality operator
			@param ref The object to compare with this
			@return True if ref is not equal to this
		*/
		bool operator!= (const Event& ref) const { return !(*this == ref); }
		
		// MARK: - Functions (const)
		
		/*!
			Send a message to the published event author (NB: The author is free to ignore messages from subscribers)
			@param message The message to send
		*/
		void tellAuthor(Message&& message) const {
			if (m_postBox != nullptr)
				m_postBox->receive(std::move(message));
		}
		
		// MARK: - Functions (mutating)

		
	private:
			//The post box for message sent from subscribers to the published event author
		PostBox* m_postBox = nullptr;
	};
	
}

#endif	//ACTIVE_EVENT_EVENT
