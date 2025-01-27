/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_SERIALISE_DOM_NODE
#define ACTIVE_SERIALISE_DOM_NODE

#include "Active/Serialise/Package/Package.h"
#include "Active/Setting/ValueSetting.h"

#include <variant>

namespace active::serialise::dom {
	
		///Concept for integer types
	template<typename T>
	concept IsInteger = !std::is_class_v<T> && !std::is_pointer_v<T> && !std::is_same_v<T, bool> && std::is_integral_v<T>;

		///Concept for integer types
	template<typename T>
	concept IsFloat = !std::is_class_v<T> && std::is_floating_point_v<T>;
	
	/*!
	 A value in a generic document object model (DOM) for serialised data transport
	 */
	struct Value : std::variant<std::monostate, bool, int64_t, double, utility::String> {

		using base = std::variant<std::monostate, bool, int64_t, double, utility::String>;
			///Indices of a value type
		enum class Index {
			undefined = 0,
			boolType,
			intType,
			floatType,
			stringType,
		};
		
		/*!
		 Conversion operator
		 @return An equivalent boolean value
		 */
		explicit operator bool() const { return setting().operator bool(); }
		/*!
		 Conversion operator
		 @return An equivalent boolean value
		 */
		template<typename T> requires IsInteger<T>
		operator T() const { return static_cast<T>(setting().operator int64_t()); }
		/*!
		 Conversion operator
		 @return An equivalent boolean value
		 */
		template<typename T> requires IsFloat<T>
		operator T() const { return static_cast<T>(setting().operator double()); }
		/*!
		 Conversion operator
		 @return An equivalent boolean value
		 */
		explicit operator utility::String() const { return setting().operator utility::String(); }

		/*!
		 Get the index of the value type
		 @return The value type index
		 */
		Index index() const { return static_cast<Index>(base::index()); }
		/*!
		 Get the value setting (allowing for anonymous conversion to a variety of value types)
		 @return The value setting
		 */
		setting::ValueSetting setting() const;
	};
	
		///Concept for value types
	template<typename T>
	concept IsValue = requires(T t) {
		{ Value{t} };
	};
	
	class Node;
	
		///Concept for node assignable types
	template<typename T>
	concept IsNodeAssignable = requires(Node& node, const T& t) {
		requires !IsValue<T>;
		{ pack(node, t) } -> std::same_as<Node&>;
	};
	
		///Concept for node transferable types
	template<typename T>
	concept IsNodeTransferable = requires(const Node& node, T t) {
		requires !IsValue<T>;
		{ unpack(node, t) } -> std::same_as<const Node&>;
	};

	/*!
	 An object in a generic document object model (DOM) for serialised data transport
	 
	 Object members are expected to be paired with a name, i.e. as a dictionary
	 */
	using Object = std::unordered_map<utility::String, Node>;
	
	/*!
	 An array in a generic document object model (DOM) for serialised data transport
	 
	 Array items are unnamed
	 */
	struct Array : std::vector<Node> {
			///Optional tag for the array items (NB: Unused for JSON. Optional for XML - otherwise the array is flattened and items use the parent tag
		utility::String itemTag;

		/*!
		 Define an array item tag
		 @param tag The item tag
		 */
		Array& withItemTag(const utility::String& tag) {
			itemTag = tag;
			return *this;
		}
	};
	
	template<typename T>
	concept IsSequenceContainer = requires(T t) {
		requires IsValue<typename T::value_type>;
		{ t.begin() };
		{ t.end() };
		{ t.front() } -> std::same_as<typename T::value_type&>;
	};
	
	template<typename T>
	concept IsAssociativeContainer = requires(T t) {
		requires !IsSequenceContainer<T> && IsValue<typename T::value_type::second_type> && std::is_convertible_v<typename T::key_type, utility::String>;
		{ t.begin() };
		{ t.end() };
	};
	
	/*!
	 A node in a generic document object model (DOM) for serialised data transport
	 
	 The DOM is essentially a hierarchy of nodes, each of which can be a value, object or array. The intention is to allow (de)serialisation of
	 ad-hoc data. It can be as simply as a single root node containing a value, or a deeply nested tree containing any combination of objects,
	 arrays and values.
	 */
	class Node : protected std::variant<std::monostate, Value, Object, Array>, public Package {
	public:

		// MARK: - Types
		
		using base = std::variant<std::monostate, Value, Object, Array>;
			///Unique pointer
		using Unique = std::unique_ptr<Node>;
			///Shared pointer
		using Shared = std::shared_ptr<Node>;
			///Optional
		using Option = std::optional<Node>;
			///Indices of possible node values
		enum class Index {
			undefined = 0,
			value,
			object,
			array,
		};
		
		// MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		Node() {}
		/*!
		 Constructor
		 @param value The node value
		 */
		Node(const Value& value) : base{value} {}
		/*!
		 Constructor
		 @param value The node value
		 */
		Node(Value&& value) : base{value} {}
		/*!
		 Constructor
		 @param value The node value
		 */
		template<typename T> requires IsValue<T>
		Node(const T& value) : base{Value{value}} {}
		/*!
		 Constructor
		 @param object The object
		 */
		template<typename T> requires IsNodeAssignable<T>
		Node(const T& object) : base{Object{}} { pack(*this, object); }
		/*!
		 Constructor
		 @param object The node object
		 */
		Node(Object&& object) : base{object} {}
		/*!
		 Constructor
		 @param array The node array
		 */
		Node(Array&& array) : base{array} {}
		/*!
		 Constructor
		 @param nodes Nodes to populate an array
		*/
		Node(const std::initializer_list<Node>& nodes);
			///Use the default copy
		Node(const Node& source) = default;
			///Use the default rvalue copy
		Node(Node&& source) noexcept = default;
		
		// MARK: Operators
		
			///Use default assignment
		Node& operator=(Node&& source) = default;
			///Use default assignment
		Node& operator=(const Node& source) = default;
		/*!
		 Assigment operator
		 @param val The value to assign
		 @return A reference to this
		 */
		template<typename T> requires IsValue<T> || IsNodeAssignable<T>
		Node& operator=(const T& val) {
			if constexpr(IsValue<T>)
				base::operator=(Value{val});
			else
				return pack(*this, val);
			return *this;
		}
		/*!
		 Assigment operator
		 @param container The sequence container of values to assign
		 @return A reference to this
		 */
		template<typename Container> requires IsSequenceContainer<Container>
		Node& operator=(Container container) {
			Array array;
			for (const auto& val : container)
				array.emplace_back(Value{val});
			base::operator=(array);
			return *this;
		}
		/*!
		 Assigment operator
		 @param container An associative container of values to assign
		 @return A reference to this
		 */
		template<typename Container> requires IsAssociativeContainer<Container>
		Node& operator=(Container container) {
			Object object;
			for (const auto& item : container)
				object.insert({utility::String{item.first}, item.second});
			base::operator=(object);
			return *this;
		}
		/*!
		 Subscript operator (NB: assumes node is an object - throws otherwise)
		 @param memberName The object member name
		 @return The member node (creates if missing)
		 */
		Node& operator[](const utility::String& memberName) { return object()[memberName]; }
		/*!
		 Subscript operator (NB: assumes node is an object - throws otherwise)
		 @param memberName The object member name
		 @return The member node (creates if missing)
		 */
		template<typename T> requires std::is_same_v<T, const char*>	//NB: This prevents some compilers from transforming a 0 index into nullptr
		Node& operator[](T memberName) { return object()[utility::String{memberName}]; }
		/*!
		 Subscript operator (NB: assumes node is an array - throws otherwise)
		 @param index Index into the required array value
		 @return The indexed value
		 */
		Node& operator[](size_t index) { return array()[index]; }
		/*!
		 Conversion operator (assumes node is a value - throws otherwise)
		 @return An equivalent boolean value
		 */
		template<typename T> requires IsValue<T> || IsNodeTransferable<T>
		operator T() const {
			if constexpr(IsValue<T>)
				return value().setting().operator T();
			else {
				T obj;
				unpack(*this, obj);
				return obj;
			}
		}
		/*!
		 Subscript operator (NB: assumes node is an object - throws otherwise)
		 @param memberName The object member name
		 @return The member node (throws if not found)
		 */
		const Node& operator[](const utility::String& memberName) const {
			if (auto iter = object().find(memberName); iter != object().end())
				return iter->second;
			throw std::out_of_range("Node name not found");
		}
		/*!
		 Subscript operator (NB: assumes node is an object - throws otherwise)
		 @param memberName The object member name
		 @return The member node (throws if not found)
		 */
		const Node& operator[](const char* memberName) const {
			if (auto iter = object().find(utility::String{memberName}); iter != object().end())
				return iter->second;
			throw std::out_of_range("Node name not found");
		}

		
		
		// MARK: - Functions (const)
		
		/*!
		 Determine if the node is empty (undefined, containing a default/undefined value or an empty object/array)
		 @return True if the node is empty
		 */
		bool empty() const;
		/*!
		 Determine if the cargo is an item, e.g. a single/homogenous value type (not an object)
		 @return True if the cargo is an item
		 */
		bool isItem() const override { return index() == Index::value; }
		/*!
		 Determine if the cargo is a value
		 @return True if the cargo is a value
		 */
		bool isValue() const { return index() == Index::value; }
		/*!
		 Determine if the node is an array
		 @return True if the node is an array
		 */
		bool isArray() const { return index() == Index::array; }
		/*!
		 Determine if the node is an object
		 @return True if the node is an object
		 */
		bool isObject() const { return index() == Index::object; }
		/*!
		 Determine if the cargo is null, i.e. has no defined content
		 @return True if the cargo is a null
		 */
		bool isNull() const override { return base::valueless_by_exception(); }
		/*!
		 Determine if the node contains a sub-node with a specified name
		 @param name The name to search for
		 @return True if the node contains the specified name
		 */
		bool contains(const utility::String name) const { return index(name).operator bool(); }
		/*!
		 Write item data to a string
		 @param dest The string to write the data to
		 @return True if the data was successfully written
		 */
		bool write(utility::String& dest) const override;
		/*!
		 Get the serialisation type for the cargo value
		 @return The item value serialisation type (nullopt = unspecified, i.e. a default is acceptable)
		 */
		std::optional<Type> type() const override;
		/*!
		 Get the index of the node type
		 @return The node type index
		 */
		Index index() const { return static_cast<Node::Index>(base::index()); }
		/*!
		 Get the index of a named item in the node
		 @param name The name to search for
		 @return The type index of the named item (nullopt if not found)
		 */
		std::optional<Index> index(const utility::String& name) const;
		/*!
		 Get the node value
		 @return The node value (throws if the node does not hold a value)
		 */
		const Value& value() const { return std::get<Value>(*this); }
		/*!
		 Get an object value setting by name
		 @param name The value name
		 @return The requested value setting (nullopt on failure)
		 */
		setting::ValueSetting::Option setting(const utility::String& name) const;
		/*!
		 Get the node object
		 @return The node object (throws if the node does not hold an object)
		 */
		const Object& object() const { return std::get<Object>(*this); }
		/*!
		 Get the node array
		 @return The node array (throws if the node does not hold an array)
		 */
		const Array& array() const { return std::get<Array>(*this); }
		/*!
		 Fill an inventory with the package items
		 @param inventory The inventory to receive the package items
		 @return True if the package has added items to the inventory
		 */
		bool fillInventory(serialise::Inventory& inventory) const override;
		/*!
		 Get the specified cargo
		 @param item The inventory item to retrieve
		 @return The requested cargo (nullptr on failure)
		 */
		serialise::Cargo::Unique getCargo(const serialise::Inventory::Item& item) const override;
		
		// MARK: - Functions (mutating)
		
		/*!
		 Get the node value
		 @return The node value (throws if the node does not hold a value)
		 */
		Value& value() { return std::get<Value>(*this); }
		/*!
		 Get the node object
		 @return The node object (throws if the node does not hold an object)
		 */
		Object& object() { return std::get<Object>(*this); }
		/*!
		 Get the node array
		 @return The node array (throws if the node does not hold an array)
		 */
		Array& array() { return std::get<Array>(*this); }
		/*!
		 Push a node into the array (assumes this node is an array - throws otherwise)
		 @param node The node to push
		 */
		void push_back(const Node& node) { array().push_back(node); }
		/*!
		 Pop the back node off the array (assumes this node is an array - throws otherwise)
		 */
		void pop_back() { array().pop_back(); }
		/*!
		 Define an array item tag (NB: this otherwise defaults to the parent array tag where required, e.g. for XML)
		 @param tag The item tag (has no effect if the node is not an array)
		 */
		Node& withItemTag(const utility::String& tag) {
			if (index() == Index::array)
				array().withItemTag(tag);
			return *this;
		}
		/*!
		 Read item data from a string
		 @param source The string to read
		 @return True if the data was successfully read
		 */
		bool read(const utility::String& source) override;
		/*!
		 Read the cargo data from the specified setting
		 @param source The setting to read
		 @return True if the data was successfully read
		 */
		bool readSetting(const setting::Value& source) override;
		/*!
		 Set to the default package content
		 */
		void setDefault() override;
		/*!
		 Insert specified cargo into the reconstruct objects - used for cargo with many instances sharing the same ID (e.g. from an array/map)
		 @param cargo The cargo to insert
		 @param item The inventory item linked with the cargo
		 @return True if the cargo was accepted (false will trigger an import failure - simply discard if this is not an error)
		 */
		bool insert(Cargo::Unique&& cargo, const serialise::Inventory::Item& item) override;
		/*!
		 Allocate inventory for new (incoming) cargo
		 @param inventory The inventory to extend
		 @param identity The cargo identity
		 @param enclosing The enclosing cargo identity
		 @return An iterator pointing to the allocated item (returns end() if inventory cannot be allocated)
		 */
		Inventory::iterator allocate(Inventory& inventory, const Identity& identity, const Identity& enclosing) override;
		/*!
		 Allocate an existing inventory item as an array
		 @param inventory The parent inventory
		 @param item The inventory item to reallocate as an array
		 @return An iterator pointing to the reallocated item (returns end() if the item cannot be reallocated as an array)
		 */
		Inventory::iterator allocateArray(Inventory& inventory, Inventory::iterator item) override;
	};


}

#endif	//ACTIVE_SERIALISE_DOM_NODE
