/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_CLONABLE
#define ACTIVE_UTILITY_CLONABLE

#include <type_traits>
#include <memory>

namespace active::utility {
		
	/*!
		Generic base class for any object that can clone itself
	*/
	class Cloner {
	public:
		/*!
			Destructor
		*/
		virtual ~Cloner() = default;
		/*!
			Object cloning
			@return A clone of this object
		*/
		virtual Cloner* clonePtr() const = 0;
	};
	
	
		///Clonable concept for classes/functions dependent on cloning
	template<class T>
	concept Clonable = std::is_base_of<Cloner, T>::value;
	
	
	/*!
		Make a clone of a specified item
		@param item The item to clone
		@return A clone of the item
	*/
	template<class T> requires Clonable<T>
	inline auto clone(const T& item) {
		return std::unique_ptr<T>{item.clonePtr()};
	}
	
}  // namespace active::utility

#endif	//ACTIVE_UTILITY_CLONABLE
