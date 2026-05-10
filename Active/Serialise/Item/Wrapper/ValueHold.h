/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_VALUE_HOLD
#define ACTIVE_SERIALISE_VALUE_HOLD

#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Item/Wrapper/ValueWrap.h"
#include "Active/Utility/Concepts.h"
#include "Active/Utility/Guid.h"
#include "Active/string/string_utf8.h"

namespace active::serialise {
	
	/*!
		Interface for a lightweight container to send temporary value data through serialisation functions
	 
		Note that this class is primarily intended for sending temporary values. Prefer `ValueWrap` for sending/receiving persistent values
		@tparam T Value native type
	*/
	template<class T>
	class ValueHold : public CargoHold<ValueWrap<T>, T> {
		using base = CargoHold<ValueWrap<T>, T>;
		using base::base;
	};
	
		///Convenience wrapper names
	using BoolHold = ValueHold<bool>;
	using DoubleHold = ValueHold<double>;
	using FloatHold = ValueHold<float>;
	using Int32Hold = ValueHold<int32_t>;
	using Int64Hold = ValueHold<int64_t>;
	using StringHold = ValueHold<string>;
	using UInt32Hold = ValueHold<uint32_t>;

}  // namespace active::serialise

#endif	//ACTIVE_SERIALISE_VALUE_HOLD
