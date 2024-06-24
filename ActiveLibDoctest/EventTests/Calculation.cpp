#include "ActiveLibDoctest/EventTests/Calculation.h"

#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Setting/Values/Int32Value.h"
#include "Active/Setting/ValueSetting.h"
#include "ActiveLibDoctest/EventTests/CalcIdentity.h"

using namespace active;
using namespace active::event;
using namespace active::math;
using namespace active::setting;
using namespace active::utility;

	///Receive a subscribed event - return true if event request is handled by this tool
bool Calculation::receive(const Event& event) {
	if (auto stack = event.findValue(parameterID); (stack != nullptr) && (stack->size() >= depth())) {
			//Attempt to perform the calculation
		if (auto message = perform(*stack); message) {
				//Add the number of consumed figures to the return message
			message->emplace_back(ValueSetting{Int32Value{depth()}, usedID});
				//Send the result to the author
			event.tellAuthor(std::move(*message));
			return true;
		}
	}
	return false;
}

Subscriber::Subscription Add::subscription() const { return { addID }; }

std::optional<Message> Add::perform(const ValueSetting& stack) {
	return Message{{DoubleValue{stack.doubleVal(stack.size() - 2) + stack.doubleVal(stack.size() - 1)}, resultID}};
}

Subscriber::Subscription Subtract::subscription() const { return { subtractID }; }
	
std::optional<Message> Subtract::perform(const ValueSetting& stack) {
	return Message{{DoubleValue{stack.doubleVal(stack.size() - 2) - stack.doubleVal(stack.size() - 1)}, resultID}};
}

Subscriber::Subscription Multiply::subscription() const { return { multiplyID }; }

std::optional<Message> Multiply::perform(const ValueSetting& stack) {
	return Message{{DoubleValue{stack.doubleVal(stack.size() - 2) * stack.doubleVal(stack.size() - 1)}, resultID}};
}

Subscriber::Subscription Divide::subscription() const { return { divideID }; }

std::optional<Message> Divide::perform(const ValueSetting& stack) {
	auto divisor = stack.doubleVal(stack.size() - 1);
	if (isZero(divisor))
		return std::nullopt;
	return Message{{DoubleValue{stack.doubleVal(stack.size() - 2) / divisor}, resultID}};
}
