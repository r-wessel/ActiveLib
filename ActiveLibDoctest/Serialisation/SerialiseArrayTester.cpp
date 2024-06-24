/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "ActiveLibDoctest/Serialisation/SerialiseArrayTester.h"

#include "Active/Serialise/Item/Wrapper/ValueWrap.h"

#include <array>

using namespace active;
using namespace active::serialise;
using namespace active::utility;

using enum Entry::Type;

/*!
	Defines the serialisation wrapper classes for `BarA`, `BarB` and `Foo`
*/

namespace {
	
		///Interface for the wrapper classes
	class BaseWrapper : public Package {
	public:
		using Unique = std::unique_ptr<BaseWrapper>;
			///The method releases a newly deserialised `Foo` instance from the wrapper (which will be inserted into the target array)
		virtual std::unique_ptr<Foo> releaseIncoming() = 0;
	};
	
		///Factory class to (de)serialise `BarA` objects
	class AWrapper : public BaseWrapper {
	public:
			///Enumerates the member fields for serialising `BarA`
		enum Field {
			text,
		};
		/*!
			The constructor can immediately be given a pointer to any object to be serialised
			During deserialisatiom, this constructor is not called until the object type and guid has been retrieved (passed in the `guid` param)
			These are found first by `FooWrapper` (below), which then creates an instance of this wrapper
		*/
		AWrapper(const BarA* a = nullptr, const Guid guid = Guid{}) {
			if (m_targetA = const_cast<BarA*>(a); m_targetA == nullptr) {
				m_newA = std::make_unique<BarA>(guid);
				m_targetA = m_newA.get();
			} else
				m_text = m_targetA->getText();
		}
		
			///The inventory specifies serialisable items in `BarA` (defines the object schema)
		bool fillInventory(active::serialise::Inventory& inventory) const override {
				///`FooWrapper` manages the guid in `Foo`, so only list the immediate members of `BarA` in the inventory
			inventory.merge(Inventory{
				{
					{ {"text"}, text, element },
				},
			}.withType(&typeid(AWrapper)));
			return true;
		}
			///Retrieve a wrapper for an immediate member of `BarA` (as requested in the inventory item)
		Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override {
			switch (item.index) {
				case text:
					return std::make_unique<StringWrap>(m_text);
			}
			return nullptr;
		}
			///Reset the data to a default state in preparation for deserialisation
		void setDefault() override {
			m_text.clear();
		}
			///Called when deserialisation is complete - we can assign the retrieved data to the object members
		bool validate() override {
			m_targetA->setText(m_text);	//The proxy received the data, and now we can call the setter to populate the deserialised object
			return true;
		}
			///The method releases a newly deserialised `Foo` instance from the wrapper (which will be inserted into the target array)
		std::unique_ptr<Foo> releaseIncoming() override { return std::unique_ptr<Foo>{m_newA.release()}; }

	private:
			///NB: The members of `BarA` are private, so we can't wrap references to them. Instead, we create proxies and then call setters later
		String m_text;
			///Pointer to the target object - for serialisation, this is passed to the constructor. For deserialisation, points to the `m_newA`
		BarA* m_targetA = nullptr;
			///Populated for deserialisation of a new `BarA` instance (with the guid deserialised by `FooWrapper`)
		std::unique_ptr<BarA> m_newA;
	};
	
	
		///Refer to `AWrapper` for documentation - this is practically identical, but wraps a `BarB` object rather than a `BarA`
	class BWrapper : public BaseWrapper {
	public:
		enum Field {
			num,
		};
		BWrapper(const BarB* b = nullptr, const Guid guid = Guid{}) {
			if (m_targetB = const_cast<BarB*>(b); m_targetB == nullptr) {
				m_newB = std::make_unique<BarB>(guid);
				m_targetB = m_newB.get();
			} else
				m_val = m_targetB->getVal();
		}
		bool fillInventory(active::serialise::Inventory& inventory) const override {
			inventory.merge(Inventory{
				{
					{ {"val"}, num, element },
				},
			}.withType(&typeid(BWrapper)));
			return true;
		}
		Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override {
			switch (item.index) {
				case num:
					return std::make_unique<DoubleWrap>(m_val);
			}
			return nullptr;
		}
		bool validate() override {
			m_targetB->setVal(m_val);
			return true;
		}
		void setDefault() override {
			m_val = 0.0;
		}
		std::unique_ptr<Foo> releaseIncoming() override { return std::unique_ptr<Foo>{m_newB.release()}; }
	private:
		double m_val;
		BarB* m_targetB = nullptr;
		std::unique_ptr<BarB> m_newB;
	};

	
		///Factory function to create a new `Foo` instance with a specified guid (used during deserialisation)
	using Factory = std::function<BaseWrapper::Unique(const Foo*, const Guid&)>;

	
		///Binds a typename to the function that create a new instance of the type
	struct Binding {
			///The type name (used in the serialised data)
		String typeName;
		const std::type_info& typeInfo;
		Factory factory;
	};
	
	
		///The bindings for `BarA` and `BarB` to their respective type names and factory functions
	static const std::array bindings = {
		Binding{"typeBarA", typeid(BarA), [](const Foo* foo, const Guid& guid){return std::make_unique<AWrapper>(dynamic_cast<const BarA*>(foo), guid); }},
		Binding{"typeBarB", typeid(BarB), [](const Foo* foo, const Guid& guid){return std::make_unique<BWrapper>(dynamic_cast<const BarB*>(foo), guid); }},
	};

	
		///Finds the binding for a specified type name
	const Binding* findBindingByName(const String& name) {
		for (const auto& binding : bindings) {
			if (binding.typeName == name)
				return &binding;
		}
		return nullptr;
	}

	
		///Finds the binding for a specified class
	const Binding* findBindingByTypeInfo(const std::type_info& info) {
		for (const auto& binding : bindings) {
			if (binding.typeInfo == info)
				return &binding;
		}
		return nullptr;
	}
	
	
		///Wrapper for `Foo` instances
	class FooWrapper : public active::serialise::Package {
	public:
			///Enumerates the member fields for serialising `Foo`
		enum Field {
			type,
			id,
		};

			///Constructs a wrapper for deserialising some (as yet unknown) kind of `Foo`
		FooWrapper() { m_isReadingAttributes = true; }
			///Constructs a wrapper for serialising an existing `Foo` instance
		FooWrapper(const Foo& outgoing) {
				//Because the type is already known, we can immediately create the correct wrapper
			if (const auto* binding = findBindingByTypeInfo(typeid(outgoing)); binding != nullptr) {
				m_wrapper = binding->factory(&outgoing, Guid{});
				m_type = binding->typeName;
				m_id = outgoing.id();
			}
		}
		
			///The attributes for a serialised `Foo` identify its type and guid - these are tagged as attributes and must be deserialised first
		bool isAttributeFirst() const override { return m_isReadingAttributes.value_or(false); }
			///The inventory specifies serialisable items in `Foo` (defines the object schema)
		bool fillInventory(active::serialise::Inventory& inventory) const override {
			if (!m_isReadingAttributes.has_value() || *m_isReadingAttributes) {
				inventory.merge(Inventory{
					{
						{ {"type"}, type, attribute },
						{ {"id"}, id, attribute },
					},
				}.withType(&typeid(FooWrapper)));
			}
			if (m_wrapper)
				m_wrapper->fillInventory(inventory);
			return true;
		}
			///Retrieve wrappers for either the type name or immediate members of `Foo` (as requested in the inventory item)
		Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override {
				//If the item isn't from this wrapper, pass the request to the wrapper for the target subclass
			if (item.ownerType != &typeid(FooWrapper)) {
				if (m_wrapper)
					return m_wrapper->getCargo(item);
				return nullptr;
			}
			switch (item.index) {
				case type:
					return std::make_unique<StringWrap>(m_type);
				case id:
					return std::make_unique<ValueWrap<Guid>>(m_id);
			}
			return nullptr;
		}
			///The method releases a newly deserialised `Foo` instance from the wrapper (which will be inserted into the target array)
		std::unique_ptr<Foo> releaseIncoming() const {
			return m_wrapper ? m_wrapper->releaseIncoming() : nullptr;
		}
			///Called when deserialisation is complete
		bool validate() override {
			return m_wrapper && m_wrapper->validate();	//Valid if we have a valid, wrapped `Foo` instance
		}
			///Reset the data to a default state in preparation for deserialisation
		void setDefault() override {
			m_type.clear();
			m_id.clear();
			m_wrapper.reset();	//This will be populated once the type and guid are deserialised
		}
			///Called when the attributes have been read (which means we should have an object type and guid for creating an object instance)
		bool finaliseAttributes() override {
			if (!m_isReadingAttributes.has_value() || !*m_isReadingAttributes || m_wrapper || !m_id)
				return false;
			m_isReadingAttributes = false;
				//Use the deserialised type name to lookup the correct binding
			if (const auto* binding = findBindingByName(m_type); binding != nullptr) {
				m_wrapper = binding->factory(nullptr, m_id);
				return true;
			}
			return false;
		}
		
	private:
		String m_type;
		Guid m_id;
		mutable BaseWrapper::Unique m_wrapper;
			//Unused for serialisation - true when deserialising the type and guid attributes
		std::optional<bool> m_isReadingAttributes;
	};


		///The indices of the array package items
	enum FieldArray {
		base,
	};

}  // namespace

	///The inventory specifies serialisable for the array (defines the object schema)
bool SerialiseArrayWrapper::fillInventory(Inventory& inventory) const {
	inventory.merge(Inventory{
		{
			{ {"content"}, base, m_test.size(), std::nullopt },
		},
	});
	return true;
}


	///Retrieve wrappers for immediate members of the array (as requested in the inventory item)
Cargo::Unique SerialiseArrayWrapper::getCargo(const Inventory::Item& item) const {
	switch (item.index) {
		case base:
			if (item.available < m_test.size())
				return std::make_unique<FooWrapper>(*m_test[item.available]);
			return std::make_unique<FooWrapper>();
	}
	return nullptr;
}


	///Reset the array to a default state in preparation for deserialisation
void SerialiseArrayWrapper::setDefault() {
	m_test.clear();
}


	///Insert a deserialised object into the array
bool SerialiseArrayWrapper::insert(Cargo::Unique&& cargo, const Inventory::Item& item) {
	if (auto* wrapper = dynamic_cast<FooWrapper*>(cargo.get()); (wrapper != nullptr)) {
		if (auto foo = wrapper->releaseIncoming(); foo) {
			m_test.emplace_back(foo);
			return true;
		}
	}
	return false;
}
