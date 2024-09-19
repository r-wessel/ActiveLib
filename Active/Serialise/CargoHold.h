/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CARGO_HOLD
#define ACTIVE_SERIALISE_CARGO_HOLD

#include "Active/Serialise/Cargo.h"
#include "Active/Serialise/CargoType.h"
#include "Active/Serialise/Item/UnknownItem.h"
#include "Active/Serialise/Package/NullPackage.h"

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
		CargoHold() : Wrap{m_nullCargo} {	//Use the static member as a placeholder for constructing a valiud reference
			if constexpr (std::is_default_constructible_v<Obj>) {	//Make an object instance when possible - other cases rely on an object maker
				m_cache = std::make_unique<Obj>();	//Then create a new instance to populate
				m_object = m_cache.get();
				Wrap::operator=(*m_cache);	//And point the reference to the new instance
			}
		}
		/*!
			Constructor
		*/
		CargoHold(const Obj& obj) : Wrap{obj} {
			if constexpr (std::is_default_constructible_v<Obj> && !std::has_virtual_destructor_v<Obj>) {	//Make an object instance when possible - other cases rely on an object maker
				m_cache = std::make_unique<Obj>(obj);
				m_object = m_cache.get();
				Wrap::operator=(*m_cache);	//And point the reference to the new instance
			} else {
				m_object = const_cast<Obj*>(&obj);
			}
		}
		CargoHold(const CargoHold&) = delete;
		/*!
			Destructor
		*/
		virtual ~CargoHold() = default;
		
		// MARK: - Constructors
		
		/*!
			Get a reference to the wrapped object
			@return A reference to the wrapped object
		*/
		Obj& get() { return *m_object; }
		/*!
			Get a reference to the wrapped object
			@return A reference to the wrapped object
		*/
		const Obj& get() const { return *m_object; }
		
	private:
		static CargoPicker<Wrap>::CargoType m_nullCargo;
		Obj* m_object = nullptr;
		std::unique_ptr<Obj> m_cache;
	};
	
	
	template<typename Wrap, typename Obj> requires std::is_base_of_v<Cargo, Wrap>
	typename CargoPicker<Wrap>::CargoType CargoHold<Wrap, Obj>::m_nullCargo;

}

#endif	//ACTIVE_SERIALISE_CARGO_HOLD
