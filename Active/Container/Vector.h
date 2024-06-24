/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_CONTAINER_VECTOR
#define ACTIVE_CONTAINER_VECTOR

#include "Active/Utility/Cloner.h"
#include "Active/Utility/Mover.h"

#include <vector>

namespace active::container {
	
	/*!
		A specialisation class for a vector of objects wrapped in unique_ptr
	 
		This allows vector to store objects with polymorphic behaviour and object copying (object type must conform to Clonable).
		Keep in mind that the normal behaviour of the vector is to take ownership of any objects stored in it (unless release is used)
	 
		NB: There is no requirement for container items to be non-null. It is entirely up the implementor if this rule should be adopted
		or enforced
	*/
	template<class T> requires utility::Clonable<T>
	class Vector : public std::vector<std::unique_ptr<T>> {
	public:
		
		// MARK: Types
		
			///Base container type
		using base = std::vector<std::unique_ptr<T>>;
			///Stored type
		using value_type = typename base::value_type;
			///Container size (index) type
		using size_type = typename base::size_type;
			///Container iterator type
		using iterator = typename base::iterator;
			///Container const iterator type
		using const_iterator = typename base::const_iterator;
		
		// MARK: Constructors
		
		/*!
			Constructor
			@param toReserve The number of items to reserve space for in the vector
		*/
		Vector(size_type toReserve = 0) : base() { if (toReserve != 0) base::reserve(toReserve); }
		/*!
			Constructor
			@param items The items to insert in the array
		*/
		template<class Derived>
		explicit Vector(const std::initializer_list<Derived>& items) : base() {
			for (const auto& item : items)
				emplace_back(item);
		}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Vector(const Vector& source) : base() { cloneFrom(source); }
		/*!
			Move constructor
			@param source The object to move
		*/
		Vector(Vector&& source) : base(std::move(source)) {}
		/*!
			Destructor
		*/
		virtual ~Vector() = default;
		
		// MARK: Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		auto operator= (const Vector& source) {
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
		auto operator= (Vector&& source) {
			if (this != &source)
				base::operator=(std::move(source));
			return *this;
		}
		
		// MARK: Functions (mutating)
		
		/*!
			Push an item at the end of the vector
			@param item The item to push (ownership passes to this vector)
		*/
		void push_back(T* item) { base::push_back(value_type{item}); }
		/*!
			Push an item at the end of the vector
			@param item The item to push
		*/
		void emplace_back(T&& item) {
			if constexpr(utility::Movable<T>)
				base::emplace_back(cloneMove(std::move(item)));
			else
				base::emplace_back(clone(item));
		}
		/*!
			Push an item at the end of the vector
			@param item The item to push
		*/
		void emplace_back(const T& item) { base::emplace_back(clone(item)); }
		/*!
			Push an item at the end of the vector
			@param item The item to push (already wrapped in unique_ptr)
		*/
		void emplace_back(value_type&& item) { base::emplace_back(std::move(item)); }
		/*!
			Push an item at the end of the vector
			@param item The item to push (already wrapped in unique_ptr)
		*/
		void emplace_back(value_type& item) { base::emplace_back(std::move(item)); }
		/*!
			Insert an item into the vector at a specified position
			@param pos The insertion position
			@param item The item to push (ownership passes to this vector)
		*/
		auto insert(const_iterator pos, T* item) { return base::insert(pos, value_type{item}); }
		/*!
			Emplace an item into the vector at a specified position
			@param pos The insertion position
			@param item The item to push (ownership passes to this vector)
			@returns An iterator pointing to the emplaced item
		*/
		auto emplace(const_iterator pos, T&& item) {
			if constexpr(utility::Movable<decltype(item)>)
				return base::emplace(pos, cloneMove(item));
			else
				return base::emplace(pos, clone(item));
		}
		/*!
		 Emplace an item into the vector at a specified position
			@param pos The insertion position
			@param item The item to push (ownership passes to this vector)
			@returns An iterator pointing to the emplaced item
		*/
		auto emplace(const_iterator pos, value_type& item) { return base::emplace(pos, std::move(item)); }
		/*!
			Release the pointer held by the specified item and erase
			@param pos An iterator pointing to the item to be removed
			@return An iterator at the next value
		*/
		auto release(iterator& pos) { return release(const_iterator(pos)); }
		/*!
			Release the pointer held by the specified item and erase
			@param pos An iterator pointing to the item to be removed
		 	@return The released pointer from the list item
		*/
		auto release(const_iterator pos) {
			auto item = std::move(const_cast<value_type&>(*pos));
			base::erase(pos);
			return item;
		}
		
	private:
		/*!
			Clone the contents of another vector into this
			@param source The object to copy
		*/
		void cloneFrom(const Vector& source) {
			base::reserve(source.size());
			for (const auto& item : source)
				base::push_back(item ? clone(*item) : value_type());
		}
	};
	
}

#endif	//ACTIVE_CONTAINER_VECTOR
