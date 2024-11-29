/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Measurement/Units/VolumeUnit.h"

#include "Active/Utility/MathFunctions.h"

#include <array>

using namespace active::math;

namespace active::measure {

	std::array<const char*, VolumeUnit::volumeCount> VolumeUnit::tags {
		"millilitre",
		"centilitre",
		"litre",
		"meterCubed",
		"inchCubed",
		"footCubed",
		"yardCubed",
		"gallon",
	};

	
	std::array<const char*, VolumeUnit::volumeCount> VolumeUnit::abbreviations {
		"ml3",
		"cm3",
		"l",
		"m3",
		"in3",
		"ft3",
		"yd3",
		"gal",
	};

	
	std::array<double, VolumeUnit::volumeCount> VolumeUnit::conversions {
		1.0 / metre3ToMillimetre3,
		1.0 / metre3ToCentimetre3,
		1.0 / metre3ToLitre,
		1.0,
		1.0 / metre3ToInch3,
		1.0 / metre3ToFoot3,
		1.0 / metre3ToYard3,
		1.0 / metre3ToGallon,
	};

	
	std::array<bool, VolumeUnit::volumeCount> VolumeUnit::metric {
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
