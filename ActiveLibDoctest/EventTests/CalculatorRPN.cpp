#include "ActiveLibDoctest/EventTests/CalculatorRPN.h"

#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Setting/ValueSetting.h"
#include "ActiveLibDoctest/EventTests/CalcApp.h"
#include "ActiveLibDoctest/EventTests/CalcIdentity.h"

using namespace active;
using namespace active::event;
using namespace active::math;
using namespace active::setting;
using namespace active::utility;

	///Receive a subscribed event
bool CalculatorRPN::receive(const Event& event) {
	if (event == digitID)
		enterDigit(event);
	else
		performOperation(event);
	return true;
}


	///Subscriptions required - in this case user input of either digits or calculator operations
Subscriber::Subscription CalculatorRPN::subscription() const { return { digitID, operationID }; };


	///Refresh the display based on the most recent value
void CalculatorRPN::refreshDisplay() {
	m_display = String(m_valueStack.back());
}


	///Enter a digit as specified by an event
void CalculatorRPN::enterDigit(const Event& event) {
	if (auto action = event.findValue(actionID); (action != nullptr) && !action->empty()) {
		String newDisplay{m_display};
		if (action->stringVal() == "\b") {
			if (!newDisplay.empty())
				newDisplay.popBack();
		} else if (action->stringVal() == "\n") {
			m_valueStack.push_back(0.0);
			newDisplay = "0";
		} else
			newDisplay.append(action->stringVal());
		DoubleValue newValue{newDisplay};
		if (newValue.isGood()) {
			m_valueStack.back() = newValue;
			refreshDisplay();
		}
	}
}


	///Perform an operation as specified by an event
void CalculatorRPN::performOperation(const Event& event) {
		//Get the operator action from the event
	if (auto action = event.findValue(actionID); (action != nullptr) && !action->empty()) {
		PostBox response;
			//Publish an event seeking a calculation
		if (calcApp->publish(Event{action->stringVal(), { ValueSetting{m_valueStack, parameterID} }, &response}) && (response.size() == 1)) {
				//Seek the calculation result and number of stack values used
			if (auto result = response.front()->findValue(resultID), used = response.front()->findValue(usedID);
						(result != nullptr) && (used != nullptr) && !result->empty() && result->isGood()) {
					//Pop used values off the stack
				for (auto i = used->int32Val(); i--; )
					m_valueStack.pop_back();
					//Push the result onto the stack and refresh the display accordingly
				m_valueStack.push_back(result->doubleVal());
				refreshDisplay();
				return;
			}
		}
			//An error has occurred if we reach this point - clear the value stack and display an error code
		m_valueStack.clear();
		m_valueStack.push_back(0);
		m_display = "E";	//Display an error
	}
}
