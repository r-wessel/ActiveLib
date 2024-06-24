/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Setting/Values/Measurement/AngleValue.h"

#include "Active/Utility/MathFunctions.h"

#include <array>
#include <set>

using namespace active::math;
using namespace active::measure;
using namespace active::setting;
using namespace active::utility;

namespace {
	
	enum class CompassDirection {
		east,
		north,
		west,
		south,
	};
	
	std::array compassAbbreviation = {
		"E",
		"N",
		"W",
		"S",
	};
	
}  // namespace

using enum CompassDirection;

/*--------------------------------------------------------------------
	Get the measurement as a formatted string
 
	unit: The output unit
 
	return: The measurement as a string
  --------------------------------------------------------------------*/
String AngleValue::operator()(const AngleUnit& unit) const {
	if (!unit.isConventionalAngle()) {
		if (unit.isSurveyBearing) {
			auto angle = data;
				//Establish East/West suffix
			String suffix{(compassAbbreviation.at(static_cast<int>(isGreaterOrEqual(angle, 1.5 * pi) || isLess(angle, pi / 2.0) ? east : west)))},
					prefix;
				//Establish North/South prefix
			if (isGreaterOrEqualZero(angle) && isLess(angle, pi)) {
				prefix = compassAbbreviation.at(static_cast<int>(north));
				angle = std::fabs((pi / 2.0) - angle);
			} else {
				prefix = compassAbbreviation.at(static_cast<int>(south));
				angle = std::fabs((1.5 * pi) - angle);
			}
			return prefix + " " + AngleValue(angle)(AngleUnit::degreesMinutesSeconds()) + " " + suffix;
		}
		return AngleValue((data - unit.zeroOffset) * (unit.isClockwisePositive ? -1.0 : 1.0))(unit);
	}
		//Ensure the angle is within +/-(2 * pi)
	if (!isBetween(data, -2.0 * pi, 2.0 * pi))
		return AngleValue(fmod(data, 2.0 * pi))(unit);
	return MeasuredValue::operator()(unit);
} //AngleValue::operator()


/*--------------------------------------------------------------------
	Assign a measurement value from a string
 
	val: A string value to assign
	unit: The input unit
 
	return: A reference to this
  --------------------------------------------------------------------*/
Value& AngleValue::assign(const String& val, const AngleUnit& unit) {
	data = 0.0;
	status = bad;
		//Start with the unit conventions (can be modified by user input)
	auto sweepDirection = unit.isClockwisePositive ? -1.0 : 1.0,
			zeroDelta = unit.zeroOffset;
	String angleText{val.uppercase()};
		//Angles may contain surveyor bearings - if so, we need to process and remove these first
	auto index = 0;
	String directions;
	std::set<CompassDirection> ordinals;
	for (const auto& abbrev : compassAbbreviation) {
		if (angleText.contains(abbrev)) {
			ordinals.insert(static_cast<CompassDirection>(index));
			angleText.replaceAll(abbrev, String{});
		}
		++index;
	}
	if (!ordinals.empty()) {
		if ((ordinals.size() != 2) ||
				(ordinals.contains(north) && (ordinals.contains(south))) ||
				(ordinals.contains(east) && (ordinals.contains(west))))
			return *this;
			//Update the zero angle offset and sweep direction based on the ordinals
		sweepDirection = ordinals.contains(west) ? 1.0 : -1.0;
		if (ordinals.contains(north))
			zeroDelta = pi / 2.0;
		else {
			zeroDelta = 1.5 * pi;
			sweepDirection *= -1.0;
		}
	}
	if (!MeasuredValue::assign(angleText, unit).isGood())
		return *this;
	data = zeroDelta + (data * sweepDirection);
	return *this;
} //AngleValue::assign
