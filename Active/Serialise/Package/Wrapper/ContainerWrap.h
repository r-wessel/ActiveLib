/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_CONTAINER_STD_WRAP
#define ACTIVE_SERIALISE_CONTAINER_STD_WRAP

#include "Active/Serialise/CargoHold.h"
#include "Active/Serialise/Concepts.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Serialise/Package/Wrapper/PackageWrap.h"
#include "Active/Serialise/Item/Wrapper/ItemWrap.h"
#include "Active/Utility/Concepts.h"

namespace active::serialise {
	
	/*!
	 A wrapper class to (de)serialising a container of objects
	 @tparam Container The container type, e.g. std::vector<String>, Vector<Point> etc
	 @tparam ObjWrapper The object wrapper to use - only relevant for Package-based contents, notably for using serialise::Mover
	 
	 This can be used in a wide variety of cases including:
	 - STL containers, e.g. std::vector or std::list. Contents can include:
		- Primitive types, e.g. double, int32_t, String etc
	 	- Item cargo, i.e. subclassed from serialise::Item
	 	- Package cargo, i.e. subclassed from serialise::Package
	 - Specialised containers of unique_ptrs, e.g. container::Vector, container::List. Contents can include:
	 	- Item cargo, i.e. subclassed from serialise::Item
	 	- Package cargo, i.e. subclassed from serialise::Package
	 */
	template<typename Container, typename ObjWrapper = serialise::PackageWrap>
	requires (IsWrappableValue<typename Container::value_type> || IsCargo<typename Container::value_type>)
	class ContainerWrap : public Package, public std::reference_wrapper<Container> {
	public:
		
			//Default container element tag
		static inline utility::String defaultTag = "Item";
			///Serialisation fields
		enum FieldIndex {
			itemID,
		};

		// MARK: Types
		
		using Obj = typename Container::value_type;
		using wrapped_t = ContainerWrap<Container>;
			///Base container type
		using base = std::reference_wrapper<Container>;
		
		// MARK: Constructors
		
		/*!
		 Constructor
		 @param container The container to wrap
		 @param isEmptySent True if the container is still required to be sent even when empty
		 @param tg The default item tag
		 */
		ContainerWrap(Container& container, bool isEmptySent = false, const utility::String& tg = defaultTag) :
				base{container}, isEmptyRequired(isEmptySent), tag{tg} {}
		/*!
		 Constructor
		 @param container The container to wrap
		 @param isEmptySent True if the container is still required to be sent even when empty
		 @param tg The default item tag
		 */
		ContainerWrap(const Container& container, bool isEmptySent = false, const utility::String& tg = defaultTag) :
				ContainerWrap{const_cast<Container&>(container), isEmptySent, defaultTag} {}
		/*!
		 Destructor
		*/
		virtual ~ContainerWrap() = default;
			
		// MARK: - Public variables
		
			//The container item tag
		utility::String tag;
			//True if empty containers should still be serialised
		bool isEmptyRequired;
			
		// MARK: - Functions (const)
		
		/*!
		 Fill an inventory with the cargo items
		 @param inventory The inventory to receive the cargo items
		 @return True if items have been added to the inventory
		 */
		bool fillInventory(Inventory& inventory) const override {
			inventory.merge(Entry{tag, itemID, static_cast<uint32_t>(base::get().size()), std::nullopt,
					(base::get().size() > 0) || isEmptyRequired, &typeid(wrapped_t)}.withType(Entry::Type::array));
			return true;
		}
		/*!
		 Get the specified cargo
		 @param item The inventory item to retrieve
		 @return The requested cargo (nullptr on failure)
		 */
		Cargo::Unique getCargo(const Inventory::Item& item) const override {
			if (item.ownerType != &typeid(wrapped_t))
				return nullptr;
			using namespace active::serialise;
			switch (item.index) {
				case itemID: {
					if constexpr (active::utility::Dereferenceable<Obj>) {
						using Value = typename Obj::element_type;
						using PackType = serialise::CargoHold<ObjWrapper, Value>;
						using ItemType = serialise::CargoHold<serialise::ItemWrap, Value>;
						if (item.available < base::get().size()) {
							auto iter = base::get().begin();
							std::advance(iter, item.available);
							if constexpr (std::is_base_of_v<Package, Value>)
								return std::make_unique<PackType>(**iter);
							else
								return std::make_unique<ItemType>(**iter);
						} else {
							if constexpr (std::is_base_of_v<Package, Value>)
								return std::make_unique<PackType>();
							else
								return std::make_unique<ItemType>();
						}
					} else {
						using PackType = serialise::CargoHold<ObjWrapper, Obj>;
						using ItemType = serialise::CargoHold<serialise::ItemWrap, Obj>;
						using ValueType = serialise::CargoHold<serialise::ValueWrap<Obj>, Obj>;
						if (item.available < base::get().size()) {
							auto iter = base::get().begin();
							std::advance(iter, item.available);
							if constexpr (std::is_base_of_v<Package, Obj>)
								return std::make_unique<PackType>(*iter);
							else {
								if constexpr (IsWrappableValue<Obj>)
									return std::make_unique<ValueType>(*iter);
								else
									return std::make_unique<ItemType>(*iter);
							}
						} else {
							if constexpr (std::is_base_of_v<Package, Obj>)
								return std::make_unique<PackType>();
							else {
								if constexpr (IsWrappableValue<Obj>)
									return std::make_unique<ValueType>();
								else
									return std::make_unique<ItemType>();
							}
						}
					}
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
			if (item.ownerType != &typeid(wrapped_t))
				return true;
			switch (item.index) {
				case itemID:
					if constexpr (active::utility::Dereferenceable<Obj>) {
						using Value = typename Obj::element_type;
						using PackType = serialise::CargoHold<ObjWrapper, Value>;
						using ItemType = serialise::CargoHold<serialise::ItemWrap, Value>;
						if constexpr (std::is_base_of_v<Package, Value>) {
							if (auto holder = dynamic_cast<PackType*>(cargo.get()); holder != nullptr)
								base::get().emplace_back(holder->get());
						} else {
							if (auto holder = dynamic_cast<ItemType*>(cargo.get()); holder != nullptr)
								base::get().emplace_back(holder->get());
						}
					} else {
						using PackType = serialise::CargoHold<ObjWrapper, Obj>;
						using ItemType = serialise::CargoHold<serialise::ItemWrap, Obj>;
						using ValueType = serialise::CargoHold<serialise::ValueWrap<Obj>, Obj>;
						if constexpr (std::is_base_of_v<Package, Obj>) {
							if (auto holder = dynamic_cast<PackType*>(cargo.get()); holder != nullptr)
								base::get().emplace_back(holder->get());
						} else {
							if constexpr (IsWrappableValue<Obj>) {
								if (auto holder = dynamic_cast<ValueType*>(cargo.get()); holder != nullptr)
									base::get().emplace_back(holder->get());
							} else {
								if (auto holder = dynamic_cast<ItemType*>(cargo.get()); holder != nullptr)
									base::get().emplace_back(holder->get());
							}
						}
					}
					break;
				default:
					break;
			}
			return true;
		}
		
	private:
	};
	
}

#endif	//ACTIVE_SERIALISE_CONTAINER_STD_WRAP
