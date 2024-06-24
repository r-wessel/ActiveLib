#ifndef ACTIVE_EVENT_TESTING_CALCULATOR_RPN
#define ACTIVE_EVENT_TESTING_CALCULATOR_RPN

#include "Active/Event/Event.h"
#include "Active/Event/Subscriber.h"

#include <vector>

	///RPN calculator class
class CalculatorRPN : public active::event::Subscriber {
public:
	CalculatorRPN() : m_valueStack{0} {}
	
		///Receive a subscribed event
	bool receive(const active::event::Event& event) override;
		///Subscriptions required - in this case user input of either digits or calculator operations
	Subscription subscription() const override;
		///The current calculator display
	const active::utility::String& display() const { return m_display; }
		///The (numeric) value of the calculator display
	double displayValue() const { return m_valueStack.back(); }
	
private:
		///Refresh the display based on the most recent value
	void refreshDisplay();
		///Enter a digit as specified by an event
	void enterDigit(const active::event::Event& event) ;
		///Perform an operation as specified by an event
	void performOperation(const active::event::Event& event);
	
	active::utility::String m_display;
	std::vector<double> m_valueStack;
};

#endif //ACTIVE_EVENT_TESTING_CALCULATOR_RPN
