#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Serialise/DOM/Node.h"
#include "Active/Serialise/JSON/JSONTransport.h"
#include "Active/Serialise/XML/XMLTransport.h"
#include "Active/Setting/Values/DoubleValue.h"
#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/MathFunctions.h"

#include <vector>
#include <map>

#include <iostream>

using namespace active;
using namespace active::math;
using namespace active::serialise;
using namespace active::serialise::dom;
using namespace active::serialise::json;
using namespace active::serialise::xml;
using namespace active::setting;
using namespace active::utility;

namespace {
	
	struct TestNode {
		String a;
		double b = 0.0;
		uint32_t c = 0;
	};
	
}

namespace active::serialise::dom {
	
		//Pack a TestNode into a dom::Node
	Node& pack(Node& node, const TestNode& test) {
		node = Object{};	//Ensure the node is an object type
		node["a"] = test.a;
		node["b"] = test.b;
		node["c"] = test.c;
		return node;
	}

		//Unpack a TestNode from a dom::Node
	const Node& unpack(const Node& node, TestNode& test) {
		test.a = node["a"].operator String();
		test.b = node["b"];
		test.c = node["c"];
		return node;
	}

}

	///Tests for DOM (de)serialisation
TEST_SUITE(TESTQ(DOMTest)) TEST_SUITE_OPEN

		///Make a DOM mode populated with content for testing
	Node makeNode() {
		Node node(Object{});
		node["boolean"] = true;
		node["integer"] = 5;
		node["double"] = 1.23;
		node["string"] = "Test";
		(node["array"] = std::vector{ 1, 2, 3, 4, 5, 6 }).withItemTag("val");
		node["map"] = std::map<utility::String, int32_t>{
			{ "first", 1},
			{ "second", 2},
			{ "third", 3},
		};
		return node;
	}


		///Test the content and structure of an imported DOM node
	void validateNode(const Node& node, size_t childSize = 10) {
		if (node.index() == Node::Index::object) {
			bool boolValue = node["boolean"];
			int64_t intValue = node["integer"];
			double doubleValue = node["double"];
			utility::String stringValue = node["string"];
			auto doubleSetting = node.setting("double");
			auto missingSetting = node.setting("nonexistent");
			CHECK_MESSAGE(node.object().size() == childSize, TEST_MESSAGE(DOM node import has wrong number of items));
			CHECK_MESSAGE(boolValue == true, TEST_MESSAGE(DOM node import has failed to import a boolean value));
			CHECK_MESSAGE(intValue == 5, TEST_MESSAGE(DOM node import has failed to import an integer value));
			CHECK_MESSAGE(isEqual(doubleValue, 1.23), TEST_MESSAGE(DOM node import has failed to import a double value));
			CHECK_MESSAGE(stringValue == "Test", TEST_MESSAGE(DOM node import has failed to import a string value));
			bool isDoubleOk = doubleSetting.operator bool() && isEqual(*doubleSetting, 1.23);
			CHECK_MESSAGE(isDoubleOk, TEST_MESSAGE(DOM node import failed to find setting));
			CHECK_MESSAGE(!missingSetting.operator bool(), TEST_MESSAGE(DOM node import found non-existent setting));
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
		root["ad-hocObj"] = Object{
			{"first", 1},
			{"second", 2},
			{"third", 1.23},
			{"fourth", "testing"},
		};
		root["assign"] = TestNode{"something", 1.23, 25};
			//Test DOM i/o via JSON
		String json;
		JSONTransport().send(root, Identity{}, json);
		CHECK_MESSAGE(json.contains("\"boolean\":true"), TEST_MESSAGE(DOM node export to JSON failed with boolean value));
		CHECK_MESSAGE(json.contains("\"integer\":5"), TEST_MESSAGE(DOM node export to JSON failed with integer value));
		CHECK_MESSAGE(json.contains("\"double\":1.23"), TEST_MESSAGE(DOM node export to JSON failed with double value));
		CHECK_MESSAGE(json.contains("\"string\":\"Test\""), TEST_MESSAGE(DOM node export to JSON failed with string value));
		CHECK_MESSAGE(json.contains("\"array\":[1,2,3,4,5,6]"), TEST_MESSAGE(DOM node export to JSON failed with array));
		CHECK_MESSAGE(json.contains("\"ad-hoc\":[1.2,2.3,\"text\",1,false]"), TEST_MESSAGE(DOM node export to JSON failed with ad-hoc array));
		bool isArrayOk = json.contains("\"ad-hocObj\":{") && json.contains("\"first\":1") &&
					json.contains("\"second\":2") && json.contains("\"third\":1.23") &&
					json.contains("\"fourth\":\"testing\"");
		CHECK_MESSAGE(isArrayOk, TEST_MESSAGE(DOM node export to JSON failed with ad-hoc object));
		Node fromJSON;
		JSONTransport().receive(fromJSON, Identity{}, json);
		TestNode assigned = fromJSON["assign"];
		validateNode(fromJSON);
		validateNode(fromJSON["object"], 6);
			//Test DOM i/o via XML
		String xml;
		XMLTransport().send(root, Identity{"testing"}, xml);
		CHECK_MESSAGE(xml.contains("<boolean>true</boolean>"), TEST_MESSAGE(DOM node export to XML failed with boolean value));
		CHECK_MESSAGE(xml.contains("<integer>5</integer>"), TEST_MESSAGE(DOM node export to XML failed with integer value));
		CHECK_MESSAGE(xml.contains("<double>1.23</double>"), TEST_MESSAGE(DOM node export to XML failed with double value));
		CHECK_MESSAGE(xml.contains("<string>Test</string>"), TEST_MESSAGE(DOM node export to XML failed with string value));
		CHECK_MESSAGE(xml.contains("<array><val>1</val><val>2</val><val>3</val><val>4</val><val>5</val><val>6</val></array>"),
					  TEST_MESSAGE(DOM node export to XML failed with array));
		CHECK_MESSAGE(xml.contains("<ad-hoc>1.2</ad-hoc><ad-hoc>2.3</ad-hoc><ad-hoc>text</ad-hoc><ad-hoc>1</ad-hoc><ad-hoc>false</ad-hoc>"),
					  TEST_MESSAGE(DOM node export to XML failed with ad-hoc array));
		bool isXMLOk = xml.contains("<ad-hocObj>") && xml.contains("<first>1</first>") &&
						xml.contains("<second>2</second>") && xml.contains("<third>1.23</third>") &&
						xml.contains("<fourth>testing</fourth>");
		CHECK_MESSAGE(isXMLOk, TEST_MESSAGE(DOM node export to XML failed with ad-hoc object));
		Node fromXML;
		XMLTransport().receive(fromXML, Identity{"testing"}, xml);
		assigned = fromXML["assign"];
		validateNode(fromXML);
		validateNode(fromXML["object"], 6);
	}

TEST_SUITE_CLOSE
