#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Serialise/Package/PackageWrap.h"
#include "Active/Serialise/XML/Package/Wrapper/Geometry/XMLPolyPoint.h"
#include "Active/Serialise/XML/XMLTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "ActiveLibDoctest/Serialisation/SerialiseArrayTester.h"
#include "ActiveLibDoctest/Serialisation/SerialiseTester.h"

using namespace active;
using namespace active::geometry;
using namespace active::math;
using namespace active::serialise;
using namespace active::serialise::xml;
using namespace active::utility;

namespace {

		///Make a report for the specified XML transport status
	String makeReportFor(const XMLTransport& transport, const String& errorMessage) {
		return errorMessage + " at row: " +String{transport.getLastRow()} + ", column: " + String{transport.getLastColumn()};
	}

		//A string for testing special chars in XML transport
	auto shapeName = u8"\u0A16d\u0A16d\u0A16d\u0A16dab\u0A16defghab\u0A16defgh<>&\"";
	
	String badXMLTag =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<tester name=\"ਖdਖdਖdਖdabਖdefghabਖdefgh&lt;&gt;&amp;&quot;\">\n\
	<shape topID=\"6\">\n\
		<hole topID=\"6\">\n\
			<vertex x=\"1.345\" y=\"2.456\" />\n\
			<vertex x=\"3.012\" y=\"2.987\" />\n\
			<vertex x=\"1.543\" y=\"3.321\" />\n\
		</hole>\n\
		<vertex x=\"1.234\" y=\"2.345\" />\n\
		<vertex x=\"3.456\" y=\"2.987\" />\n\
		<vertex x=\"1.876\" y=\"4.765\" />\n\
	</shap>\n\
 </tester>";

	String missingQuote =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<tester name=\"ਖdਖdਖdਖdabਖdefghabਖdefgh&lt;&gt;&amp;&quot;\">\n\
	<shape topID=\"6\">\n\
		<hole topID=\"6\">\n\
			<vertex x=\"1.345\" y=\"2.456\" />\n\
			<vertex x=\"3.012\" y=\"2.987\" />\n\
			<vertex x=\"1.543\" y=\"3.321\" />\n\
		</hole>\n\
		<vertex x=\"1.234\" y=\"2.345\" />\n\
		<vertex x=\"3.456\" y=\"2.987\" />\n\
		<vertex x=\"1.876\" y=\"4.765 />\n\
	</shape>\n\
 </tester>";
	String badCharacter =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<tester name=\"ਖdਖdਖdਖdabਖdefghabਖdefgh&lt;&xx;&amp;&quot;\">\n\
	<shape topID=\"6\">\n\
		<hole topID=\"6\">\n\
			<vertex x=\"1.345\" y=\"2.456\" />\n\
			<vertex x=\"3.012\" y=\"2.987\" />\n\
			<vertex x=\"1.543\" y=\"3.321\" />\n\
		</hole>\n\
		<vertex x=\"1.234\" y=\"2.345\" />\n\
		<vertex x=\"3.456\" y=\"2.987\" />\n\
		<vertex x=\"1.876\" y=\"4.765\" />\n\
	</shape>\n\
 </tester>";
	Memory corrupt{(void*)
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<tester name=\"ਖdਖdਖdਖdabਖdefghabਖdefgh&lt;&amp;&quot;\">\n\
	<shape topID=\"6\">\n\
		<hole topID=\"6\">\n\
			<vertex x=\"1.345\" y=\"2.456\" />\n\
			<vertex x=\"3.012\" y=\"2.987\" />\n\
			<vertex x=\"1.543\" y=\"3.321\" />\n\
		</hole>\n\
		<vertex x=\"1.234\" y=\"2.345\" />\n\
		<vertex x=\"3.456\" y=\"2.987\" />\n\
		<vertex x=\"1.876\" y=\"4.765\0\" />\n\
	</shape>\n\
 </tester>", 0};
	Memory badEncoding{(void*)
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
 <tester name=\"abc\xe2\x28\xa1\">\n\
	<shape topID=\"6\">\n\
		<hole topID=\"6\">\n\
			<vertex x=\"1.345\" y=\"2.456\" />\n\
			<vertex x=\"3.012\" y=\"2.987\" />\n\
			<vertex x=\"1.543\" y=\"3.321\" />\n\
		</hole>\n\
		<vertex x=\"1.234\" y=\"2.345\" />\n\
		<vertex x=\"3.456\" y=\"2.987\" />\n\
		<vertex x=\"1.876\" y=\"4.765\" />\n\
	</shape>\n\
 </tester>", 0};
	String badNameChar =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<tester name=\"ਖdਖdਖdਖdabਖdefghabਖdefgh&lt;&gt;&amp;&quot;\">\n\
	<shape topID=\"6\">\n\
		<9hole topID=\"6\">\n\
			<vertex x=\"1.345\" y=\"2.456\" />\n\
			<vertex x=\"3.012\" y=\"2.987\" />\n\
			<vertex x=\"1.543\" y=\"3.321\" />\n\
		</9hole>\n\
		<vertex x=\"1.234\" y=\"2.345\" />\n\
		<vertex x=\"3.456\" y=\"2.987\" />\n\
		<vertex x=\"1.876\" y=\"4.765\" />\n\
	</shape>\n\
 </tester>";
	
}  // namespace

	///Tests for XML serialisation
TEST_SUITE(TESTQ(XMLTest)) TEST_SUITE_OPEN
	
		///Tests for sending and receiving data via XML
	TEST_CASE(TESTQ(testXMLSendReceive)) {
			//Initialise sample test data
		active::geometry::Polygon polyOut{PolyPoint{1.234, 2.345}, PolyPoint{3.456, 2.987}, PolyPoint{1.876, 4.765}}, polyIn;
		polyOut.insertHole(active::geometry::Polygon{PolyPoint{1.345, 2.456}, PolyPoint{3.012, 2.987}, PolyPoint{1.543, 3.321}});
		polyOut.renumber();
		XMLTransport transporter;
			//Initialise the object that will be transported via XML
		SerialiseTester shapeOut{shapeName, polyOut}, shapeIn;
			//Allocate a string to hold the outgoing XML
		String collector;
			//Send the test object as XML (into the collection string)
		try {
			transporter.send(PackageWrap{shapeOut}, SerialiseTester::tag, collector);
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(XML send failed));
		}
		CHECK_MESSAGE(!collector.empty(), TEST_MESSAGE(XML send produced no output));
			//Receive the XML data from the collection string into another object
		try {
			transporter.receive(PackageWrap{shapeIn}, SerialiseTester::tag, collector);
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(XML receive failed));
		}
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE((shapeIn == shapeOut), TEST_MESSAGE(Object received via XML does not match the object sent));
			//Initialise sample test array data
		collector.clear();
		SerialiseArrayTester arrayTesterOut, arrayTesterIn;
		arrayTesterOut.emplace_back(BarA{Guid{true}, "Something"});
		arrayTesterOut.emplace_back(BarB{Guid{true}, 1.234});
		arrayTesterOut.emplace_back(BarA{Guid{true}, "Whatever"});
		arrayTesterOut.emplace_back(BarB{Guid{true}, 98.7654});
		try {
			transporter.send(SerialiseArrayWrapper{arrayTesterOut}, SerialiseArrayWrapper::tag, collector);
			CHECK_MESSAGE(!collector.empty(), TEST_MESSAGE(XML send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(XML send failed));
		}
			//Receive the XML data from the collection string into another object
		try {
			transporter.receive(SerialiseArrayWrapper{arrayTesterIn}, SerialiseArrayWrapper::tag, collector);
			CHECK_MESSAGE(arrayTesterOut == arrayTesterIn, TEST_MESSAGE(Array received via XML does not match the array sent));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(XML receive failed));
		}
		
			//Negative tests (handle bad data)

		SerialiseTester testObject;
		String report;
			//Read XML with mismatching tag
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, badXMLTag);
			FAIL_CHECK(TEST_MESSAGE(XML reader accepted input with mismatching tags));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "A closing tag is missing at row: 12, column: 9",
						  TEST_MESSAGE(Failure report for input with mismatching tags wrong));
		}
			//Read XML with missing attribute quote
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, missingQuote);
			FAIL_CHECK(TEST_MESSAGE(XML reader accepted attribute with missing quote));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "The equals character is missing in an attribute at row: 11, column: 32",
						  TEST_MESSAGE(Failure report for input withmissing attribute quote wrong));
		}
			//Read XML with a bad XML character
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, badCharacter);
			FAIL_CHECK(TEST_MESSAGE(XML reader accepted a bad XML character));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "Found an unknown or invalid escaped character at row: 2, column: 72",
						  TEST_MESSAGE(Failure report for input with a bad XML character wrong));
		}
			//Read XML with corrupt data (null char)
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, corrupt);
			FAIL_CHECK(TEST_MESSAGE(XML reader accepted XML containing corrupt data));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "The closing brace for a tag is missing (>) at row: 11, column: 29",
						  TEST_MESSAGE(Failure report for input with corrupt data (null char) wrong));
		}
			//Read XML with bad utf8 encoding
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, badEncoding);
			FAIL_CHECK(TEST_MESSAGE(XML reader accepted XML with bad UTF8 encoding));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "The closing brace for a tag is missing (>) at row: 2, column: 19",
						  TEST_MESSAGE(Failure report for input with bad utf8 encoding wrong));
		}
			//Read XML with an invalid char in tag
		try {
			transporter.receive(PackageWrap{testObject}, SerialiseTester::tag, badNameChar);
			FAIL_CHECK(TEST_MESSAGE(XML reader accepted XML with an invalid char in tag));
		} catch(std::system_error& error) {
			CHECK_MESSAGE(makeReportFor(transporter, error.code().message()) == "An invalid XML tag was processed at row: 4, column: 20",
						  TEST_MESSAGE(Failure report for input with an invalid char in tag wrong));
		}
	}

TEST_SUITE_CLOSE
