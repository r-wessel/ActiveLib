/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CONTAINER_WRAP
#define ACTIVE_SERIALISE_CONTAINER_WRAP

#include "Active/Container/Vector.h"
#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Package/PackageWrap.h"
#include "Active/Serialise/Item/Wrapper/ItemWrap.h"

namespace active::serialise {
	
	/*!
		A specialisation class for a vector of objects wrapped in unique_ptr
	 
		This allows vector to store objects with polymorphic behaviour and object copying (object type must conform to Clonable).
		Keep in mind that the normal behaviour of the vector is to take ownership of any objects stored in it (unless release is used)
	 
		NB: There is no requirement for container items to be non-null. It is entirely up the implementor if this rule should be adopted
		or enforced
	*/
	template<template<class> class Pack, class Value> requires (utility::Clonable<Value> && std::is_base_of_v<serialise::Cargo, Value>)
	class ContainerWrap : public Package, public std::reference_wrapper<Pack<Value>> {
	public:
		
			//Default container element tag
		static inline utility::String defaultTag = "Item";
			///Serialisation fields
		enum FieldIndex {
			itemID,
		};

		// MARK: Types
		
		using value_type = Pack<Value>;
			///Base container type
		using base = std::reference_wrapper<value_type>;
		
		// MARK: Constructors
		
		/*!
		 Constructor
		 @param vector The vector to wrap
		 @param isEmptySent True if the container is still required to be sent even when empty
		 @param tg The default item tag
		*/
		ContainerWrap(value_type& vector, bool isEmptySent = false, const utility::String& tg = defaultTag) :
				base{vector}, isEmptyRequired(isEmptySent), tag{tg} {}
		/*!
			Destructor
		*/
		virtual ~ContainerWrap() = default;
			
		// MARK: - Public variables
		
			//The container item tag
		utility::String tag;
			//The container item tag
		bool isEmptyRequired;
			
		// MARK: - Functions (const)
		
		/*!
			Fill an inventory with the cargo items
			@param inventory The inventory to receive the cargo items
			@return True if items have been added to the inventory
		*/
		bool fillInventory(Inventory& inventory) const override {
			inventory.merge(Entry{tag, itemID, static_cast<uint32_t>(base::get().size()), std::nullopt,
					(base::get().size() > 0) || isEmptyRequired, &typeid(ContainerWrap<Pack, Value>)}.withType(Entry::Type::array));
			return true;
		}
		/*!
			Get the specified cargo
			@param item The inventory item to retrieve
			@return The requested cargo (nullptr on failure)
		*/
		Cargo::Unique getCargo(const Inventory::Item& item) const override {
			if (item.ownerType != &typeid(ContainerWrap<Pack, Value>))
				return nullptr;
			using namespace active::serialise;
			switch (item.index) {
				case itemID: {
					Value& value = (item.available < base::get().size()) ? *(base::get()[item.available]) : m_value;
					if constexpr (std::is_base_of_v<Package, Value>)
						return std::make_unique<PackageWrap>(value);
					else
						return std::make_unique<ItemWrap>(value);
				}
				default:
					return nullptr;	//Requested an unknown index
			}
		}
	
		// MARK: - Functions (mutating)
		
		/*!
			Set to the default package content
		*/
		void setDefault() override { base::get().clear(); }
		/*!
			Insert specified cargo into the package - used for cargo with many instances sharing the same ID (e.g. from an array/map)
			@param cargo The cargo to insert
			@param item The inventory item linked with the cargo
		*/
		bool insert(Cargo::Unique&& cargo, const Inventory::Item& item) override {
			if (item.ownerType != &typeid(ContainerWrap<Pack, Value>))
				return true;
			switch (item.index) {
				case itemID:
					base::get().emplace_back(m_value);
					break;
				default:
					break;
			}
			return true;
		}
		
	private:
		mutable Value m_value;
	};
	
}

#endif	//ACTIVE_SERIALISE_CONTAINER_WRAP
