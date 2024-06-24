/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Measurement/Units/MassUnit.h"

#include "Active/Utility/MathFunctions.h"

#include <array>

using namespace active::math;

namespace active::measure {

	std::array<const char*, MassUnit::massCount> MassUnit::tags {
		"milligram",
		"gram",
		"kilogram",
		"tonne",
		"ounce",
		"pound",
		"stone",
		"ton",
	};

	
	std::array<const char*, MassUnit::massCount> MassUnit::abbreviations {
		"mg",
		"g",
		"kg",
		"t",
		"oz",
		"lb",
		"st",
		"ton",
	};

	
	std::array<double, MassUnit::massCount> MassUnit::conversions {
		1.0 / kilogramToMilligram,
		1.0 / kilogramToGram,
		1.0,
		1.0 / kilogramToTonne,
		1.0 / kilogramToOunce,
		1.0 / kilogramToPound,
		1.0 / kilogramToStone,
		1.0 / kilogramToTon,
	};

	
	std::array<bool, MassUnit::massCount> MassUnit::metric {
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
