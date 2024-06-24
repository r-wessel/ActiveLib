/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Measurement/Units/AreaUnit.h"

#include "Active/Utility/MathFunctions.h"

#include <array>

using namespace active::math;

namespace active::measure {

	std::array<const char*, AreaUnit::areaCount> AreaUnit::tags {
		"millimetreSquare",
		"centimetreSquare",
		"metreSquare",
		"are",
		"hectare",
		"inchSquare",
		"footSquare",
		"yardSquare",
		"mileSquare",
	};

	
	std::array<const char*, AreaUnit::areaCount> AreaUnit::abbreviations {
		"mm2",
		"cm2",
		"m2",
		"a",
		"ha",
		"in2",
		"ft2",
		"yd2",
		"mi2",
	};

	
	std::array<double, AreaUnit::areaCount> AreaUnit::conversions {
		1.0 / metre2ToMillimetre2,
		1.0 / metre2ToCentimetre2,
		1.0,
		1.0 / metre2ToAre,
		1.0 / metre2ToHectare,
		1.0 / metre2ToInch2,
		1.0 / metre2ToFoot2,
		1.0 / metre2ToYard2,
		1.0 / metreToMile,
	};

	
	std::array<bool, AreaUnit::areaCount> AreaUnit::metric {
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		false,
	};
	
}  // namespace active::measure
