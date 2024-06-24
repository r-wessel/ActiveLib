#ifndef ACTIVE_EVENT_TESTING_CALC
#define ACTIVE_EVENT_TESTING_CALC

#include "Active/Event/Event.h"
#include "Active/Event/Subscriber.h"

	///Base class for tools that provide calculation services
class Calculation : public active::event::Subscriber {
public:
		///Get the stack depth consumed by the calculation
	virtual int32_t depth() const { return 2; }
		///Receive a subscribed event - return true if event request is handled by this tool
	bool receive(const active::event::Event& event) override;
		///Perform the calculation and return the result as a message (nullopt on failure)
	virtual std::optional<active::event::Message> perform(const active::setting::ValueSetting& stack) { return std::nullopt; }
};

	///Addition tool
class Add : public Calculation {
public:
	Subscription subscription() const override;
	std::optional<active::event::Message> perform(const active::setting::ValueSetting& stack) override;
};

	///Subtraction tool
class Subtract : public Calculation {
public:
	Subscription subscription() const override;
	std::optional<active::event::Message> perform(const active::setting::ValueSetting& stack) override;
};

	///Multiplication tool
class Multiply : public Calculation {
public:
	Subscription subscription() const override;
	std::optional<active::event::Message> perform(const active::setting::ValueSetting& stack) override ;
};

	///Division tool
class Divide : public Calculation {
public:
	Subscription subscription() const override;
	std::optional<active::event::Message> perform(const active::setting::ValueSetting& stack) override;
};

#endif //ACTIVE_EVENT_TESTING_CALC
