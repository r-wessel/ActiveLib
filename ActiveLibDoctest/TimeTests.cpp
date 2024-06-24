#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/Time.h"

using namespace active::math;
using namespace active::utility;

TEST_SUITE(TESTQ(TimeTests)) TEST_SUITE_OPEN

	  ///Tests for creating/modifying time
  TEST_CASE(TESTQ(testTimeContent)) {
	  Time current;
	  Time past{1900, std::chrono::January, 15};
	  CHECK_MESSAGE(past.year() == 1900, TEST_MESSAGE(Year assignment failed));
	  CHECK_MESSAGE(past.month() == std::chrono::January, TEST_MESSAGE(Month assignment failed));
	  CHECK_MESSAGE(past.day() == 15, TEST_MESSAGE(Day assignment failed));
	  CHECK_MESSAGE(past < current, TEST_MESSAGE(Time less-than failed));
	  Time another{current};
	  CHECK_MESSAGE(another == current, TEST_MESSAGE(Time equality failed));
	  current.setHour(10);
	  current.setUTCOffset(1);
	  another.setHour(8);
	  another.setUTCOffset(-1);
	  CHECK_MESSAGE(another == current, TEST_MESSAGE(Time equality with UTC offset failed));
	  another.setUTCOffset(-2);
	  CHECK_MESSAGE(another > current, TEST_MESSAGE(Time greater-than failed));
	  another.setUTCOffset(0);
	  CHECK_MESSAGE(another < current, TEST_MESSAGE(Time less-than failed));
  }

  	///Tests for time calculations
  TEST_CASE(TESTQ(testTimeCalcs)) {
	  Time current;
	  auto changed = current;
	  changed.addMinutes(-2);
	  CHECK_MESSAGE(isEqual(current.differenceInSeconds(changed), -120), TEST_MESSAGE(Time equality failed));
  }

TEST_SUITE_CLOSE
