/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MEASURE_MASS_UNIT
#define ACTIVE_MEASURE_MASS_UNIT

#include "Active/Setting/Values/Measurement/Units/Unit.h"

namespace active::measure {

		///Enumeration of known mass measurement units
	enum class MassType : char {
		milligram,
		gram,
		kilogram,
		tonne,
		ounce,
		pound,
		stone,
		ton,
	};

	
	/*!
		Structure to represent mass units
	*/
	struct MassUnit : public Unit<MassUnit, MassType> {
		
		using Type = MassType;
		
		using enum MassType;

		// MARK: - Static functions
		
			//Get unit for grams
		static MassUnit grams(uint8_t prec = 1, bool suffixes = true) { return MassUnit{gram, prec, true, suffixes}; }
			//Get unit for kilograms
		static MassUnit kilograms(uint8_t prec = 4, bool suffixes = true) { return MassUnit{kilogram, prec, true, suffixes}; }
			//Get unit for pounds
		static MassUnit pounds(uint8_t prec = 4, bool suffixes = true) { return MassUnit{pound, prec, true, suffixes}; }
		
		// MARK: - Constructors
	
		/*!
			Default constructor
		*/
		
		/*!
			Default constructor
		*/
		MassUnit() : Unit{kilogram} {}	//Default to kilograms - can be overridden as required
		/*!
			Constructor
			@param type The primary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
			@param suffixes True if the value should be displayed with the unit suffix, e.g. 100mm
		*/
		MassUnit(MassType type, uint8_t prec, bool isDecimal = true, bool suffixes = true) :
				Unit{type, prec, isDecimal, suffixes} {}
		/*!
			Constructor
			@param first The primary unit type
			@param second The secondary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
		*/
		MassUnit(MassType first, MassType second, uint8_t prec, bool isDecimal = true) :
				Unit{first, second, prec, isDecimal} {}

	private:
#ifdef WINDOWS
		friend typename Unit;
#else
		friend struct Unit;
#endif
		
		// MARK: Unit constants

			///The number of supported area measurement units
		constexpr static int massCount = static_cast<int>(ton) + 1;

		static std::array<const char*, massCount> tags;
		static std::array<const char*, massCount> abbreviations;
		static std::array<double, massCount> conversions;
		static std::array<bool, massCount> metric;
	};
	
}
	
#endif //ACTIVE_MEASURE_MASS_UNIT
