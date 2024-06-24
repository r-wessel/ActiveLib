#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"
#include "Active/Utility/Time.h"

#include <map>

using namespace active;
using namespace active::math;
using namespace active::utility;

namespace {
	auto sampleText = u8"ਖdਖdਖdਖdabਖdefghabਖdefgh";
	
	std::map<char32_t, char32_t> emojiFilter = {
		{ 0x00A9, 0x00A9 },
		{ 0x00AE, 0x00AE },
		{ 0x203C, 0x203C },
		{ 0x2049, 0x2049 },
		{ 0x20E3, 0x20E3 },
		{ 0x2122, 0x2122 },
		{ 0x2139, 0x2139 },
		{ 0x2194, 0x2199 },
		{ 0x21A9, 0x21AA },
		{ 0x231A, 0x231A },
		{ 0x231B, 0x231B },
		{ 0x2328, 0x2328 },
		{ 0x23CF, 0x23CF },
		{ 0x23E9, 0x23F3 },
		{ 0x23F8, 0x23FA },
		{ 0x24C2, 0x24C2 },
		{ 0x25AA, 0x25AA },
		{ 0x25AB, 0x25AB },
		{ 0x25B6, 0x25B6 },
		{ 0x25C0, 0x25C0 },
		{ 0x25FB, 0x25FE },
		{ 0x2600, 0x27EF },
		{ 0x2934, 0x2934 },
		{ 0x2935, 0x2935 },
		{ 0x2B00, 0x2BFF },
		{ 0x3030, 0x3030 },
		{ 0x303D, 0x303D },
		{ 0x3297, 0x3297 },
		{ 0x3299, 0x3299 },
		{ 0x1F000, 0x1F02F },
		{ 0x1F0A0, 0x1F0FF },
		{ 0x1F100, 0x1F64F },
		{ 0x1F680, 0x1F6FF },
		{ 0x1F910, 0x1F9FF },
	};
	
		///Return true if a specified unicode char is an emoji
	bool isEmoji(char32_t uniChar) {
		auto match = emojiFilter.upper_bound(uniChar);
		if (match == emojiFilter.end())
			match = prev(match);
		else if (match != emojiFilter.begin())
			match--;
		return ((match->first <= uniChar) && (match->second >= uniChar));
	}
	
}  // namespace

TEST_SUITE(TESTQ(StringTests)) TEST_SUITE_OPEN
	
		//Test the integrity of string content for general functions
	TEST_CASE(TESTQ(testStringContent)) {
		
			//Positive tests - these are well-formed statements that have a valid impact on the test string
		
			//Initialisation
		String test{sampleText};
		CHECK_MESSAGE(test == sampleText, TEST_MESSAGE(String constructed with incorrect content));
		CHECK_MESSAGE(test.length() == 24, TEST_MESSAGE(String constructed with incorrect size));
			//Substring
		auto sub = test.substr(3, 10);
		CHECK_MESSAGE(sub == u8"dਖdਖdabਖde", TEST_MESSAGE(String substring content wrong));
		CHECK_MESSAGE(sub.length() == 10, TEST_MESSAGE(String substring size wrong));
		sub.replace(5, 2, "cd");
		CHECK_MESSAGE(sub.length() == 10, TEST_MESSAGE(Replace text size incorrect));
		CHECK_MESSAGE(sub == u8"dਖdਖdcdਖde", TEST_MESSAGE(Replace text content incorrect));
			//Erase
		sub.erase(5, 2);
		CHECK_MESSAGE(sub.length() == 8, TEST_MESSAGE(Erase text size incorrect));
		CHECK_MESSAGE(sub == u8"dਖdਖdਖde", TEST_MESSAGE(Erase text content incorrect));
			//Replace
		test.replace(2, 0, U"x𞢈z");
		CHECK_MESSAGE(test.length() == 27, TEST_MESSAGE(Replace with wide char text size incorrect));
		CHECK_MESSAGE(test[3] == U'𞢈', TEST_MESSAGE(Replace with wide char text content incorrect));
		std::u32string uniString = test;
		CHECK_MESSAGE(uniString.size() == 27, TEST_MESSAGE(Unicode string constructed with incorrect size));
		CHECK_MESSAGE(uniString[3] == U'\U0001E888', TEST_MESSAGE(Unicode string constructed with incorrect content));
		std::u16string uniString16 = test;
		String test16{uniString16};
		CHECK_MESSAGE(test16.size() == 27, TEST_MESSAGE(Unicode string constructed with incorrect size));
		CHECK_MESSAGE(test16[3] == U'𞢈', TEST_MESSAGE(Unicode string constructed with incorrect content));
			//Copy to buffer
		Memory charBuffer;
		test.writeTo(BufferOut{charBuffer});
		CHECK_MESSAGE(test == String{charBuffer.data()}, TEST_MESSAGE(String.copyTo(char) incorrect content));
		charBuffer.clear();
		test.writeTo(BufferOut{charBuffer}, String::UTF32);
		CHECK_MESSAGE(test == String(charBuffer.data(), std::nullopt, String::UTF32), TEST_MESSAGE(String.copyTo(char32_t) incorrect content));
		auto upper = test.uppercase();
		auto lower = upper.lowercase();
		CHECK_MESSAGE(test == lower, TEST_MESSAGE(String case transformations incorrect content));
		
			//Negative tests - these are invalid operations on the target string, attempting to provoke errors
		
			//Bad length bounds
		test = String{sampleText, 100};
		CHECK_MESSAGE(test.length() == 24, TEST_MESSAGE(String length wrong constructed from bad bounds));
			//Bad start/length request
		sub = test.substr(100, 10);
		CHECK_MESSAGE(sub.empty(), TEST_MESSAGE(String contents not empty after substr out of bounds));
			//Bad replace position
		test = sampleText;
		CHECK_MESSAGE(test.length() == 24, TEST_MESSAGE(String length wrong assigned from bad bounds));
		test.replace(100, 10, "hi");	//Appends to the end of the string when start position is after the end
		CHECK_MESSAGE(test.length() == 26, TEST_MESSAGE(String replace failed with bad bounds));
			//Void replacement input (effectively an erase)
		test.replace(0, 100, "");
		CHECK_MESSAGE(test.empty(), TEST_MESSAGE(String replace of all contents failed to erase string));
			//Erase position out of bounds
		test = String(sampleText);
		test.erase(100, 10);
		CHECK_MESSAGE(test.length() == 24, TEST_MESSAGE(String erase out of bounds changed string length));
			//Test find and erase in multibyte chars
		String exampleStr{u8"½Pint Solutions® Inc Copyright © 2024"};
		auto pos = exampleStr.findFirstOf(u8"®©");
		exampleStr.erase(*pos, 1);
			//Test interoperability with std::string types
		String example1{u8"ようこそ 日本 へ"};
		String example2{u"ようこそ 日本 へ"};
		String example3{U"ようこそ 日本 へ"};
		std::u8string examplestd1{u8"ようこそ 日本 へ"};
		std::u16string examplestd2{u"ようこそ 日本 へ"};
		std::u32string examplestd3{U"ようこそ 日本 へ"};
		CHECK_MESSAGE(((example1 == examplestd1) && (example2 == examplestd2) && (example3 == examplestd3)), TEST_MESSAGE(String init failed));
		String copy1 = examplestd1;
		String copy2 = examplestd2;
		String copy3 = examplestd3;
		std::u8string copystd1 = copy1;
		std::u16string copystd2 = copy2;
		std::u32string copystd3 = copy3;
		CHECK_MESSAGE(((example1 == copystd1) && (example2 == copystd2) && (example3 == copystd3)), TEST_MESSAGE(String copy failed));

	} // namespace StringTest

	TEST_CASE(TESTQ(testStringFind)) {
		
			//Positive tests - these are well-formed statements that have a valid impact on the test string
		
		String test{sampleText};
		auto result = test.find(u8"bਖd");
		CHECK_MESSAGE(result == 9, TEST_MESSAGE(String find returned wrong position));
		result = test.rfind(u8"bਖd");
		CHECK_MESSAGE(result == 17, TEST_MESSAGE(String find returned wrong position));
		result = test.findFirstNotOf(u8"ਖd");
		CHECK_MESSAGE(result == 8, TEST_MESSAGE(String findFirstNotOf returned wrong position));
		result = test.findFirstOf("ab");
		CHECK_MESSAGE(result == 8, TEST_MESSAGE(String findFirstOf returned wrong position));
		result = test.findLastOf("ab");
		CHECK_MESSAGE(result == 17, TEST_MESSAGE(String findLastOf returned wrong position));
		result = test.findLastNotOf(u8"abਖefgh");
		CHECK_MESSAGE(result == 19, TEST_MESSAGE(String findLastNotOf returned wrong position));
		result = test.findLastNotOf(u8"aਖdefgh");
		CHECK_MESSAGE(result == 17, TEST_MESSAGE(String findLastNotOf returned wrong position));
		result = test.findLastNotOf(u8"abਖdefgh");
		CHECK_MESSAGE(!result, TEST_MESSAGE(String findLastNotOf returned wrong position));
	}

		///Tests for string conversions
	TEST_CASE(TESTQ(testStringConversion)) {
			//Double precision floating point tests
		String testDoubleStr{ "654321.12345678" };
		double testDouble{testDoubleStr};
		CHECK_MESSAGE(isEqual(testDouble, 654321.12345678, 1e-7), TEST_MESSAGE(String conversion to double failed));
		String output5{testDouble, 1e-5};
		CHECK_MESSAGE(output5 == "654321.12346", TEST_MESSAGE(Double conversion to string with 5 dp failed));
		String output3{testDouble, 1e-3};
		CHECK_MESSAGE(output3 == "654321.123", TEST_MESSAGE(Double conversion to string with 3 dp failed));
		String output6{1.234, 1e-6};
		CHECK_MESSAGE(output6 == "1.234", TEST_MESSAGE(Double conversion to string with 6 dp & no padding failed));
		String output6b{1.234, 1e-6, true};
		CHECK_MESSAGE(output6b == "1.234000", TEST_MESSAGE(Double conversion to string with 6 dp and padding failed));
			//Signed 16-bit integer tests
		String test16{"-7654"};
		int16_t val16{test16};
		CHECK_MESSAGE(val16 == -7654, TEST_MESSAGE(String conversion to int16_t failed));
		String output16{val16};
		CHECK_MESSAGE(output16 == test16, TEST_MESSAGE(int16_t conversion to String failed));
			//Signed 32-bit integer tests
		String test32{"-1073741824"};
		int32_t val32{test32};
		CHECK_MESSAGE(val32 == -1073741824, TEST_MESSAGE(String conversion to int32_t failed));
		String output32{val32};
		CHECK_MESSAGE(output32 == test32, TEST_MESSAGE(int32_t conversion to String failed));
			//Unsigned 32-bit integer tests
		String testu32{"2147483648"};
		uint32_t valu32{testu32};
		CHECK_MESSAGE(valu32 == 2147483648, TEST_MESSAGE(String conversion to uint32_t failed));
		String outputu32{valu32};
		CHECK_MESSAGE(outputu32 == testu32, TEST_MESSAGE(Uint32_t conversion to String failed));
			//Signed 64-bit integer tests
		String test64{"-9007199254740992"};
		int64_t val64{test64};
		CHECK_MESSAGE(val64 == -9007199254740992, TEST_MESSAGE(String conversion to int64_t failed));
		String output64{val64};
		CHECK_MESSAGE(output64 == test64, TEST_MESSAGE(int64_t conversion to String failed));
			//Unsigned 64-bit integer tests
		String testu64{"9223372036854775808"};
		uint64_t valu64{testu64};
		CHECK_MESSAGE(valu64 == 9223372036854775808u, TEST_MESSAGE(String conversion to uint64_t failed));
		String outputu64{valu64};
		CHECK_MESSAGE(outputu64 == testu64, TEST_MESSAGE(Uint64_t conversion to String failed));
	}
	
		///Tests for string editing
	TEST_CASE(TESTQ(testStringEditing)) {
			//Test for replacement of expression in a string containing partial and full matches
		String sentence{"Once upon a time, it was the best of times"};
		sentence.replaceAll("times", "algorithms");
		CHECK_MESSAGE(sentence == "Once upon a time, it was the best of algorithms", TEST_MESSAGE(String replaceAll failed));
			//Strip out specified characters
		sentence = "Some 😀 text 😅 with 🥸 annoying🤔 emojis🤕";
			///Search for char in specified array
		auto firstEmoji = sentence.findFirstOf("👹🥸😀🤔🤢🤕😅🦷");
		CHECK_MESSAGE(firstEmoji == 5, TEST_MESSAGE(String::findFirstOf failed to find char));
		CHECK_MESSAGE(sentence[*firstEmoji] == U'😀', TEST_MESSAGE(String subscript failed to get correct char));
			///Search for char by filter
		firstEmoji = sentence.findIf([&sentence](auto uniChar){ return isEmoji(uniChar); });
		CHECK_MESSAGE(firstEmoji == 5, TEST_MESSAGE(String::findIf failed to find char));
			//Strip out all emoji
		sentence.replaceIf([&sentence](auto uniChar){ return isEmoji(uniChar); }, "");
			//Eliminate double spacing
		for (;;)
			if (auto len = sentence.size(); len == sentence.replaceAll("  ", " ").size())
				break;
		sentence.replaceAll("with", "without");
		CHECK_MESSAGE(sentence == "Some text without annoying emojis", TEST_MESSAGE(String.replaceAnyOf failed));
	}

		///Throw random data at String to try to provoke a crash
	TEST_CASE(TESTQ(testStringFuzzing)) {
		constexpr Memory::size_type testLen = 0x0100;
		constexpr Memory::size_type testCount = 10;
		auto total = 0;
		srand(Time{}.microsecond());
		for (auto n = 0; n < testCount; ++n) {
			Memory random;
			{
				BufferOut temp{random};
				for (auto i = 0; i < testLen; ++i, ++total)
					temp.write(rand() % 256);
			}
			String test{BufferIn{random}};	//We can't confirm correctness - this is to test the String class doesn't crash when fed random (bad) data
		}
	}

		///Tests for string comparisons
	TEST_CASE(TESTQ(testStringCompare)) {
		CHECK_MESSAGE(String{"A"} == String{"A"}, TEST_MESSAGE(String equality check failed));
		CHECK_MESSAGE(String{"A"} != String{"B"}, TEST_MESSAGE(String inequality check failed));
		CHECK_MESSAGE(String{"A"} < String{"B"}, TEST_MESSAGE(String less-than check failed));
		CHECK_MESSAGE(String{"B"} > String{"A"}, TEST_MESSAGE(String greater-than check failed));
	}

TEST_SUITE_CLOSE
