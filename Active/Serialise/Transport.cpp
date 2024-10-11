/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Serialise/Transport.h"

using namespace active::serialise;
using namespace active::utility;

/*--------------------------------------------------------------------
	Default constructor
 
	policy: Content policy, i.e. unknown instances and requirements policy
	timeFormat: The serialisation date/time format (nullopt = use the format specified in each outgoing item)
  --------------------------------------------------------------------*/
Transport::Transport(Policy policy, TimeFormat timeFormat) noexcept {
	switch (policy) {
		case verbose:
			m_isEveryEntryRequired = true;
			[[fallthrough]];
		case strict:
			m_isMissingEntryFailed = true;
			[[fallthrough]];
		case moderate:
			m_isUnknownNameSkipped = false;
			break;
		default:
			break;	//NB: Default member variable settings equate to relaxed policy
	}
	m_timeFormat = timeFormat;
} //Transport::Transport
