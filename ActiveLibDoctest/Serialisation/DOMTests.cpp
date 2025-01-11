#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Serialise/DOM/Node.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/JSON/JSONTransport.h"
#include "Active/Serialise/XML/XMLTransport.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/MathFunctions.h"

#include <array>

using namespace active;
using namespace active::math;
using namespace active::serialise;
using namespace active::serialise::dom;
using namespace active::serialise::json;
using namespace active::serialise::xml;
using namespace active::setting;
using namespace active::utility;

	///Tests for DOM (de)serialisation
TEST_SUITE(TESTQ(DOMTest)) TEST_SUITE_OPEN
	
	struct TestNode {
		String a;
		double b = 0.0;
		uint32_t c = 0;
	};

	Node& operator << (Node& node, const TestNode& test) {
		node =  Object{};
		node["a"] = test.a;
		node["b"] = test.b;
		node["c"] = test.c;
		return node;
	}

	const Node& operator >> (const Node& node, TestNode& test) {
		test.a = node["a"].operator String();
		test.b = node["b"];
		test.c = node["c"];
		return node;
	}


		///Make a DOM mode populated with content for testing
	Node makeNode() {
		Node node(Object{});
		node["boolean"] = true;
		node["integer"] = 5;
		node["double"] = 1.23;
		node["string"] = "Test";
		(node["array"] = std::vector{ 1, 2, 3, 4, 5, 6 }).withItemTag("val");
		return node;
	}


		///Test the content and structure of an imported DOM node
	void testNode(const Node& node, size_t childSize = 8) {
		if (node.index() == Node::Index::object) {
			bool boolValue = node["boolean"];
			int64_t intValue = node["integer"];
			double doubleValue = node["double"];
			utility::String stringValue = node["string"];
			CHECK_MESSAGE(node.object().size() == childSize, TEST_MESSAGE(DOM node import has wrong number of items));
			CHECK_MESSAGE(boolValue == true, TEST_MESSAGE(DOM node import has failed to import a boolean value));
			CHECK_MESSAGE(intValue == 5, TEST_MESSAGE(DOM node import has failed to import an integer value));
			CHECK_MESSAGE(isEqual(doubleValue, 1.23), TEST_MESSAGE(DOM node import has failed to import a double value));
			CHECK_MESSAGE(stringValue == "Test", TEST_MESSAGE(DOM node import has failed to import a string value));
			if (auto iter = node.object().find("array"); (iter != node.object().end()) && (iter->second.index() == Node::Index::array) &&
				(iter->second.array().size() == 6)) {
				auto sourceArray{iter->second.array()};
				for (auto i = 0; i < sourceArray.size(); ++i) {
					int64_t intItem = sourceArray[i];
					CHECK_MESSAGE(intItem == i + 1, TEST_MESSAGE(DOM node import has failed to import a value in an array item));
				}
					
			} else
				FAIL_CHECK(TEST_MESSAGE(DOM node import has failed to import an array));
		} else
			FAIL_CHECK(TEST_MESSAGE(DOM node import has failed to import an object at the root));
	}


		///Tests for sending and receiving data via a DOM
	TEST_CASE(TESTQ(testDOMContent)) {
			//Assemble content to be serialised
		Node root(makeNode());
		root["object"] = makeNode();
		root["ad-hoc"] = { 1.2, 2.3, "text", 1, false };
		root["assign"] = TestNode{"something", 1.23, 25};
			//Test DOM i/o via JSON
		String json;
		JSONTransport().send(std::forward<Cargo&&>(root), Identity{}, json);
		CHECK_MESSAGE(json.contains("\"boolean\":true"), TEST_MESSAGE(DOM node export to JSON failed with boolean value));
		CHECK_MESSAGE(json.contains("\"integer\":5"), TEST_MESSAGE(DOM node export to JSON failed with integer value));
		CHECK_MESSAGE(json.contains("\"double\":1.23"), TEST_MESSAGE(DOM node export to JSON failed with double value));
		CHECK_MESSAGE(json.contains("\"string\":\"Test\""), TEST_MESSAGE(DOM node export to JSON failed with string value));
		CHECK_MESSAGE(json.contains("\"array\":[1,2,3,4,5,6]"), TEST_MESSAGE(DOM node export to JSON failed with array));
		CHECK_MESSAGE(json.contains("\"ad-hoc\":[1.2,2.3,\"text\",1,false]"), TEST_MESSAGE(DOM node export to JSON failed with ad-hoc values));
		Node fromJSON;
		JSONTransport().receive(std::forward<Cargo&&>(fromJSON), Identity{}, json);
		TestNode assigned = fromJSON["assign"];
		testNode(fromJSON);
		testNode(fromJSON["object"], 5);
			//Test DOM i/o via XML
		String xml;
		XMLTransport().send(std::forward<Cargo&&>(root), Identity{"testing"}, xml);
		CHECK_MESSAGE(xml.contains("<boolean>true</boolean>"), TEST_MESSAGE(DOM node export to XML failed with boolean value));
		CHECK_MESSAGE(xml.contains("<integer>5</integer>"), TEST_MESSAGE(DOM node export to XML failed with integer value));
		CHECK_MESSAGE(xml.contains("<double>1.23</double>"), TEST_MESSAGE(DOM node export to XML failed with double value));
		CHECK_MESSAGE(xml.contains("<string>Test</string>"), TEST_MESSAGE(DOM node export to XML failed with string value));
		CHECK_MESSAGE(xml.contains("<array><val>1</val><val>2</val><val>3</val><val>4</val><val>5</val><val>6</val></array>"),
					  TEST_MESSAGE(DOM node export to XML failed with array));
		CHECK_MESSAGE(xml.contains("<ad-hoc><item>1.2</item><item>2.3</item><item>text</item><item>1</item><item>false</item></ad-hoc>"),
					  TEST_MESSAGE(DOM node export to XML failed with ad-hoc values));
		Node fromXML;
		XMLTransport().receive(std::forward<Cargo&&>(fromXML), Identity{"testing"}, xml);
		assigned = fromXML["assign"];
		testNode(fromXML);
		testNode(fromXML["object"], 5);
	}

TEST_SUITE_CLOSE
