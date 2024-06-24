#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Attribute/Colour.h"
#include "Active/Utility/Time.h"

using namespace active::attribute;

TEST_SUITE(TESTQ(AttributeTests)) TEST_SUITE_OPEN

	  ///Tests for creating/modifying time
  TEST_CASE(TESTQ(testColour)) {
	Colour colour{ 128, 128, 128 };
	CHECK_MESSAGE(!colour.isTransparent(), TEST_MESSAGE(Colour transparency wrong));
	auto hex = colour.hex();
	CHECK_MESSAGE(hex == "808080", TEST_MESSAGE(Colour to hex failed));
	colour = {200, 200, 200, 0};
	CHECK_MESSAGE(colour.isTransparent(), TEST_MESSAGE(Colour transparency wrong));
	hex = colour.hex(true);
	CHECK_MESSAGE(hex == "C8C8C800", TEST_MESSAGE(Colour to hex with null alphafailed));
	colour.a = 0.785;
	hex = colour.hex(true);
	CHECK_MESSAGE(hex == "C8C8C8C8", TEST_MESSAGE(Colour to hex with alpha failed));
}

TEST_SUITE_CLOSE
