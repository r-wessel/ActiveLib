/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_EVENT_MESSAGE
#define ACTIVE_EVENT_MESSAGE

#include "Active/Setting/SettingList.h"

namespace active::event {

	class Message;

	/*!
		Base class for messages sent between event subscribers and authors
	*/
	class Message : public setting::SettingList {
	public:
		using SettingList::SettingList;
		/*!
			Destructor
		*/
		virtual ~Message() = default;
		
		/*!
			Clone method
			@return A clone of this object
		*/
		virtual Message* clonePtr() const { return new Message{*this}; }
	};
	
}

#endif	//ACTIVE_EVENT_MESSAGE
