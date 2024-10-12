#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Serialise/JSON/JSONTransport.h"
#include "Active/Serialise/Package/Wrapper/ContainerWrap.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPolyPoint.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "ActiveLibDoctest/Serialisation/SerialiseArrayTester.h"
#include "ActiveLibDoctest/Serialisation/SerialiseTester.h"

using namespace active;
using namespace active::geometry;
using namespace active::math;
using namespace active::serialise::json;
using namespace active::serialise::xml;
using namespace active::utility;

using enum active::utility::TextEncoding;

namespace {
	
		///Make a report for the specified JSON tranport status
	String makeReportFor(const JSONTransport& transport, const String& errorMessage) {
		return errorMessage + " at row: " +String{transport.getLastRow()} + ", column: " + String{transport.getLastColumn()};
	}
	
		//A string for testing special chars in JSON transport
	auto shapeName = u8"ਖdਖdਖdਖdabਖdefghabਖdefgh<>&\"\\/";
	
	String unknownJSONName =
"{\n\
	\"name\": \"ਖdਖdਖdਖdabਖdefghabਖdef\\u0436\",\n\
	\"shape\": {\n\
		\"hole\": [\n\
			{\n\
				\"vertex\": [\n\
					{\n\
						\"x\": 1.345,\n\
						\"y\": 2.456\n\
					},\n\
					{\n\
						\"x\": 3.012,\n\
						\"y\": 2.987\n\
					},\n\
					{\n\
						\"x\": 1.543,\n\
						\"y\": 3.321\n\
					}\n\
				],\n\
				\"topID\": 6\n\
			}\n\
		],\n\
		\"verte\": [\n\
			{\n\
				\"x\": 1.234,\n\
				\"y\": 2.345\n\
			},\n\
			{\n\
				\"x\": 3.456,\n\
				\"y\": 2.987\n\
			},\n\
			{\n\
				\"x\": 1.876,\n\
				\"y\": 4.765\n\
			}\n\
		],\n\
		\"topID\": 6\n\
	}\n\
}";
String missingQuote =
"{\n\
	\"name\": \"ਖdਖdਖdਖdabਖdefghabਖdef,\n\
	\"shape\": {\n\
		\"hole\": [\n\
			{\n\
				\"vertex\": [\n\
					{\n\
						\"x\": 1.345,\n\
						\"y\": 2.456\n\
					},\n\
					{\n\
						\"x\": 3.012,\n\
						\"y\": 2.987\n\
					},\n\
					{\n\
						\"x\": 1.543,\n\
						\"y\": 3.321\n\
					}\n\
				],\n\
				\"topID\": 6\n\
			}\n\
		],\n\
		\"vertex\": [\n\
			{\n\
				\"x\": 1.234,\n\
				\"y\": 2.345\n\
			},\n\
			{\n\
				\"x\": 3.456,\n\
				\"y\": 2.987\n\
			},\n\
			{\n\
				\"x\": 1.876,\n\
				\"y\": 4.765\n\
			}\n\
		],\n\
		\"topID\": 6\n\
	}\n\
}";
String badNumber =
"{\n\
	\"name\": \"ਖdਖdਖdਖdabਖdefghabਖdef\",\n\
	\"shape\": {\n\
		\"hole\": [\n\
			{\n\
				\"vertex\": [\n\
					{\n\
						\"x\": 1.345,\n\
						\"y\": 2.456\n\
					},\n\
					{\n\
						\"x\": 3.012,\n\
						\"y\": 2.987\n\
					},\n\
					{\n\
						\"x\": 1.543,\n\
						\"y\": 3.321\n\
					}\n\
				],\n\
				\"topID\": 6\n\
			}\n\
		],\n\
		\"vertex\": [\n\
			{\n\
				\"x\": ABCD,\n\
				\"y\": 2.345\n\
			},\n\
			{\n\
				\"x\": 3.456,\n\
				\"y\": 2.987\n\
			},\n\
			{\n\
				\"x\": 1.876,\n\
				\"y\": 4.765\n\
			}\n\
		],\n\
		\"topID\": 6\n\
	}\n\
}";
String missingBrace =
"{\n\
	\"name\": \"ਖdਖdਖdਖdabਖdefghabਖdef\",\n\
	\"shape\": {\n\
		\"hole\": [\n\
			{\n\
				\"vertex\": [\n\
					{\n\
						\"x\": 1.345,\n\
						\"y\": 2.456\n\
					},\n\
					{\n\
						\"x\": 3.012,\n\
						\"y\": 2.987\n\
					},\n\
					{\n\
						\"x\": 1.543,\n\
						\"y\": 3.321\n\
					}\n\
				],\n\
				\"topID\": 6\n\
			}\n\
		],\n\
		\"vertex\": [\n\
			{\n\
				\"x\": 1.234,\n\
				\"y\": 2.345\n\
			},\n\
			{\n\
				\"x\": 3.456,\n\
				\"y\": 2.987\n\
			},\n\
			{\n\
				\"x\": 1.876,\n\
				\"y\": 4.765\n\
			}\n\
		],\n\
		\"topID\": 6\n\
	}\n\
";

}  // namespace


	///Tests for JSON serialisation
TEST_SUITE(TESTQ(JSONTest)) TEST_SUITE_OPEN
	
		///Tests for sending and receiving data via JSON
	TEST_CASE(TESTQ(testJSONSendReceive)) {
			//Initialise sample test data
		active::geometry::Polygon polyOut{PolyPoint{1.234, 2.345}, PolyPoint{3.456, 2.987, 0.0, math::pi / 8}, PolyPoint{1.876, 4.765}}, polyIn;
		polyOut.insertHole(active::geometry::Polygon{PolyPoint{1.345, 2.456}, PolyPoint{3.012, 2.987}, PolyPoint{1.543, 3.321, 0.0, math::pi / 20}});
		polyOut.renumber();
		JSONTransport transporter;
			//Initialise the object that will be transported via JSON
		SerialiseTester shapeOut{shapeName, polyOut}, shapeIn;
			//Test writing JSON to memory
		
			//Send the test object as JSON (into the collection string)
		Memory memCollector;
		try {
			transporter.send(PackageWrap{shapeOut}, SerialiseTester::tag, memCollector);
			CHECK_MESSAGE(!memCollector.empty(), TEST_MESSAGE(JSON send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF8) send failed));
		}
			//Receive the JSON data from the collection string into another object (auto-discovery of encoding)
		try {
			transporter.receive(PackageWrap{shapeIn}, SerialiseTester::tag, BufferIn{memCollector, std::nullopt});
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF8) receive failed));
		}
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE(shapeOut == shapeIn, TEST_MESSAGE(Object received via JSON (UTF8) does not match the object sent));
		
			//Send the test object as JSON (encoded as UTF16)
		memCollector.clear();
		try {
			transporter.send(PackageWrap{shapeOut}, SerialiseTester::tag, BufferOut{memCollector, std::nullopt, UTF16});
			CHECK_MESSAGE(!memCollector.empty(), TEST_MESSAGE(JSON send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF16) send failed));
		}
			//Receive the JSON data from the collection string into another object (auto-discovery of encoding)
		try {
			shapeIn = SerialiseTester{};
			transporter.receive(PackageWrap{shapeIn}, SerialiseTester::tag, BufferIn{memCollector, std::nullopt});
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF16) receive failed));
		}
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE(shapeOut == shapeIn, TEST_MESSAGE(Object received via JSON (UTF16) does not match the object sent));
		
			//Send the test object as JSON (encoded as UTF16BE with BOM)
		memCollector.clear();
		try {
			transporter.send(PackageWrap{shapeOut}, SerialiseTester::tag, BufferOut{memCollector, std::nullopt, DataFormat{UTF16, true, true}});
			CHECK_MESSAGE(!memCollector.empty(), TEST_MESSAGE(JSON send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF16 with BOM) send failed));
		}
			//Receive the JSON data from the collection string into another object (auto-discovery of encoding)
		try {
			shapeIn = SerialiseTester{};
			transporter.receive(PackageWrap{shapeIn}, SerialiseTester::tag, BufferIn{memCollector, std::nullopt});
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF16 with BOM) receive failed));
		}
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE(shapeOut == shapeIn, TEST_MESSAGE(Object received via JSON (UTF16) does not match the object sent));
		
			//Send the test object as JSON ( (encoded as UTF32))
		memCollector.clear();
		try {
			transporter.send(PackageWrap{shapeOut}, SerialiseTester::tag, BufferOut{memCollector, std::nullopt, UTF32});
			CHECK_MESSAGE(!memCollector.empty(), TEST_MESSAGE(JSON send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF32) send failed));
		}
			//Receive the JSON data from the collection string into another object (auto-discovery of encoding)
		try {
			shapeIn = SerialiseTester{};
			transporter.receive(PackageWrap{shapeIn}, SerialiseTester::tag, BufferIn{memCollector, std::nullopt});
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON (UTF32) receive failed));
		}
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE(shapeOut == shapeIn, TEST_MESSAGE(Object received via JSON (UTF32) does not match the object sent));

			//Initialise sample test array data
		String collector;
		SerialiseArrayTester arrayTesterOut, arrayTesterIn;
		arrayTesterOut.emplace_back(BarA{Guid{true}, "Something"});
		arrayTesterOut.emplace_back(BarB{Guid{true}, 1.234});
		arrayTesterOut.emplace_back(BarA{Guid{true}, "Whatever"});
		arrayTesterOut.emplace_back(BarB{Guid{true}, 98.7654});
		try {
			transporter.send(SerialiseArrayWrapper{arrayTesterOut}, Identity{}, collector);
			CHECK_MESSAGE(!collector.empty(), TEST_MESSAGE(JSON send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON send failed));
		}
			//Receive the JSON data from the collection string into another object
		try {
			transporter.receive(SerialiseArrayWrapper{arrayTesterIn}, Identity{}, collector);
			CHECK_MESSAGE(arrayTesterOut == arrayTesterIn, TEST_MESSAGE(Array received via JSON does not match the array sent));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON receive failed));
		}

			//Negative tests (handle bad data)

		SerialiseTester testObject;
		String report;
			//Read JSON with an unknown name - use policy that rejects unknown names
		JSONTransport checkedTransport(Transport::Policy::moderate);
		try {
			checkedTransport.receive(PackageWrap{testObject}, SerialiseTester::tag, unknownJSONName);
			FAIL_CHECK(TEST_MESSAGE(JSON reader accepted input with an unknown name));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(checkedTransport, error.code().message()) == "An unknown name was found in the JSON at row: 24, column: 5",
						  TEST_MESSAGE(Failure report for input with an incorrect name wrong));
		}
			//Read JSON with a missing quote
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, missingQuote);
			FAIL_CHECK(TEST_MESSAGE(JSON reader accepted input with a missing quote));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "A scope has been started but not closed at row: 3, column: 4",
						  TEST_MESSAGE(Failure report for input with a missing quote wrong));
		}
			//Read JSON with an invalid number
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, badNumber);
			FAIL_CHECK(TEST_MESSAGE(JSON reader accepted input with a bad numeric value));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "An invalid value was found at row: 25, column: 12",
						  TEST_MESSAGE(Failure report for input with a bad numeric value wrong));
		}
			//Read JSON with a missing closing brace
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, missingBrace);
			FAIL_CHECK(TEST_MESSAGE(JSON reader accepted input with a missing closing brace));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "A scope has been opened but not closed at row: 39, column: 0",
						  TEST_MESSAGE(Failure report for input with a missing closing brace wrong));
		}
	} //testJSONSendReceive


		///Tests for sending and receiving items via JSON
	TEST_CASE(TESTQ(testJSONItem)) {
		JSONTransport transport;
		String importedText, inputText{"Some sample text"};
		transport.receive(ValueWrap<String>{importedText}, Identity{}, "\"" + inputText + "\"");
		CHECK_MESSAGE(importedText == inputText, TEST_MESSAGE(Text import from JSON does not match input));
		double importedNum = 0.0, inputNum = 1.234;
		transport.receive(ValueWrap<double>{importedNum}, Identity{}, String{inputNum});
		CHECK_MESSAGE(isEqual(importedNum, inputNum), TEST_MESSAGE(Double-precision import from JSON does not match input));
	} //testJSONItem


	///Tests for sending and receiving items via JSON
   TEST_CASE(TESTQ(testContainer)) {
	   JSONTransport transport;
	   	//Test string array
	   std::vector<String> test1{"Something", "Whatever", "more", "Testing"};
	   String json;
	   try {
		   transport.send(ContainerWrap{test1}, Identity{}, json);
	   } catch(std::system_error& error) {
		   FAIL_CHECK(TEST_MESSAGE(ContainerWrap failed JSON export of std::vector<String>));
	   }
	   std::vector<String> test1In;
	   try {
		   transport.receive(ContainerWrap{test1In}, Identity{}, json);
	   } catch(std::system_error& error) {
		   FAIL_CHECK(TEST_MESSAGE(ContainerWrap failed JSON import to std::vector<String>));
	   }
	   CHECK_MESSAGE(test1 == test1In, TEST_MESSAGE(String array JSON send/receive strings failed));
		   //Test guid array
	   std::vector<Guid> testg1{Guid{true}, Guid{true}, Guid{true}, Guid{true}};
	   json.clear();
	   try {
		   transport.send(ContainerWrap{testg1}, Identity{}, json);
	   } catch(std::system_error& error) {
		   FAIL_CHECK(TEST_MESSAGE(ContainerWrap failed JSON export of std::vector<Guid>));
	   }
	   std::vector<Guid> test1gIn;
	   try {
		   transport.receive(ContainerWrap{test1gIn}, Identity{}, json);
	   } catch(std::system_error& error) {
		   FAIL_CHECK(TEST_MESSAGE(ContainerWrap failed JSON import to std::vector<Guid>));
	   }
	   CHECK_MESSAGE(testg1 == test1gIn, TEST_MESSAGE(String array JSON send/receive guids failed));
		   //Test double array
	   json.clear();
	   std::vector<double> test2{1.0, 2.0, 3.14};
	   try {
		   transport.send(ContainerWrap{test2}, Identity{}, json);
	   } catch(std::system_error& error) {
		   FAIL_CHECK(TEST_MESSAGE(ContainerWrap failed JSON export of std::vector<double>));
	   }
	   std::vector<double> test2In;
	   try {
		   transport.receive(ContainerWrap{test2In}, Identity{}, json);
	   } catch(std::system_error& error) {
		   FAIL_CHECK(TEST_MESSAGE(ContainerWrap failed JSON import to std::vector<double>));
	   }
	   CHECK_MESSAGE(test2 == test2In, TEST_MESSAGE(String array JSON send/receive doubles failed));
   } //testContainer

TEST_SUITE_CLOSE //JSONTest
