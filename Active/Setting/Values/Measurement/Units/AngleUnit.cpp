/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Measurement/Units/AngleUnit.h"

#include "Active/Utility/MathFunctions.h"

#include <array>

using namespace active::math;

namespace active::measure {
	
	std::array<const char*, AngleUnit::lengthCount> AngleUnit::tags {
		"radian",
		"degree",
		"minute",
		"second",
		"gradian",
	};

	
	std::array<const char*, AngleUnit::lengthCount> AngleUnit::abbreviations {
		"ra",
		"°",
		"'",
		"\"",
		"gr",
	};

	
	std::array<double, AngleUnit::lengthCount> AngleUnit::conversions {
		1.0,
		degreeToRadian,
		minuteToRadian,
		secondToRadian,
		gradToRadian,
	};


	std::array<bool, AngleUnit::lengthCount> AngleUnit::metric {
		true,
		false,
		false,
		false,
		false,
	};
	
}  // namespace active::measure
