/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MEASURE_AREA_UNIT
#define ACTIVE_MEASURE_AREA_UNIT

#include "Active/Setting/Values/Measurement/Units/Unit.h"

namespace active::measure {

		///Enumeration of known area measurement units
	enum class AreaType : char {
		millimetreSquare,
		centimetreSquare,
		metreSquare,
		are,
		hectare,
		inchSquare,
		footSquare,
		yardSquare,
		mileSquare,
	};

	
	/*!
		Structure to represent area units
	*/
	struct AreaUnit : public Unit<AreaUnit, AreaType> {
		
		using Type = AreaType;
		
		using enum AreaType;

		// MARK: - Static functions
		
			//Get unit for square metres
		static AreaUnit metresSquare(uint8_t prec = 6, bool suffixes = true) { return AreaUnit{metreSquare, prec, true, suffixes}; }
			//Get unit for hectares
		static AreaUnit hectares(uint8_t prec = 2, bool suffixes = true) { return AreaUnit{hectare, prec, true, suffixes}; }
			//Get unit for square feet
		static AreaUnit feetSquare(uint8_t prec = 5, bool suffixes = true) { return AreaUnit{footSquare, prec, true, suffixes}; }
		
		// MARK: - Constructors
	
		/*!
			Default constructor
		*/
		AreaUnit() : Unit{metreSquare} {}	//Default to square metres - can be overridden as required
		/*!
			Constructor
			@param type The primary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
			@param suffixes True if the value should be displayed with the unit suffix, e.g. 100mm
		*/
		AreaUnit(AreaType type, uint8_t prec, bool isDecimal = true, bool suffixes = true) :
				Unit{type, prec, isDecimal, suffixes} {}

	private:
#ifdef WINDOWS
		friend typename Unit;
#else
		friend struct Unit;
#endif
		
		// MARK: Unit constants

			///The number of supported area measurement units
		constexpr static int areaCount = static_cast<int>(mileSquare) + 1;

		static std::array<const char*, areaCount> tags;
		static std::array<const char*, areaCount> abbreviations;
		static std::array<double, areaCount> conversions;
		static std::array<bool, areaCount> metric;
	};
	
}
	
#endif //ACTIVE_MEASURE_AREA_UNIT
