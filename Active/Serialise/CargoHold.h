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

#include "Active/Serialise/Package/Wrapper/Mover.h"

#include <type_traits>

namespace active::serialise {
	
	/*!
	 Class binding a cargo wrapper referencing an embedded object (effectively a shortcut to making a cargo-capable version)
	 
	 In most cases, serialisation wrappers just hold a reference to an original instance of the data to be tranported. This technique can't be used
	 to write temporary values (i.e. the reference would become invalid) or to deserialise incoming values where a data placeholder has not been
	 allocated, e.g. expecially for polygmorphic classes where the exact type is not known in advance
	*/
	template<typename WrapT, typename ObjT> requires std::is_base_of_v<Cargo, WrapT>
	class CargoHold : public WrapT {
	public:
		// MARK: - Constructors
		
		/*!
		 Default constructor (wraps a null placeholder - can be used for deserialisation incoming concrete or polymorphic objects)
		 */
		CargoHold() : WrapT{m_nullCargo} {	//Use the static member as a placeholder for constructing a valiud reference
			if constexpr (std::is_default_constructible_v<ObjT>) {	//Make an object instance when possible - other cases rely on an object maker
				m_cache = std::make_unique<ObjT>();	//Then create a new instance to populate
				m_object = m_cache.get();
				WrapT::operator=(*m_cache);	//And point the reference to the new instance
			}
		}
		/*!
		 Constructor (use to transport concrete types by value - avoid where polymorphic behaviours required)
		 @param obj An object to be tranported (by value)
		 */
		CargoHold(const ObjT& obj) : WrapT{obj} {
			if constexpr (std::has_virtual_destructor_v<ObjT>)
				m_object = const_cast<ObjT*>(&obj);
			else {
				m_cache = std::make_unique<ObjT>(obj);
				m_object = m_cache.get();
				WrapT::operator=(*m_cache);	//And point the reference to the new instance
			}
		}
		/*!
		 Constructor (use to tranport objects using a base abstract class type, e.g. requiring polymorphic type labelling in serialisation)
		 @param obj A unique pointer to the object to be transported
		 */
		CargoHold(std::unique_ptr<ObjT> obj) : WrapT{*obj} {
			m_cache = std::move(obj);
			m_object = m_cache.get();
		}
		CargoHold(const CargoHold&) = delete;
		/*!
		 Destructor
		 */
		virtual ~CargoHold() = default;
		
		// MARK: - Functions (const)
		
		/*!
		 Get a reference to the wrapped object
		 @return A reference to the wrapped object
		 */
		const ObjT& get() const {
			if (auto mover = dynamic_cast<const Mover*>(this); (mover != nullptr) && !mover->isNull()) {
				if constexpr (std::is_arithmetic_v<ObjT>)
					return *(reinterpret_cast<ObjT*>(mover->getIncoming()));
				else {
					if (auto obj = dynamic_cast<ObjT*>(mover->getIncoming()); obj != nullptr)
						return *obj;
				}
			}
			if (m_object == nullptr)
				throw std::bad_alloc();
			return *m_object;
		}
		
		// MARK: - Functions (mutating)
		
		/*!
		 Get a reference to the wrapped object
		 @return A reference to the wrapped object
		 */
		ObjT& get() { return const_cast<ObjT&>(const_cast<const CargoHold<WrapT, ObjT>*>(this)->get()); }
		/*!
		 Release the cached object
		 @return The cached object (nullptr if no object has been received/allocated)
		 */
		std::unique_ptr<ObjT> release() {
			WrapT::operator=(m_nullCargo);
			m_object = nullptr;
			return std::move(m_cache);
		}

	private:
			///Placeholder to provide a reference when no object is available
		static typename CargoPicker<WrapT>::CargoType m_nullCargo;
			///Pointer to the stored object
		ObjT* m_object = nullptr;
			///Allocated object
		std::unique_ptr<ObjT> m_cache;
	};
	
	
	template<typename WrapT, typename ObjT> requires std::is_base_of_v<Cargo, WrapT>
	typename CargoPicker<WrapT>::CargoType CargoHold<WrapT, ObjT>::m_nullCargo;

}

#endif	//ACTIVE_SERIALISE_CARGO_HOLD
