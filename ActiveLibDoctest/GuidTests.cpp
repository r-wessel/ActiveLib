#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Utility/Guid.h"

using namespace active::utility;

TEST_SUITE(TESTQ(GuidTests)) TEST_SUITE_OPEN

	  ///Tests for creating/modifying guids
  TEST_CASE(TESTQ(testGuidContent)) {
	  String guidString{"852CE416-03B2-44DA-837B-EA920C611B6D"};
	  
	  Guid badGuid{String{"abcd"}};
	  CHECK_MESSAGE(!badGuid, TEST_MESSAGE(Guid created from bad data));
	  String test = badGuid;
	  CHECK_MESSAGE(test == "00000000-0000-0000-0000-000000000000", TEST_MESSAGE(Null guid to string failed));
	  Guid goodGuid{guidString};
	  CHECK_MESSAGE(goodGuid, TEST_MESSAGE(Guid from string failed));
	  CHECK_MESSAGE(goodGuid.operator String().lowercase() == guidString.lowercase(), TEST_MESSAGE(Guid to string failed));
	  Guid another = goodGuid;
	  CHECK_MESSAGE(another == goodGuid, TEST_MESSAGE(Guid assignment/comparison failed));
	  CHECK_MESSAGE(another.operator String().lowercase() == guidString.lowercase(), TEST_MESSAGE(Guid to string failed));
	  int64_t val = -2317212765;
	  auto valGuid{Guid::fromInt(val)};
	  CHECK_MESSAGE(valGuid, TEST_MESSAGE(Guid from int64_t failed));
	  auto guidVal = Guid::toInt(valGuid);
	  CHECK_MESSAGE(val == guidVal, TEST_MESSAGE(int64_t from Guid failed));
  }

TEST_SUITE_CLOSE
