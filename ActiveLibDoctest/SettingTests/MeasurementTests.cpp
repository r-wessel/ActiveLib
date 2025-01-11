#include "ActiveLibDoctest/TestingPlatforms.h"

#include "Active/Setting/Values/Measurement/AngleValue.h"
#include "Active/Setting/Values/Measurement/LengthValue.h"
#include "Active/Setting/Values/Measurement/MassValue.h"
#include "Active/Utility/MathFunctions.h"

#include <variant>

using namespace active::math;
using namespace active::measure;
using namespace active::setting;
using namespace active::utility;

TEST_SUITE(TESTQ(MeasurementTests)) TEST_SUITE_OPEN

		///Tests for creating/modifying time
	TEST_CASE(TESTQ(testLengthValues)) {
		using enum LengthType;
			//Fractional feet/inches precision 1/64
		LengthValue lengthFootFracInch{{foot, inch, 6, false}, 3.0};
		CHECK_MESSAGE(lengthFootFracInch.operator String() == "9' 10 7/64\"", TEST_MESSAGE(Conversion to feet and fractional inches failed));
			//Fractional feet/inches with 0 feet (should omit feet from result)
		lengthFootFracInch = 0.0508;
		CHECK_MESSAGE(lengthFootFracInch.operator String() == "2\"", TEST_MESSAGE(Conversion to feet and fractional inches with 'zero suppression' failed));
			//Decimal feet/inches precision 1e-4
		LengthValue lengthFootDecInch{{foot, inch, 4}, 3.0};
		CHECK_MESSAGE(lengthFootDecInch.operator String() == "9' 10.1102\"", TEST_MESSAGE(Conversion to feet and decimal inches failed));
			//Default metric precision 1e-4
		LengthValue lengthMetre;
		lengthMetre = "9' 10 7/64\"";
		CHECK_MESSAGE(isEqual(lengthMetre.data, 3.0, 1e-4), TEST_MESSAGE(Conversion from feet and fractional inches to metres failed));
	}

		///Tests for time calculations
	TEST_CASE(TESTQ(testAngleValues)) {
		using enum AngleType;
		AngleValue angleValue;
		angleValue = "90°";
		CHECK_MESSAGE(isEqual(angleValue, active::math::pi / 2.0, 1e-4), TEST_MESSAGE(Conversion from degrees to radians failed));
		angleValue.setUnit(AngleUnit::degreesMinutesSeconds());
		angleValue = "57.672°";
		CHECK_MESSAGE(angleValue.operator String() == "57° 40' 19\"", TEST_MESSAGE(Conversion to degrees/minutes/seconds failed));
		angleValue.setUnit(AngleUnit::surveyorBearings());
		CHECK_MESSAGE(angleValue.operator String() == "N 32° 19' 41\" E", TEST_MESSAGE(Conversion to surveyor bearings failed));
		angleValue = "S 32° 19' 41\" E";
		CHECK_MESSAGE(isEqual(angleValue, 5.276619), TEST_MESSAGE(Conversion from surveyor bearings failed));
	}

		///Tests for time calculations
	TEST_CASE(TESTQ(testMassValues)) {
		using enum MassType;
		MassValue areaValue;
		areaValue = "100g";
		CHECK_MESSAGE(isEqual(areaValue, 0.1, 1e-4), TEST_MESSAGE(Conversion from grams to kilos failed));
		areaValue.setUnit(MassUnit::pounds());
		CHECK_MESSAGE(areaValue.operator String() == "0.2205lb", TEST_MESSAGE(Conversion to pounds failed));
		areaValue = "5";
		CHECK_MESSAGE(isEqual(areaValue, 2.267962, 1e-4), TEST_MESSAGE(Conversion from pounds to kilos failed));
	}

TEST_SUITE_CLOSE
