/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_EVENT_PARTICIPANT
#define ACTIVE_EVENT_PARTICIPANT

#include "Active/Utility/NameID.h"

namespace active::event {
	
	/*!
		A base class for any object participating in Event management (publishing or subscribing)
		
		Participants are asked to fullfil several tasks:
		- Audit their environment to ensure normal function
		- Attach any components relevant to the app, e.g. UI additions
		- Initialise, e.g. allocate resources
		- Shutdown (release resources etc)
	*/
	class Participant : public ::active::utility::NameID {
	public:
		/*!
			Default constructor
			@param identity Optional name/ID for the participant
		*/
		Participant(const NameID& identity = NameID{}) : NameID{identity} {}
		/*!
			Destructor
		*/
		virtual ~Participant() = default;
		
		// MARK: - Operators

		/*!
			Equality operator
			@param ref The object to compare with this
			@return True if ref is equal to this
		*/
		bool operator== (const Participant& ref) const { return (id && ref.id) ? (id == ref.id) : false; }	//Only match on ID
		/*!
			Inequality operator
			@param ref The object to compare with this
			@return True if ref is not equal to this
		*/
		bool operator!= (const Participant& ref) const { return !(*this == ref); }
		
		/*!
			Audit the participant environment to ensure it can function
			@return True if the participant is able to function
		*/
		virtual bool audit() { return true; }
		/*!
			Attach participant components to the app (as required)
			@return True if the participant is able to function
		*/
		virtual bool attach() { return true; }
		/*!
			Start the participant operation
			@return True if the participant is able to continue
		*/
		virtual bool start() { return true; }
		/*!
			Stop participation (release resources etc)
		*/
		virtual void stop() {}
	};
	
}

#endif	//ACTIVE_EVENT_PARTICIPANT
