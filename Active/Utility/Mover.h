/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_MOVER
#define ACTIVE_UTILITY_MOVER

#include <type_traits>
#include <memory>

namespace active::utility {
		
	/*!
		Generic base class for any object that can clone itself and move its resources to the clone
	*/
	class Mover {
	public:
		/*!
			Destructor
		*/
		virtual ~Mover() = default;
		/*!
			Object cloning with move
			@return A clone of this object with resources moved to the clone
		*/
		virtual Mover* movePtr() = 0;
	};
	
	
		///Movable concept for classes/functions dependent on cloning with a move
	template<class T>
	concept Movable = std::is_base_of<Mover, T>::value;
	
	
	/*!
		Make a clone of a specified item, moving the item resources to the clone
		@param item The item to clone
		@return A clone of the item
	*/
	template<class T> requires Movable<T>
	inline auto cloneMove(T&& item) {
		return std::unique_ptr<T>{item.movePtr()};
	}
	
}  // namespace active::utility

#endif	//ACTIVE_UTILITY_MOVER
