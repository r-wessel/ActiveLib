#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Serialise/Document/Object.h"
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
using namespace active::serialise::json;
using namespace active::serialise::xml;
using namespace active::setting;
using namespace active::utility;

namespace {

		//Test field indices
	enum Field {
		a = 0,
		b,
		c,
		d,
		e,
		f,
		foo,
	};
	
		//Test field tags
	std::array field{
		"a",
		"b",
		"c",
		"d",
		"e",
		"f",
		"foo",
	};

		//Base test class
	class Foo {
	public:
		inline static const String type = "Foo";
		
		Foo(bool filled = false) {
			if (filled) {
				m_a = true;
				m_b = 1;
				m_c = Time{false};
			}
		}
		Foo(const doc::Object& incoming, const SettingList* spec = nullptr) {
			m_a = incoming.value(field.at(a)).value_or(ValueSetting{false});
			m_b = incoming.value(field.at(b)).value_or(ValueSetting{0});
			m_c = incoming.value(field.at(c)).value_or(ValueSetting{Time{false}});
		}
		
		bool operator==(const Foo& ref) const {
			return (m_a == ref.m_a) && (m_b == ref.m_b) && (m_c.compare(ref.m_c, 0.01) == std::strong_ordering::equivalent);
		}

		doc::Object send(const SettingList* spec = nullptr) const {
			using enum Field;
			return doc::Object{Foo::type}
					<< ValueSetting{m_a, field.at(a)}
					<< ValueSetting(m_b, field.at(b))
					<< ValueSetting(m_c, field.at(c));
		}
		
	private:
		bool m_a = false;
		int32_t m_b = 0;
		Time m_c;
	};

		//Test subclass
	class Bar : public Foo {
	public:
		inline static const String type = "Bar";

		Bar(bool filled = false) : Foo(filled) {
			if (filled) {
				m_foo = Foo(true);
				m_d = 2.34;
				m_e = Guid{true};
				m_f = "3";
			}
		}
		Bar(const doc::Object& incoming, const SettingList* spec = nullptr) : Foo{incoming, spec} {
			if (auto fooIn = incoming.object<Foo>(field.at(foo)); fooIn)
				m_foo = *fooIn;
			m_d = incoming.value(field.at(d)).value_or(ValueSetting{0.0});
			m_e = incoming.value(field.at(e)).value_or(ValueSetting{Guid{}});
			m_f = incoming.value(field.at(f)).value_or(ValueSetting{String{}});
		}

		bool operator==(const Bar& ref) const {
			return Foo::operator==(ref) && (m_foo == ref.m_foo) && isEqual(m_d, ref.m_d) && (m_e == ref.m_e) && (m_f == ref.m_f);
		}

		doc::Object send(const SettingList* spec = nullptr) const {
			using enum Field;
			return Foo::send(spec).withType(Bar::type)
					<< m_foo.send(spec).withTag(field.at(foo))
					<< ValueSetting{m_d, field.at(d)}
					<< ValueSetting(m_e, field.at(e))
					<< ValueSetting(m_f, field.at(f));
		}
		
	private:
		Foo m_foo;
		double m_d = 0.0;
		Guid m_e;
		String m_f;
	};
	
		///Handler class capable of reconstructing serialised test classes
	class TestHandler : public doc::Handler {
	public:
		TestHandler() {
			add<Foo>({Foo::type});
			add<Bar>({Bar::type});
		}
	};
	
}  // namespace

	///Tests for base64 encoding
TEST_SUITE(TESTQ(documentTest)) TEST_SUITE_OPEN
	
		///Tests for sending and receiving data via base64
	TEST_CASE(TESTQ(testDocumentContent)) {
			//Make a handler class for reconstructing objects from a serialised document
		auto handler = std::make_shared<TestHandler>();
		Bar bar(true);
		auto docOut = bar.send();
		String json, xml;
			//JSON test
		JSONTransport transportJSON;
			//Send the test object as JSON (into the collection string)
		try {
			transportJSON.send(serialise::PackageWrap{docOut}, doc::Object::defaultTag, json);
			CHECK_MESSAGE(!json.empty(), TEST_MESSAGE(JSON send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON send failed));
		}
			//Receive the JSON data from the collection string into another object
		doc::Object docJSON;
		try {
			transportJSON.receive(serialise::PackageWrap{docJSON}, doc::Object::defaultTag, json);
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(JSON receive failed));
		}
			//Unpack the document into an object
		auto barJSON = docJSON.usingHandler(handler).object<Bar>();
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE((barJSON && (bar == *barJSON)), TEST_MESSAGE(Object received via JSON does not match the object sent));
			//XML test
		XMLTransport transportXML;
			//Send the test object as XML (into the collection string)
		try {
			transportXML.send(serialise::PackageWrap{docOut}, doc::Object::defaultTag, xml);
			CHECK_MESSAGE(!xml.empty(), TEST_MESSAGE(XML send produced no output));
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(XML send failed));
		}
			//Receive the XML data from the collection string into another object
		doc::Object docXML;
		try {
			transportXML.receive(serialise::PackageWrap{docXML}, doc::Object::defaultTag, xml);
		} catch(...) {
			FAIL_CHECK(TEST_MESSAGE(XML receive failed));
		}
			//Unpack the document into an object
		auto barXML = docXML.usingHandler(handler).object<Bar>();
			//Confirm that the incoming object matches the original outgoing object
		CHECK_MESSAGE((barXML && (bar == *barXML)), TEST_MESSAGE(Object received via XML does not match the object sent));
	}

TEST_SUITE_CLOSE
