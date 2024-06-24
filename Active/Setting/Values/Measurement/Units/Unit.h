/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MEASURE_UNIT
#define ACTIVE_MEASURE_UNIT

#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/String.h"

#include <array>
#include <optional>

namespace active::measure {

	/*!
		Template for all classes governing the formatting of measured values when converted to a string
	 
		Formatting options include:
		- Primary unit, e.g. millimetre.
		- Secondary unit. Optional and generally only relevant to imperial units, e.g. a length might be expressed in feet and inches.
		- Tertiary unit. Optional, but used by some units, e.g. degrees/minutes/seconds for angle measurements
		- Precision:
			- For floating point formats, the maximum number of decimal places.
			- For fractions, the maximum divisor (2^-n) e.g. 1 = x/2, 2 = x/4 etc.
		- Fractions. When true, 0.5 inches will be formatted as 1/2". If a second unit is selected, only the second part will be converted to
					a fraction, e.g. 3 feet 2.5 inches will be formatted as 3' 2 1/2"
		- Suffixes. When true, measurements are displayed with the appropriate unit suffix, e.g. 100mm (vs just 100). This option is ignored when
					a second unit is picked (suffixes are always added) because it is otherwise very hard to read the value (and this code could
					not convert the text back to the original value either)
	*/
	template<typename T, typename Type>
	struct Unit {
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param type The primary unit type
		*/
		Unit(Type type) : primary{type} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
		*/
		Unit(Type first, Type second) : primary{primary}, secondary(second) {}
		/*!
			Constructor
			@param type The primary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
			@param suffixes True if the value should be displayed with the unit suffix, e.g. 100mm
		*/
		Unit(Type type, uint8_t prec, bool isDecimal = true, bool suffixes = true) :
				primary{type}, precision{prec}, base{isDecimal ? 10.0 : 2.0}, isUnitSuffix{suffixes} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
		*/
		Unit(Type first, Type second, uint8_t prec, bool isDecimal = true) :
				primary{first}, secondary{second}, precision{prec}, base{isDecimal ? 10.0 : 2.0} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
			@param third The tertiary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
		*/
		Unit(Type first, Type second, Type third, uint8_t prec, bool isDecimal = true) :
				primary{first}, secondary{second}, tertiary{third}, precision{prec}, base{isDecimal ? 10.0 : 2.0} {}
		
		// MARK: - Variables
		
			///The primary unit to display
		Type primary;
			///An optional secondary unit, e.g. for foot/inch combos (nullopt = none)
		std::optional<Type> secondary;
			///An optional teriary unit, e.g. for degrees/minutes/seconds combos (nullopt = none, ignored if no secondary unit)
		std::optional<Type> tertiary;
			///The display precision (decimal places for metric or 2^-n for fractional units
		uint8_t precision = 4;
			///The numeric base for fractional quantities (10.0 = floating point to specified decimal places, other values create a fraction)
		double base = 10.0;
			///True if values should be displayed with a unit suffix, e.g. "100.0mm" rather than "100.0" (always true when multiple units)
		bool isUnitSuffix = true;
			///True if a value is displayed with multiple units and zero values (apart from the last unit) should be included, e.g. 0' 10" vs 10"
		bool isLeadingZero = false;
		
		// MARK: - Functions (const)
		
		/*!
			Determine if this format creates fractions
			@return True if fractions are created
		*/
		bool isDecimal() const {
			return math::isEqual(base, 10.0);	//Base 10 numbers are never displayed as fractions
		}
		/*!
			The format divisor for fractional formatting
			@return The divisor
		*/
		double divisor() const {
			return pow(base, static_cast<double>(precision));
		}
		/*!
			Get the formatted rounding/display precision
			@return The rounding/display precision
		*/
		double eps() const {
			return 1.0 / divisor();
		}
		
		/*!
			Determine if the specified unit type is metric
			@param type The target unit type
			@return True if the specified unit type is metric
		*/
		bool isMetric(Type type) const {
			return T::metric.at(static_cast<size_t>(type));
		}
		/*!
			A simple unit value conversion function (multiplying the value by a conversion factor)
			@param value The value to convert
			@param factor The conversion factor
			@param reversed True if the conversion should work in reverse (from a standard unit to this unit)
			@return The converted value
		*/
		double simpleRatio(double value, double factor, bool reversed) const {
			return value * (reversed ? factor : 1.0 / factor);
		}
		/*!
			Convert a value of a specified unit type the standard unit (e.g. to metres for length)
			@param type The target unit type
			@param value The value to convert
			@param reversed True if the conversion should work in reverse (from a standard unit to the specified unit)
			@return The converted value
		*/
		double conversion(Type type, double value, bool reversed = false) const {
				//Values are simply multiplied by a fixed ratio by default (some unit types may override with more complex behaviours)
			return simpleRatio(value, T::conversions.at(static_cast<size_t>(type)), reversed);
		}
		/*!
			Get the abbreviated suffix for a unit type, e.g. "mm"
			@param type The target unit type
			@return The unit type abbreviated suffix
		*/
		utility::String suffix(Type type) const {
			return T::abbreviations.at(static_cast<size_t>(type));
		}
		/*!
			Get a unit type from a text tag
			@param text The incoming tag
			@return The equivalent unit type
		*/
		std::optional<Type> fromTag(const utility::String& text) const {
			for (auto i = 0; i < T::tags.size(); ++i)
				if (text == T::tags[i])
					return static_cast<Type>(i);
			return std::nullopt;
		}
		/*!
			Get the tag name for a unit type, e.g. "millimetres"
			@param type The incoming unit type
			@return The unit type tag
		*/
		utility::String toTag(Type type) const {
			return T::tags.at(static_cast<size_t>(type));
		}
		/*!
			Find the first unit type suffix in text, e.g. "mm" in "100mm"
			@param text The text to find the suffix in
			@param startPos The character to start searching the text from
			@return A unit type paired with the start position in the string (nullopt if no suffix is found)
		*/
		std::optional<std::pair<Type, utility::String::size_type>> findSuffix(const utility::String& text,
																			  utility::String::size_type startPos = 0) const {
			std::optional<std::pair<Type, utility::String::size_type>> result;
			for (auto i = 0; i < T::abbreviations.size(); ++i) {
				if (auto pos = text.find(T::abbreviations.at(i), startPos); pos && (!result || (*pos < result->second)))
					result = std::make_pair(static_cast<Type>(i), *pos);
			}
			return result;
		}
	};
	
}
	
#endif //ACTIVE_MEASURE_UNIT
