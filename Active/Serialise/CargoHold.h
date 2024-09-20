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
#include "Active/Utility/Concepts.h"

#include <type_traits>

namespace active::serialise {
	
	/*!
	 Class binding a cargo wrapper referencing an embedded object (effectively a shortcut to making a cargo-capable version)
	 
	 In most cases, serialisation wrappers just hold a reference to an original instance of the data to be tranported. This technique can't be used
	 to write temporary values (i.e. the reference would become invalid) or to deserialise incoming values where a data placeholder has not been
	 allocated, e.g. expecially for polygmorphic classes where the exact type is not known in advance
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
		 Default constructor (wraps a null placeholder - can be used for deserialisation incoming concrete or polymorphic objects)
		 */
		CargoHold() : Wrap{m_nullCargo} {	//Use the static member as a placeholder for constructing a valiud reference
			if constexpr (std::is_default_constructible_v<Obj>) {	//Make an object instance when possible - other cases rely on an object maker
				m_cache = std::make_unique<Obj>();	//Then create a new instance to populate
				m_object = m_cache.get();
				Wrap::operator=(*m_cache);	//And point the reference to the new instance
			}
		}
		/*!
		 Constructor (use to transport concrete types by value - avoid where polymorphic behaviours required)
		 @param obj An object to be tranported (by value)
		 */
		CargoHold(const Obj& obj) : Wrap{obj} {
			if constexpr (std::has_virtual_destructor_v<Obj>)
				m_object = const_cast<Obj*>(&obj);
			else {
				m_cache = std::make_unique<Obj>(obj);
				m_object = m_cache.get();
				Wrap::operator=(*m_cache);	//And point the reference to the new instance
			}
		}
		/*!
		 Constructor (use to tranport objects using a base abstract class type, e.g. requiring polymorphic type labelling in serialisation)
		 @param obj A unique pointer to the object to be transported
		 */
		CargoHold(std::unique_ptr<Obj> obj) : Wrap{*obj} {
			m_cache = std::move(obj);
			m_object = m_cache.get();
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
