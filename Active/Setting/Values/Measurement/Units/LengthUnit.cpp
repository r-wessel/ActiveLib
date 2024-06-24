/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Measurement/Units/LengthUnit.h"

#include "Active/Utility/MathFunctions.h"

#include <array>

using namespace active::math;

namespace active::measure {
	
	std::array<const char*, LengthUnit::lengthCount> LengthUnit::tags {
		"millimetre",
		"centimetre",
		"metre",
		"kilometre",
		"inch",
		"foot",
		"yard",
		"mile",
	};

	
	std::array<const char*, LengthUnit::lengthCount> LengthUnit::abbreviations {
		"mm",
		"cm",
		"m",
		"km",
		"\"",
		"'",
		"yd",
		"mi",
	};

	
	std::array<double, LengthUnit::lengthCount> LengthUnit::conversions {
		1.0 / metreToMillimetre,
		1.0 / metreToCentimetre,
		1.0,
		1.0 / metreToKilometre,
		1.0 / metreToInch,
		1.0 / metreToFoot,
		1.0 / metreToYard,
		1.0 / metreToMile,
	};

	
	std::array<bool, LengthUnit::lengthCount> LengthUnit::metric {
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
