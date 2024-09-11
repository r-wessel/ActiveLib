/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_CONTAINER_MAP
#define ACTIVE_CONTAINER_MAP

#include "Active/Utility/Cloner.h"
#include "Active/Utility/Mover.h"

#include <algorithm>
#include <map>
#include <vector>

namespace active::container {

	/*!
		A specialisation class for a map of objects wrapped in unique_ptr paired with a key
	 
		This allows map to store objects with polymorphic behaviour and object copying (object type must conform to Clonable).
		Keep in mind that the normal behaviour of the map is to take ownership of any objects stored in it (unless release is used)
	 
		NB: There is no requirement for container items to be non-null. It is entirely up the implementor if this rule should be adopted
		or enforced
	*/
	template<class Key, class T> requires utility::Clonable<T>
	class Map : public std::map<Key, std::unique_ptr<T>> {
	public:
		
		// MARK: Types
		
			///Base container type
		using base = std::map<Key, std::unique_ptr<T>>;
			///Paired key/raw value type
		using raw_type = std::pair<const Key, T>;
			///Stored type
		using mapped_type = typename base::mapped_type;
			///Paired key/value type
		using value_type = typename base::value_type;
			///Node handle type
		using node_type = typename base::node_type;
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
		Map() : base() {}
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Map(const Map& source) : base() { cloneFrom(source); }
		/*!
			Move constructor
			@param source The object to move
		*/
		Map(Map&& source) : base(std::move(source)) {}
		/*!
			Destructor
		*/
		virtual ~Map() = default;
		
		// MARK: Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		auto operator= (const Map& source) {
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
		auto operator= (Map&& source) {
			if (this != &source)
				base::operator=(std::move(source));
			return *this;
		}
		/*!
			Subscript operator
			@param key The subscript key
			@return A reference to the value paired with the key
		*/
		mapped_type& operator[] (const Key& key) { return base::operator[](key); }
		
		// MARK: Functions (const)
		
		/*!
			Find a value with a specified key
			@return An iterator pointing to the key/value pair (end() on failure)
		*/
		auto find(const Key& key) const { return base::find(key); }
		/*!
			Get the map keys
			@return An array containing the map keys
		*/
		std::vector<Key> keys() const {
			std::vector<Key> result;
			std::for_each(base::begin(), base::end(), [&result](const auto& entry){ result.push_back(entry.first); });
			return result;
		}
		
		// MARK: Functions (mutating)
		
		/*!
			Insert a key/raw value pair into the map
			@param item The item to push
			@return A map iterator pointing to the matching/inserted item paired with a bool (true if a new item was inserted)
		*/
		auto insert(const raw_type& item) { return emplace(item.first, clone(item.second)); }
		/*!
			Insert a key/value pair into the map
			@param item The item to push
			@return A map iterator pointing to the matching/inserted item paired with a bool (true if a new item was inserted)
		*/
		auto insert(const value_type& item) { return emplace(item.first, clone(*item.second)); }
		/*!
			Insert a key/object pair into the map
			@param item The item to push
			@return A map iterator pointing to the matching/inserted item paired with a bool (true if a new item was inserted)
		*/
		auto insert(value_type&& item) {
			emplace(item.first, std::move(item.second));
		}
		/*!
			Insert a map node
			@param node The node to insert
			@return A map iterator pointing to the matching/inserted item paired with a bool (true if a new item was inserted)
		*/
		auto insert(node_type&& node) { return base::insert(std::move(node)); }
		/*!
			Emplace a key/value pair into the map
			@param item The key/value pair to insert (already wrapped in unique_ptr)
		*/
		auto emplace(const Key& key, mapped_type&& item) { return base::emplace(key, std::move(item)); }
		/*!
			Emplace a key/value pair into the map
			@param item The item to push
		*/
		auto emplace(const Key& key, T&& item) {
			if constexpr(utility::Movable<T>)
				base::emplace(key, cloneMove(std::move(item)));
			else
				base::emplace(key, clone(item));
		}
		/*!
			Release the pointer held by the specified item and erase
			@param pos An iterator pointing to the item to be removed
			@return An iterator at the next value
		*/
		auto release(iterator& pos) {
			auto item = std::move(pos->second);
			erase(pos);
			return item;
		}
		/*!
			Release the pointer held by the specified item and erase
			@param pos An iterator pointing to the item to be removed
		 	@return The released pointer from the map item
		*/
		auto release(const_iterator pos) {
			auto& val = const_cast<std::unique_ptr<T>&>(pos->second);
			auto item = std::unique_ptr<T>{val.release()};
			base::erase(pos);
			return item;
		}
		
	private:
		/*!
			Clone the contents of another map into this
			@param source The object to copy
		*/
		void cloneFrom(const Map& source) {
			for (const auto& item : source)
				base::operator[](item.first) = (item.second ? clone(*item.second) : mapped_type());
		}
	};
	
}

#endif	//ACTIVE_CONTAINER_MAP
