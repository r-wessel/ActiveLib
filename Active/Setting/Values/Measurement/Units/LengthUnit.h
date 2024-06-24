/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MEASURE_LENGTH_UNIT
#define ACTIVE_MEASURE_LENGTH_UNIT

#include "Active/Setting/Values/Measurement/Units/Unit.h"

namespace active::measure {
	
	// MARK: - Types
	
		///Enumeration of known length measurement units
	enum class LengthType : char {
		millimetre,
		centimetre,
		metre,
		kilometre,
		inch,
		foot,
		yard,
		mile,
	};
	
	// MARK: - Constructors
	
	/*!
		Structure to represent length units
	*/
	struct LengthUnit : public Unit<LengthUnit, LengthType> {
		
		using Type = LengthType;
		
		using enum LengthType;

		// MARK: - Static functions
		
			//Get unit for millimetres
		static LengthUnit millimetres(uint8_t prec = 1, bool suffixes = true) { return LengthUnit{millimetre, prec, true, suffixes}; }
			//Get unit for centimetres
		static LengthUnit centimetres(uint8_t prec = 2, bool suffixes = true) { return LengthUnit{centimetre, prec, true, suffixes}; }
			//Get unit for metres
		static LengthUnit metres(uint8_t prec = 4, bool suffixes = true) { return LengthUnit{metre, prec, true, suffixes}; }
			//Get unit for decimal inches
		static LengthUnit decimalInches(uint8_t prec = 3, bool suffixes = true) { return LengthUnit{inch, prec, true, suffixes}; }
			//Get unit for decimal feet
		static LengthUnit decimalFeet(uint8_t prec = 4, bool suffixes = true) { return LengthUnit{foot, prec, true, suffixes}; }
			//Get unit for feet & decimal inches
		static LengthUnit feetDecInches(uint8_t prec = 3) { return LengthUnit{foot, inch, prec, true}; }
			//Get unit for feet & fractional inches
		static LengthUnit feetFracInches(uint8_t prec = 6) { return LengthUnit{foot, inch, prec, false}; }
		
		// MARK: - Constructors
	
		/*!
			Default constructor
		*/
		LengthUnit() : Unit{metre} {}	//Default to metres - can be overridden as required
		/*!
			Constructor
			@param type The primary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
			@param suffixes True if the value should be displayed with the unit suffix, e.g. 100mm
		*/
		LengthUnit(LengthType type, uint8_t prec, bool isDecimal = true, bool suffixes = true) :
				Unit{type, prec, isDecimal, suffixes} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
		*/
		LengthUnit(LengthType first, LengthType second, uint8_t prec, bool isDecimal = true) :
				Unit{first, second, prec, isDecimal} {}

	private:
#ifdef WINDOWS
		friend typename Unit;
#else
		friend struct Unit;
#endif
		
		// MARK: Unit constants

			///The number of supported length measurement units
		constexpr static int lengthCount = static_cast<int>(mile) + 1;

		static std::array<const char*, lengthCount> tags;
		static std::array<const char*, lengthCount> abbreviations;
		static std::array<double, lengthCount> conversions;
		static std::array<bool, lengthCount> metric;
	};
	
}
	
#endif //ACTIVE_MEASURE_LENGTH_UNIT
