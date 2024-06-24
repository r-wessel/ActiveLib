#include "ActiveLibDoctest/TestingPlatforms.h"

#include "ActiveLibDoctest/EventTests/Button.h"
#include "ActiveLibDoctest/EventTests/Calculation.h"
#include "ActiveLibDoctest/EventTests/CalcApp.h"
#include "ActiveLibDoctest/EventTests/CalcIdentity.h"
#include "ActiveLibDoctest/EventTests/CalculatorRPN.h"

using namespace active;
using namespace active::math;

TEST_SUITE(TESTQ(EventTests)) TEST_SUITE_OPEN

		//Test event handling
	TEST_CASE(TESTQ(testEventHandling)) {
			//Make some buttons for a simulated UI
		Button zero{digitID, "0"}, one{digitID, "1"}, two{digitID, "2"}, three{digitID, "3"}, four{digitID, "4"}, five{digitID, "5"},
				six{digitID, "6"}, seven{digitID, "7"}, eight{digitID, "8"}, nine{digitID, "9"}, enter{digitID, "\n"}, del{digitID, "\b"},
				plus{operationID, addID}, minus{operationID, subtractID}, multiply{operationID, multiplyID}, divide{operationID, divideID};
			//Define the calculation operator function handlers
		calcApp->add<Add>();
		calcApp->add<Subtract>();
		calcApp->add<Multiply>();
		calcApp->add<Divide>();
			//Make a new a calculator and add it to the app
		auto calculator = calcApp->add<CalculatorRPN>();
			//Simulate the user pressing buttons
		one();
		two();
		CHECK_MESSAGE(math::isEqual(calculator->displayValue(), 12.0), TEST_MESSAGE(Keyed entry wrong));
			//Simulate pressing 'enter'
		enter();
		CHECK_MESSAGE(math::isZero(calculator->displayValue()), TEST_MESSAGE(Entering value does not reset to zero));
		three();
		four();
		CHECK_MESSAGE(math::isEqual(calculator->displayValue(), 34.0), TEST_MESSAGE(Keyed entry wrong));
			//Add the first and second numbers
		plus();
		CHECK_MESSAGE(math::isEqual(calculator->displayValue(), 46.0), TEST_MESSAGE(Addition total wrong));
		enter();
		six();
		multiply();
		CHECK_MESSAGE(math::isEqual(calculator->displayValue(), 276.0), TEST_MESSAGE(Multiplication calc wrong));
		enter();
		four();
		divide();
		CHECK_MESSAGE(math::isEqual(calculator->displayValue(), 69.0), TEST_MESSAGE(Division calc wrong));
			//Make a divide-by-zero error
		enter();
		zero();
		divide();
		CHECK_MESSAGE(calculator->display() == "E", TEST_MESSAGE(Divide-by-zero did not display an error));
	}

TEST_SUITE_CLOSE
