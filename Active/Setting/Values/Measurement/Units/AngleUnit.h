/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MEASURE_ANGLE_UNIT
#define ACTIVE_MEASURE_ANGLE_UNIT

#include "Active/Setting/Values/Measurement/Units/Unit.h"

namespace active::measure {
	
	// MARK: - Types
	
		///Enumeration of known angle measurement units
	enum class AngleType : char {
		radian,
		degree,
		minute,
		second,
		gradian,
	};
	
	// MARK: - Constructors
	
	/*!
		Structure to represent angle units
	*/
	struct AngleUnit : public Unit<AngleUnit, AngleType> {

		// MARK: - Types
	
		using Type = AngleType;
		
		using enum AngleType;

		// MARK: - Static functions
		
			//Get unit for radians
		static AngleUnit radians(uint8_t prec = 4, bool suffixes = true) { return AngleUnit{radian, prec, true, suffixes}; }
			//Get unit for decimal degrees
		static AngleUnit degreesDec(uint8_t prec = 4, bool suffixes = true) { return AngleUnit{degree, prec, true, suffixes}; }
			//Get unit for whole degrees
		static AngleUnit degrees(bool suffixes = true) { return AngleUnit{degree, 0, true, suffixes}; }
			//Get unit for whole degrees & minutes
		static AngleUnit degreesMinutes() { return AngleUnit{degree, minute, 0}; }
			//Get unit for whole degrees, minutes & seconds
		static AngleUnit degreesMinutesSeconds() { return AngleUnit{degree, minute, second, 0}; }
			//Get unit for surveyor bearings
		static AngleUnit surveyorBearings() {
			AngleUnit result{AngleUnit{degree, minute, second, 0, true}};
			result.isSurveyBearing = true;
			return result;
		}

		// MARK: - Constructors
	
		/*!
			Default constructor
		*/
		AngleUnit() : Unit{radian} {}	//Default to metres - can be overridden as required
		/*!
			Constructor
			@param type The primary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
			@param suffixes True if the value should be displayed with the unit suffix, e.g. 100mm
		*/
		explicit AngleUnit(AngleType type, uint8_t prec, bool isDecimal = true, bool suffixes = true) :
				Unit{type, prec, isDecimal, suffixes} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
		*/
		explicit AngleUnit(AngleType first, AngleType second, uint8_t prec, bool isDecimal = true) :
				Unit{first, second, prec, isDecimal} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
			@param third The tertiary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
		*/
		explicit AngleUnit(AngleType first, AngleType second, AngleType third, uint8_t prec, bool isDecimal = true) :
				Unit{first, second, third, prec, isDecimal} {}

		// MARK: - Variables
	
			///True if convention for survey bearings is used, e.g. N 25° W
		bool isSurveyBearing = false;
			///True if s clockwise sweep angle is positive. Ignored when isSurveyBearing = true
		bool isClockwisePositive = false;
			///Offset to 0° (from the positive x axis)) e.g. zeroOffset = pi / 2 if 0° points North. Ignored when isSurveyBearing = true
		double zeroOffset = 0.0;

		// MARK: - Functions (const)
		
		/*!
			Determine if this angle unit uses the math convention for angles (0° aligns to positive x axis, positive sweep is anti-clockwise)
			@return True if a conventional angle is used
		*/
		bool isConventionalAngle() const { return !isSurveyBearing && !isClockwisePositive && math::isZero(zeroOffset); }
	

	private:
#ifdef WINDOWS
		friend typename Unit;
#else
		friend struct Unit;
#endif
		
		// MARK: Unit constants

			///The number of supported length measurement units
		constexpr static int lengthCount = static_cast<int>(gradian) + 1;

		static std::array<const char*, lengthCount> tags;
		static std::array<const char*, lengthCount> abbreviations;
		static std::array<double, lengthCount> conversions;
		static std::array<bool, lengthCount> metric;
	};
	
}
	
#endif //ACTIVE_MEASURE_ANGLE_UNIT
