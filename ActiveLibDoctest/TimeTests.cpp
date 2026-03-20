#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/Time.h"

using namespace active::math;
using namespace active;

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
	  current.setUTCOffset(60);
	  another.setHour(8);
	  another.setUTCOffset(-60);
	  CHECK_MESSAGE(another == current, TEST_MESSAGE(Time equality with UTC offset failed));
	  another.setUTCOffset(-120);
	  CHECK_MESSAGE(another > current, TEST_MESSAGE(Time greater-than failed));
	  another.setUTCOffset(0);
	  CHECK_MESSAGE(another < current, TEST_MESSAGE(Time less-than failed));
  }

  	///Tests for time calculations
  TEST_CASE(TESTQ(testTimeCalcs)) {
	  Time current;
	  auto changed = current;
	  changed.addMinutes(-2);
	  CHECK_MESSAGE(isEqual(current.differenceInSeconds(changed), -120), TEST_MESSAGE(Time delta failed));
	  Time lateAM{2025, std::chrono::July, 21, 12, 45};
	  lateAM.setUTCOffset(60);
	  auto earlyPM{lateAM};
	  earlyPM.addMinutes(30);
	  CHECK_MESSAGE(earlyPM > lateAM, TEST_MESSAGE(Time comparison failed));
	  CHECK_MESSAGE(isEqual(lateAM.differenceInMinutes(earlyPM), 30), TEST_MESSAGE(Time delta failed));
  }

TEST_SUITE_CLOSE
