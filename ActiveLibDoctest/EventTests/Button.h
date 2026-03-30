#ifndef ACTIVE_EVENT_TESTING_BUTTON
#define ACTIVE_EVENT_TESTING_BUTTON

#include "Active/Utility/NameID.h"

	///Base class for simulated calculator UI buttons
class Button {
public:
	/*!
		Constructor
		@param type The event type emitted when pressed
		@param action The action setting attached to an event when pressed
	*/
	Button(active::NameID type, active::string action);
	
		///Trigger a button press
	void operator()() const;

private:
	active::NameID m_type;
	active::string m_action;
};

#endif //ACTIVE_EVENT_TESTING_BUTTON
