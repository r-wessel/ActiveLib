/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_MEASURE_VOLUME_UNIT
#define ACTIVE_MEASURE_VOLUME_UNIT

#include "Active/Setting/Values/Measurement/Units/Unit.h"

namespace active::measure {

		///Enumeration of known volume measurement units
	enum class VolumeType : char {
		millilitre,
		centilitre,
		litre,
		meterCubed,
		inchCubed,
		footCubed,
		yardCubed,
		gallon,
	};

	
	/*!
		Structure to represent volume units
	*/
	struct VolumeUnit : public Unit<VolumeUnit, VolumeType> {
		
		using Type = VolumeType;
		
		using enum VolumeType;

		// MARK: - Static functions
		
			//Get unit for cubic metres
		static VolumeUnit metresCubed(uint8_t prec = 6, bool suffixes = true) { return VolumeUnit{meterCubed, prec, true, suffixes}; }
			//Get unit for cubic feet
		static VolumeUnit feetCubed(uint8_t prec = 5, bool suffixes = true) { return VolumeUnit{footCubed, prec, true, suffixes}; }
			//Get unit for gallons
		static VolumeUnit gallons(uint8_t prec = 2, bool suffixes = true) { return VolumeUnit{gallon, prec, true, suffixes}; }
		
		// MARK: - Constructors
	
		/*!
			Default constructor
		*/
		VolumeUnit() : Unit{meterCubed} {}	//Default to cubic metres - can be overridden as required
		/*!
			Constructor
			@param type The primary unit type
			@param prec The unit display precision (decimal places for metric or 2^-n for fractional units)
			@param isDecimal True if fractional values should display as decimal (floating point) numbers
			@param suffixes True if the value should be displayed with the unit suffix, e.g. 100mm
		*/
		VolumeUnit(VolumeType type, uint8_t prec, bool isDecimal = true, bool suffixes = true) :
				Unit{type, prec, isDecimal, suffixes} {}

	private:
#ifdef WINDOWS
		friend typename Unit;
#else
		friend struct Unit;
#endif
		
		// MARK: Unit constants

			///The number of supported volume measurement units
		constexpr static int volumeCount = static_cast<int>(gallon) + 1;

		static std::array<const char*, volumeCount> tags;
		static std::array<const char*, volumeCount> abbreviations;
		static std::array<double, volumeCount> conversions;
		static std::array<bool, volumeCount> metric;
	};
	
}
	
#endif //ACTIVE_MEASURE_VOLUME_UNIT
