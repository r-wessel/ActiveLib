/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_ANGLE_VALUE
#define ACTIVE_SETTING_ANGLE_VALUE

#include "Active/Setting/Values/Measurement/MeasuredValue.h"
#include "Active/Setting/Values/Measurement/Units/AngleUnit.h"

namespace active::setting {

	/*!
		An angle measurement value
	 
		Angles are always stored in radians with the convention that zero degrees aligns to the positive x axis and a positive angle sweeps in an
		anticlockwise direction (viewed along the negative direction of the z axis). Formatting determines how this is displayed/edited in the UI,
		e.g. surveyor conventions may orientate 0° to North (positive y axis) with positive sweep in a clockwise direction.
		Refer to the `Unit` template for more detailed information about the presentation of measured values.
	 
		Refer to `MeasuredValue` for more detailed information and examples for using AngleValue.
	*/
	class AngleValue : public MeasuredValue<active::measure::AngleUnit> {
	public:
		using MeasuredValue::operator=;
		using enum measure::AngleType;

		/*!
			Constructor
			@param val The measurement value
		*/
		AngleValue(double val = 0.0) : MeasuredValue(val) {}

		// MARK: - Operators

		/*!
			Get the measurement as a formatted string
			@param unit The output unit
			@return The measurement as a string
		*/
		utility::String operator()(const measure::AngleUnit& unit) const override;
		
		/*!
			Assign a measurement value from a string
			@param val A string value to assign
			@param unit The input unit
			@return A reference to this
		*/
		Value& assign(const utility::String& val, const measure::AngleUnit& unit) override;
	};
	
}

#endif //ACTIVE_SETTING_ANGLE_VALUE
