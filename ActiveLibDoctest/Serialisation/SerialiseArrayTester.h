/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Container/Vector.h"
#include "Active/Serialise/Package/Package.h"
#include "Active/Utility/Cloner.h"
#include "Active/Utility/Guid.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/String.h"

/*!
	Classes for testing serialisation of an array of different classes subclassed from a common base

	This demonstrates the hardest case:
	- The classes to be serialised won't have any awareness of the serialisation processes or functions
	- The base and subclass data has to be serialised into a flattened structure, not nested into base/subclass sections
	- An instance of any subclass can only be reconstructed using certain parameters rather than a default constructor
 
	Building serialisation directly into classes is simpler, but not always practical or even possible, e.g. 3rd-party libraries. And
	compartmentalising the different class levels is simpler to implement (each level can act as a subsection in the serialised structure, e.g.:
	
		typename
 		field1
		Base
 			field2

	…is simpler to implement than:
 
 		typename
		field1
		field2

	…because the former directly echoes the class structure. And an inability to use a default constructor makes instantiation during deserialisation
	harder (because the construction parameters must be deserialised first).
 
	For this test, 2 classes (`BarA` and `BarB`) are subclassed from `Foo`. All can only be reconstructed using their original guid. The test must
	serialise and deserialise an array of these subclasses (`SerialiseArrayTester`). An equality operator is implemented to compare exported and
	imported objects to comfirm successful completion.
 
	The only public class required as a wrapper for the serialisation of the array is `SerialiseArrayWrapper`. If it was necessary to serialise `BarA`
	or `BarB` individually, their respective wrapper classes would also need to be public (`AWrapper`, `BWrapper` and `BaseWrapper`).
 
	In addition to providing serialisation of 3rd-party objects, independent wrappers can facilitate compatibility with multiple schemas (with no
	direct support from the target classes).

	JSON serialisation:
 
		{
			"content": [
				{
					"type": "typeBarA",
					"id": "F74C8696-6C1E-6F33-619B-FD8E979E68A4",
					"text": "Something"
				},
				{
					"type": "typeBarB",
					"id": "2B7B6DE3-37DA-D783-F9A8-15EC329C1E44",
					"val": 1.234
				},
				{
					"type": "typeBarA",
					"id": "57DED30C-D8CF-1913-3233-7722BED08EBA",
					"text": "Whatever"
				},
				{
					"type": "typeBarB",
					"id": "AF469661-FBFF-7140-1FCE-62A0DFCE91BE",
					"val": 98.7654
				}
			]
		}

	XML serialisation:
 
		<?xml version="1.0" encoding="utf-8"?>
		<arrayTester>
			<content type="typeBarA" id="F74C8696-6C1E-6F33-619B-FD8E979E68A4">
				<text>Something</text>
			</content>
			<content type="typeBarB" id="2B7B6DE3-37DA-D783-F9A8-15EC329C1E44">
				<val>1.234</val>
			</content>
			<content type="typeBarA" id="57DED30C-D8CF-1913-3233-7722BED08EBA">
				<text>Whatever</text>
			</content>
			<content type="typeBarB" id="AF469661-FBFF-7140-1FCE-62A0DFCE91BE">
				<val>98.7654</val>
			</content>
		</arrayTester>
*/

	///The base class for the test objects
class Foo : public active::utility::Cloner {
public:
	Foo(const active::utility::Guid& guid) : m_id{guid} {}
	Foo* clonePtr() const override	{ return new Foo(*this); }

	virtual bool operator==(const Foo& ref) const { return m_id == ref.m_id; }
	const active::utility::Guid& id() const { return m_id; }

private:
		///The base class holds an immmutable guid
	active::utility::Guid m_id = active::utility::Guid{true};
};


	///The first test class (to be serialised). Note that the constructor must be provided with a guid
	///This class has only a single member variable, but this concept can be easily extended (including child objects)
class BarA : public Foo {
public:
	BarA(const active::utility::Guid& guid, const active::utility::String& str = active::utility::String{}) : Foo{guid}, m_text{str} {}
	BarA* clonePtr() const override	{ return new BarA(*this); }

	bool operator==(const Foo& ref) const override {
		if (const auto* refA = dynamic_cast<const BarA*>(&ref); refA != nullptr)
			return Foo::operator==(ref) && (m_text == refA->m_text);
		return false;
	}
	const active::utility::String& getText() const { return m_text; }
	void setText(const active::utility::String& text) { m_text = text; }

private:
	active::utility::String m_text;
};


	///The second test class (to be serialised). Same as the previous, but with a different member variable to illustrate the method
class BarB : public Foo {
public:
	BarB(const active::utility::Guid& guid, double val = 0.0) : Foo{guid}, m_val{val} {}
	BarB* clonePtr() const override	{ return new BarB(*this); }

	bool operator==(const Foo& ref) const override {
		if (const auto* refB = dynamic_cast<const BarB*>(&ref); refB != nullptr)
			return Foo::operator==(ref) && active::math::isEqual(m_val, refB->m_val);
		return false;
	}
	double getVal() const { return m_val; }
	void setVal(double val) { m_val = val; }

private:
	double m_val;
};


	///The array to hold object instances (to be serialised). Note that this class (and the objects it contains) have no serialisation awareness
class SerialiseArrayTester : public active::container::Vector<Foo> {
public:
		//equality operator to confirm imported objects match the original exports
	bool operator==(const SerialiseArrayTester& ref) const {
		if (size() != ref.size())
			return false;
		for (auto i = 0; i < size(); ++i)
			if (!(*(*this)[i] == *ref[i]))
				return false;
		return true;
	}
};


	///The wrapper class that (de)serialises the array according to the required schema (NB: the same wrapper is used for import and export)
class SerialiseArrayWrapper : public active::serialise::Package {
public:
		///NB: This name is not exported in JSON if the object is at the outermost (root) level
	inline static active::utility::String tag = "arrayTester";

		///Wrapper is constructed with a reference to the object to be imported/exported (the target array in this case)
	SerialiseArrayWrapper(const SerialiseArrayTester& test) : m_test{const_cast<SerialiseArrayTester&>(test)}	{}
	
		///Provides an inventory of the serialisation content (the object schema)
	bool fillInventory(active::serialise::Inventory& inventory) const override;
		///Retrieves the data container associated with an item from the inventory
	Cargo::Unique getCargo(const active::serialise::Inventory::Item& item) const override;
	/*!
		Get the recommended cargo entry type
		@return The cargo entry type (nullopt = deduce automatically from cargo characteristics)
	*/
	std::optional<active::serialise::Identity::Role> entryRole() const override { return active::serialise::Identity::Role::array; }
		///Resets the target object to a default state prior to importing deserialised data
	void setDefault() override;
		///Inserts a newly deserialised object container into the target (identified by an inventory item)
	bool insert(active::serialise::Cargo::Unique&& cargo, const active::serialise::Inventory::Item& item) override;
		///Validate the content of the target after import - nothing to do in this case (and could be omitted) but included for illustration
	bool validate() override { return true; }

private:
		///The target to be (de)serialised
	SerialiseArrayTester& m_test;
};
