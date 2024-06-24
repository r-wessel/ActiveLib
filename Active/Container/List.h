/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_CONTAINER_LIST
#define ACTIVE_CONTAINER_LIST

#include "Active/Utility/Cloner.h"
#include "Active/Utility/Mover.h"

#include <list>
#include <memory>

namespace active::container {
	
	/*!
		A specialisation class for a list of objects wrapped in unique_ptr
	 
		This allows list to store objects with polymorphic behaviour and object copying (object type must conform to Clonable).
		Keep in mind that the normal behaviour of the list is to take ownership of any objects stored in it (unless release is used)
	 
		NB: There is no requirement for container items to be non-null. It is entirely up the implementor if this rule should be adopted
		or enforced
	*/
	template<class T> requires utility::Clonable<T>
	class List : public std::list<std::unique_ptr<T>> {
	public:
		
		// MARK: Types
		
			///Stored type
		using value_t = std::unique_ptr<T>;
			///Base container type
		using base = std::list<value_t>;
			///Container size (index) type
		using size_type = typename base::size_type;
			///Container iterator type
		using iterator = typename base::iterator;
			///Container const iterator type
		using const_iterator = typename base::const_iterator;
		
		// MARK: Constructors
		
		/*!
			Constructor
		*/
		List() : base() {}
		/*!
			Constructor
			@param items The items to insert into the list
		*/
		template<class Derived>
		explicit List(const std::initializer_list<Derived>& items) : base() {
			for (const auto& item : items)
				emplace_back(item);
		}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		List(const List& source) : base () { cloneFrom(source); }
		/*!
			Move constructor
			@param source The object to move
		*/
		List(List&& source) : base(std::move(source)) {}
		/*!
			Destructor
		*/
		virtual ~List() = default;
		
		// MARK: Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		auto operator= (const List& source) {
			if (this != &source) {
				base::clear();
				cloneFrom(source);
			}
			return *this;
		}
		/*!
			Assignment operator
			@param source The object to move
			@return A reference to this
		*/
		auto operator= (List&& source) {
			if (this != &source)
				base::operator=(std::move(source));
			return *this;
		}
		
		// MARK: Functions (mutating)
		
		/*!
			Push an item at the end of the list
			@param item The item to push (ownership passes to this list)
		*/
		void push_back(T* item) { base::push_back(value_t(item)); }
		/*!
			Push an item at the end of the list
			@param item The item to push (ownership passes to this list)
		*/
		void push_back(T& item) { base::push_back(clone(item)); }
		/*!
			Push an item at the end of the list
			@param item The item to push
		*/
		void emplace_back(T&& item) {
			if constexpr(utility::Movable<T>)
				base::emplace_back(cloneMove(std::move(item)));
			else
				base::emplace_back(clone(item));
		}
		/*!
			Push an item at the end of the list
			@param item The item to push
		*/
		void emplace_back(const T& item) { base::emplace_back(clone(item)); }
		/*!
			Push an item at the end of the list
			@param item The item to push (already wrapped in unique_ptr)
		*/
		void emplace_back(value_t&& item) { base::emplace_back(std::move(item)); }
		/*!
			Push an item at the end of the list
			@param item The item to push (already wrapped in unique_ptr)
		*/
		void emplace_back(value_t& item) { base::emplace_back(std::move(item)); }
		/*!
			Push an item at the end of the list
			@param item The item to push (ownership passes to this list)
		*/
		void push_front(T* item) { base::push_front(value_t(item)); }
		/*!
			Push an item at the front of the list
			@param item The item to push
		*/
		void emplace_front(T&& item) {
			if constexpr(utility::Movable<T>)
				base::emplace_front(cloneMove(std::move(item)));
			else
				base::emplace_front(clone(item));
		}
		/*!
			Push an item at the front of the list
			@param item The item to push (already wrapped in unique_ptr)
		*/
		void emplace_front(value_t&& item) { base::emplace_front(std::move(item)); }
		/*!
			Release the pointer held by the specified item and erase
			@param pos An iterator pointing to the item to be removed
			@return An iterator at the next value
		*/
		auto release(iterator& pos) {
			auto item = std::move(*pos);
			pos = this->erase(pos);
			return item;
		}
		
	private:
		/*!
			Clone the contents of another list into this
			@param source The object to copy
		*/
		void cloneFrom(const List& source) {
			for (const auto& item : source)
				base::push_back(item ? clone(*item) : value_t());
		}
	};
	
}

#endif	//ACTIVE_CONTAINER_LIST
