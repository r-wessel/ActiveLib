/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_LENGTH_VALUE
#define ACTIVE_SETTING_LENGTH_VALUE

#include "Active/Setting/Values/Measurement/MeasuredValue.h"
#include "Active/Setting/Values/Measurement/Units/LengthUnit.h"

namespace active::setting {

	/*!
		A length measurement value
	 
		Lengths are always stored in metres, but formatting determines how this is displayed/edited in the UI. Refer to the `Unit` template for
		more detailed information about the presentation of measured values.
	 
		Refer to `MeasuredValue` for more detailed information and examples for using LengthValue.
	*/
	using LengthValue = MeasuredValue<active::measure::LengthUnit>;
	
}

#endif //ACTIVE_SETTING_LENGTH_VALUE
