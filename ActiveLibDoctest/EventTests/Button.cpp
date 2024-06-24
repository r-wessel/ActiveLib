#include "ActiveLibDoctest/EventTests/Button.h"

#include "Active/Event/Event.h"
#include "Active/Setting/Values/StringValue.h"
#include "ActiveLibDoctest/EventTests/CalcApp.h"
#include "ActiveLibDoctest/EventTests/CalcIdentity.h"

using namespace active;
using namespace active::event;
using namespace active::setting;
using namespace active::utility;

Button::Button(utility::NameID type, String action) : m_type{type}, m_action{action} {}

void Button::operator()() const {
	calcApp->publish(Event{m_type, {{StringValue{m_action}, actionID}}});
}
