/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SETTING_AREA_VALUE
#define ACTIVE_SETTING_AREA_VALUE

#include "Active/Setting/Values/Measurement/MeasuredValue.h"
#include "Active/Setting/Values/Measurement/Units/AreaUnit.h"

namespace active::setting {

	/*!
		An area measurement value
	 
		Areas are always stored in square metres, but formatting determines how this is displayed/edited in the UI. Refer to the `Unit` template for
		more detailed information about the presentation of measured values.
	 
		Refer to `MeasuredValue` for more detailed information and examples for using AreaValue.
	*/
	using AreaValue = MeasuredValue<active::measure::AreaUnit>;
	
}

#endif //ACTIVE_SETTING_AREA_VALUE
