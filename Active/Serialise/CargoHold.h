/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CARGO_HOLD
#define ACTIVE_SERIALISE_CARGO_HOLD

#include "Active/Serialise/Cargo.h"

#include <type_traits>

namespace active::serialise {
	
	/*!
		Class binding a cargo wrapper referencing an embedded object (effectively a shortcut to making a cargo-capable version)
	*/
	template<typename Wrap, typename Obj> requires std::is_base_of_v<Cargo, Wrap>
	class CargoHold : public Wrap {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<CargoHold>;
			///Shared pointer
		using Shared = std::shared_ptr<CargoHold>;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		CargoHold() : Wrap{m_object} {}
		/*!
			Constructor
		*/
		CargoHold(const Obj& obj) : m_object{obj}, Wrap{m_object} {}
		/*!
			Constructor
		*/
		CargoHold(Obj&& obj) : m_object{std::move(obj)}, Wrap{m_object} {}
		/*!
			Copy constructor
			@param source The cargo to copy
		*/
		CargoHold(const CargoHold& source) : m_object{source.m_object}, Wrap{m_object} {}
		/*!
			Destructor
		*/
		virtual ~CargoHold() = default;
		
	private:
		Obj m_object;
	};
	
}

#endif	//ACTIVE_SERIALISE_CARGO_HOLD
