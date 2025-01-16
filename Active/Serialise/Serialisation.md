
# Serialisation

## Contents
1. [Purpose](#purpose)
2. [Application](#app)
3. [Ad hoc serialisation](#ahs)
	- [Basic Example](#ahsexample)
	- [Node Content](#nodeContent)
	- [Object Nodes](#objNodes)
	- [Array Nodes](#arrayNodes)
	- [Value Nodes](#valueNodes)
	- [Direct Object Conversion](#objconvert)
4. [Document Based Application](#dba)
	- [Implementation Example](#docexample)
	- [Optional Document Features](#optdoc)
5. [Schema Driven Application](#sda)
	- [Terminology](#terms)
	- [Serialisation Overview](#serialover)
	- [Deserialisation Overview](#deserialover)
	- [Implementation Overview](#implover)
	- [Implementation Example](#serexample)
	- [Deserialising Polymorphic Objects](#despoly)

## Purpose <a name="purpose"></a>

This module was created to address a range of serialisation requirements:

* Simple (de)serialisation of ad hoc data;
* Persisting complex data structures or relationships;
* Adapting a single data set to conform with multiple external (published) schemas;
* Migrating legacy data during deserialisation;
* Facilitating protocol changes between XML and JSON, i.e. the same code can serialise to both JSON and XML.

## Application <a name="app"></a>

This library offers 3 methods for serialising data:

1. Ad hoc  
Can be used for any (de)serialisation process, but best suited to highly dynamic situations, e.g. where the requirements frequently change or the structure of the data may be unknown. Any source can be deserialised into a generic DOM hierarchy that can be traversed for data extraction as required. Likewise, a DOM can be created on the fly from ad hoc data and serialised (as JSON, XML or whatever other Transport type has been implemented).
2. Document-based  
This approach trades some flexibility and efficiency for simplicity. Serialisation uses a built-in, minimal schema, but is capable of conveying data of any complexity. This uses an intermediate data representation (DOM) and therefore has higher memory overheads, but is simple to implement. This provides an easy way to either serialise individual records into a database as a JSON/XML blob or to read/write an entire database from/to a single, serialised file. Recommended for cases where there is no need to create or conform to any specific schema.
3. Schema-driven  
Efficient and highly customisable, but requires more effort. Serialisation can be tailored to the requirements of a specific schema including structure, names and value type/format, e.g. enabling interaction with some specific 3rd-party service. This approach is best for situations demanding detailed control over the (de)serialisation process. Note that there is no intermediate DOM involved – information flows directly between native structures and serialised data using a dependency injection container – meaning that memory overheads are typically low.

These techniques can be mixed within an application and any implementation automatically works with JSON and XML serialisation (or other Tranport-based implementations). The latter two also have specific processes to accomodate data migration and validation processes (important considerations for this library).

## Ad hoc Serialisation <a name="ahs"></a>

Applicable where flexiblity is paramount, e.g. data content is dynamic or used for prototyping/testing. This approach can be used in any context, but the other document/schema-based approaches offer distinct advantages for some situations, e.g. the schema-based serialisation has low overheads and strong schema management.
### Basic Example <a name="ahsexample"></a>

Possible applications are quite diverse. The basic principle is that a `dom::Node` is a dynamic, intermediary container for serialised data. It supports a hierarchical structure, with a root node branching into a node tree structure. Nodes can be populated using any combination of initialiser lists or algorithms.

Some simple examples follow below (checks omitted for clarity):
1. Programatically adding data to a node (for serialisation)  
Constructing a DOM node can be as simple as entering the data directly into an initialiser list, directly reflecting the serialised layout:
```cpp
Node node = Object{
	{"someBool", true},
	{"someInt", 25},
	{"someFloat", 1.234},
	{"someText", "Example"},
	{"someArray", { 1.23, 25, "text", false }},
	{"someObject", Object{
		{"objName", "Sample"},
		{"objVal", 5.432},
		{"objNum", 9876},
	}}
};
```
…and serialised using a `Transport` object, e.g. to JSON:
```cpp
String json;
JSONTransport().send(node, Identity{}, json);
```
…which generates:
```json
{
	"someInt": 25,
	"someText": "Example",
	"someFloat": 1.234,
	"someBool": true,
	"someArray": [
		1.23,
		25,
		"text",
		false
	],
	"someObject": {
		"objName": "Sample",
		"objVal": 5.432,
		"objNum": 9876
	}
}
```
…or to XML:
```cpp
String xml;
XMLTransport().send(node, Identity{"sampleXML"}, xml);
```
…which generates:
```xml
<?xml version="1.0" encoding="utf-8"?>
<sampleXML>
  <someInt>25</someInt>
  <someText>Example</someText>
  <someFloat>1.234</someFloat>
  <someBool>true</someBool>
  <someArray>
    <item>1.23</item>
    <item>25</item>
    <item>text</item>
    <item>false</item>
  </exampleArray>
  <someObject>
    <objName>Sample</objName>
    <objVal>5.432</objVal>
    <objNum>9876</objNum>
  </someObject>
</sampleXML>
```
> NOTE: XML requires an enclosing tag at the root level ('sampleXML' in this case) where JSON does not. If you intend to serialise to XML, define an enclosing `Identity` in the call to `XMLTransport::send`. An enclosing identity can similarly be defined for JSONTransport serialisation, but it has no effect.

2. Receiving serialised data  
The JSON created in the above example can be deserialised back into a DOM node:
```cpp
Node incoming;
JSONTransport().receive(incoming, Identity{}, json);
```
…and values can be extracted from the deserialised node:
```cpp
bool val = incoming["someBool"];	//val == true
String text = incoming["someText"];	//text == "Example"
double arrayVal = incoming["someArray"][0];	//arrayVal == 1.23
String arrayText = incoming["someArray"][2];	//arrayText == "text"
double objectVal = incoming["someObject"]["objVal"];	//arrayVal == 5.432
String objectText = incoming["someObject"]["objName"];	//arrayText == "Sample"
```

### Node Content <a name="ahs"></a>  

A `dom::Node` can contain be of the following:

1. Object:
An unordered map pairing a name (String) key with a `dom::Node` value, i.e. `std::unordered_map<String, dom::Node>`

2. Array:
An array of `dom::Node`, i.e. `std::vector<dom::Node>`

3. Value:
A single boolean/integer/floating/string value (can also be undefined, i.e. null)

4. Undefined:
No content - equivalent to null in JSON.

The node type can be tested by calling `Node::index()`, which returns a `Node::Index` enumerator value, e.g.:
```cpp
switch (node.index()) {
	case Index::value:
		...
	case Index::object:
		...
	case Index::array:
		...
	case Index::undefined:
		...
}

```
A number of Node methods assert a specific content type, e.g. `Node::object()` returns the node as an `Object`. An exception is thrown if the assertion is false.

### Object Nodes <a name="objNodes"></a>

An object node contains any number of names paired with a Node (i.e. `unordered_map<String, Node>`). A new object node can be constructed from an initialiser list:
```cpp
Node node = Object{
	{"firstName", "Joe"},
	{"surname", "Bloggs"},
	{"streetNo", 123},
	{"height", 1.82},
	{"phone", Object{
			{"home", "0123456789"},
			{"work", "0987654321"},
		}
	}
	{"wfh", true},
};
```
…and/or defined piecemeal:
```cpp
Node node(Object{});
node["firstName"] = "Joe";
node["surname"] = "Bloggs",
node["streetNo"] = 123,
node["height"] = 1.82,
Node phone(Object{});
phone["home"] = "0123456789";
phone["work"] = "0987654321";
node["phone"] = phone;
node["wfh"] = true;
```
The node can be exported to JSON:
```cpp
String json;
XMLTransport().send(node, Identity{}, json);
```
…resulting in:
```Json
{
	"firstName": "Joe",
	"height": 1.82,
	"surname": "Bloggs",
	"streetNo": 123,
	"phone": {
		"home": "0123456789",
		"work": "0987654321"
	},
	"wfh": true
}
```
…or exported to XML:
```cpp
String xml;
XMLTransport().send(node, Identity{"contact"}, xml);
```
…resulting in:
```Xml
<?xml version="1.0" encoding="utf-8"?>
<contact>
  <firstName>Joe</firstName>
  <height>1.82</height>
  <surname>Bloggs</surname>
  <streetNo>123</streetNo>
  <phone>
    <home>0123456789</home>
    <work>0987654321</work>
  </phone>
  <wfh>true</wfh>
</contact>
```

Object nodes can also be constructed directly from STL associative containers, e.g.:
```cpp
Node mapNode = std::map<utility::String, int32_t>{
	{ "first", 1 },
	{ "second", 2 },
	{ "third", 3 },
};
```

…and used in range-based for loops, e.g.:
```cpp
for (auto& item : mapNode.object()) {
	auto key = item.first;
	int32_t value = item.second;
}
```

Values can be extracted from an object node in two ways:

1. Subscript operator (by name - const):
An exception will be thrown if the node is either not an object or the name does not exist. The required type is deduced from the variable to receive the value.
```cpp
double height = node["height"];
```
2. Node::setting(const String& name):
The return type is a ValueSetting. If the node is not an object or the name is not found, it will be populated with NullValue{}. Otherwise it contains whatever value/type was deserialised from the source, but can be transformed to any other type.
```cpp
double height = node.setting("height").value_or(DoubleValue{0.0});
```


### Array Nodes <a name="arrayNodes"></a>  

An array node contains any number of Node items (i.e. `vector<Node>`). A new array node can be constructed from an initialiser list:
```cpp
Node node = {
	"Joe",
	"Bloggs",
	123,
	1.82,
	true,
};
```

> TIP: Be mindful to use an appropriate form of Node constructor to avoid accidentally creating an initialiser list, e.g.:
> ```cpp
> Node node{Object{}};	//Constructs an array node containing an object node
> ```
> …vs
> ```cpp
> Node node(Object{});	//Constructs an object node
> ```
Node content can also be assembled piecemeal:
```cpp
Node testRoot(Array{});
Node node(Array{});
node.push_back("Joe");
node.push_back("Bloggs");
node.push_back(123);
node.push_back(1.82);
node.push_back(true);
testRoot.push_back(node.withItemTag("info"));
```
The node can be exported to JSON:
```cpp
String json;
JSONTransport().send(testRoot, Identity{}, json);
```
…resulting in:
```json
[
	[
		"Joe",
		"Bloggs",
		123,
		1.82,
		true
	]
]
```
…or exported to XML:
```cpp
String xml;
XMLTransport().send(testRoot.withItemTag("contact"), Identity{"contacts"}, xml);
```
…resulting in:
```xml
<?xml version="1.0" encoding="utf-8"?>
<contacts>
  <contact>
    <info>Joe</info>
    <info>Bloggs</info>
    <info>123</info>
    <info>1.82</info>
    <info>true</info>
  </contact>
</contacts>
```
> NOTE: XML does not explicitly define an array. An item tag is defined by calling Node::withItemTag in the line `testRoot.push_back(node.withItemTag("info"));`. If the tag is omitted, the array becomes a flat list of <contact> elements, e.g.:
> ```cpp
> <?xml version="1.0" encoding="utf-8"?>
> <contacts>
>   <contact>Joe</contact>
>   <contact>Bloggs</contact>
>   <contact>123</contact>
>   <contact>1.82</contact>
>   <contact>true</contact>
> </contacts>
> ```

Array nodes can also be constructed directly from STL sequence containers, e.g.:
```cpp
Node vectNode = std::vector{ 1, 2, 3, 4, 5, 6 };
```

…and used in range-based for loops, e.g.:
```cpp
for (auto& item : vectNode.array())
	int32_t value = item;
```

### Value Nodes <a name="valueNodes"></a>

Value nodes adapt to most value types automatically, performing conversions as required e.g.:
```cpp
Node node = 1.23;
bool boolVal = node;	//boolVal == true
uint32 intVal = node;	//intVal == 1
double val = node;	//val == 1.23
String stringVal = node;	//stringVal == "1.23"
```

The internal value type of a node can be validated using Value::index() if necessary.

### Direct Object Conversion <a name="objconvert"></a>

Conversions between custom objects and dom:Node can be defined, enabling direct assignment of an object instance to a node (and vice versa), by defining up to two functions in the active::serialise::dom namespace. The following examples will define the conversion functions for an example struct:  
```cpp
struct Example {
	String a;
	double b = 0.0;
	uint32_t c = 0;
};
```

1. Assigning an instance to dom::Node  
Define a `pack` function that populates the instance data into a Node:  
```cpp  
	//Pack an Example struct into a dom::Node
Node& pack(Node& node, const Example& test) {
	node = Object{};	//Ensure the node is an object type
	node["a"] = test.a;
	node["b"] = test.b;
	node["c"] = test.c;
	return node;
}
```  
2. Assigning an instance to dom::Node  
Define an `upack` function that extracts data from the Node to populate in instance:
```cpp  
	//Unpack an Example struct from a dom::Node
const Node& unpack(const Node& node, Example& test) {
	test.a = node["a"].operator String();
	test.b = node["b"];
	test.c = node["c"];
	return node;
}
```  

This enables simple coding for transferring data between object instances and `dom::Node`, e.g.:
```cpp
Node node(Object{});
	//Assign an object instance into a node
node["example"] = Example{"something", 1.23, 25};
	//...and assign a node to an object
Example obj = node["example"];
```

## Document Based Application <a name="dba"></a>

Applicable where a serialisation schema is not specified, simplicity is paramount and legacy data migration might need to be employed.

### Implementation Example <a name="docexample"></a>

The following class will be used as an illustration:

```cpp
class Category {
public:
	static inline const char* defaultName = "untitled";
		//Constructor and methods omitted for clarity
private:
		//Member data fields
	String m_name = defaultName;
	uint32_t m_index = 0;
		//Member objects
	std::vector<Category> m_children;
};
```

Steps for implementing document-based serialisation:
1. All member fields need a unique serialisation name/tag (i.e. unique within the class) - it's good practice to define them using a field enumerator and a string array to avoid manual repetition, e.g.:  

```cpp
enum Field {
	name,
	index,
	child,
};
std::array fieldName{
	"name",
	"index",
	"child",
};
```

2. The target class should also have a unique identifying type name, e.g.:  

```cpp
inline static const String typeName = "Category";
```

3. Add a member function to *send* the object to a serialised document `Object`, e.g.:

```cpp
doc::Object send(const SettingList* spec = nullptr) const {
	auto result = doc::Object{typeName}  //Make a new Object
			<< ValueSetting{m_name, fieldName.at(name)}  //Send member variables
			<< ValueSetting{m_index, fieldName.at(index)};
		//Send member objects
	for (auto& childCat : m_children)
		result << childCat.send(spec).withTag(fieldName.at(child));
	return result;
}
```

4. Add a new constructor to reconstruct an instance from an incoming document `Object`:

```cpp
Category(const doc::Object& incoming, const SettingList* spec = nullptr) {
		//Retrieve member variables
	m_name = incoming.value(fieldName.at(name)).value_or(ValueSetting{String{defaultName}});
	m_index = incoming.value(fieldName.at(index)).value_or(ValueSetting{0});
		//Retrieve member objects
	for (auto& member : incoming.objects) {
		if (member.tag == fieldName.at(child))
			if (auto category = member.object<Category>(); category)
				m_children.emplace_back(category);
	}
}
```

Any object implementing these functions can then be sent to a document object, e.g.:

```cpp
Category category;
auto document = category.send();
```

…and then serialised into a `String`, `File` or `Memory`, either as JSON:

```cpp
String jsonString;
JSONTransport().send(PackageWrap{document}, Object::defaultTag, jsonString); 
```

…or XML:

```cpp
Memory xmlOutput;
XMLTransport().send(PackageWrap{document}, Object::defaultTag, xmlOutput); 
```

The document can be reconstructed from serialised data by defining a `Handler` and populating it with any types it should be expected to deserialise, e.g.:

```cpp
class DocHandler : public Handler {
public:
	DocHandler() {
		add<Category>({Category::typeName});
	}
};
```

> Note:  
> The list of object types added to a Handler would typically be much longer for a real application. These can be added at any time in and optionally in separate batches (to keep awareness to classes within the tightest possible scope). A handler would also probably be constructed once (as a shared_pointer) and persist for the lifetime for the app.

An instance of this handler can be used to reconstruct the document from JSON or XML, e.g.:


```cpp
	//Rebuild the document from JSON
Object document;
JSONTransport().receive(PackageWrap{document}, Object::defaultTag, jsonString);
	//And use a handler to reconstruct an Example object from the document
auto handler = std::make_shared<DocHandler>();
auto category = document.usingHandler(handler).object<Category>();
```

### Optional Document Features <a name="optdoc"></a>

The following features are optional, but can be invaluable in some circumstances.

1. Using Wrapper Classes   
It may not be possible to add the required serialisation functionality directly to a class, e.g.:
	- It's defined in 3rd-party code
	- Knowledge of serialisation is inappropriate, e.g. in a low-level geometry library.
A wrapper class can be used in this case, holding a reference to an instance of the target class and fulfilling the required functions on its behalf.

2. Using SettingList
Both the required *send* function and constructor from a document object have an optional *spec* parameter, e.g.:

```cpp
Object send(const SettingList* spec = nullptr) const;
```

For sending objects to a document, this parameter can be populated with settings that specify the required content. For example, the user license might limit how much content they are allowed to save. Rather than scatter knowledge of licensing data all over the serialisation code base, a setting could be lodged in the *setting* parameter that can be checked by relevant objects (regulating the saved extent accordingly).   
The settings passed to the constructor can be used similarly, but also for managing migration of legacy data. A `SettingList` is able to hold both values and objects - if legacy data can't be handled at the object level (e.g. for major structural changes), it can be accumulated using object(s) in the *settings* and postprocessed when the initial deserialisation is complete. This technique can simplify migration code by focussing the code at a high level rather than scattering the knowledge across many lower-level objects.


## Schema Driven Application <a name="sda"></a>

Applicable where control over the schema is paramount.

### Terminology <a name="terms"></a>

It will be much easier to implement required serialisation functions if the terminology and processes are clearly understood. The names and processes use the freight/shipping industry as an analogy, where the details of the goods are anonymised into abstract containers for transport, i.e.:

- For export, the transporter requests an inventory for the cargo to send;
- Each entry in the inventory will provide a clear identity for each cargo item;
- The transporter will seek cargo from a sender according to the entries in the inventory;
- For import, the transporter requests an inventory for the cargo to deliver;
- The transporter will issue cargo to the recipient using the identities in the inventory entries;
- Only the sender and receiver has any knowledge of the meaning/content of the cargo. The transporter just deals with generic cargo and only connects it with the sender receiver through the inventories they provide;
- The sender/receiver have no knowledge of how the transporter functions - they simply provide inventories and send/receive cargo through the transporter.
	
The following library terms are drawn from this analogy:
- `Transport`: Sends/receives objects in a serialised form. Currently implemented by `JSONTransport` and `XMLTransport`.
- `Cargo`: Some object/structure/value to be transported (or a suitable `Wrapper` - see below). Think of this as a dependency injection container: the Transport understands the serialisation mechanics, but not the data structure/content that flows through it; the object dependencies that define structure/content are provided by a `Cargo` object.
- `Item`: A type of `Cargo`, but for single items (e.g. a number or string).
- `Package`: A type of `Cargo` capable of holding multiple `Item`s or `Package`s.
- `Wrapper`: A light-weight object that acts as a `Cargo` container on behalf of the value/object to be (de)serialised. A `Wrapper` may be used for 3rd-party classes or classes that cannot be constructed from default settings and then deserialised piecemeal.
- `Identity`: Properties that identify `Cargo` in a serialised form, e.g. a name/tag)
- `Entry`: A description of `Cargo` including:
	- its `Identity`;
	- number of instances;
	- maximum allowable instances;
	- whether the item is required;
	- if the item is an attribute.
- `Inventory`: A list of `Entry`s (effectively the serialisation schema)

### Serialisation Overview <a name="serialover"></a>

The serialisation process can be summarised as:
1. The `send` function is called in a `Transport` object (`JSONTransport` or `XMLTransport`)  to export `Cargo` with a specified `Identity` as serialised data to a `BufferOut` destination (can target a `String`, `Memory` or `File`).
2. The `Transport` object requests an `Inventory` for the `Cargo`
3. The `Cargo` is serialised with its `Identity`, e.g. `"name": "Foo"` (JSON) or `<name>Foo</name>` (XML)
4. Each `Entry` in the `Inventory` is processed in turn. If an entry specifies multiple instances, e.g. an array, each instance is also processed in turn. The instance `Cargo` is requested and processed by recursing into the export function.
5. When every `Entry` from the `Inventory` has been processed, the `Cargo` ending is serialised, e.g. `</object>`.

### Deserialisation Overview <a name="deserialover"></a>

The deserialisation process is very similar:
1. The `receive` function is called in a `Transport` object (`JSONTransport` or `XMLTransport`) to import `Cargo` with a specified `Identity` from a `BufferIn` serialised source (can target a `String`, `Memory` or `File`).
2. The `Transport` object requests an `Inventory` for the `Cargo`
3. The `Transport` import function then iterates through the source seeking `Cargo` with an `Identity` that can be registered against an `Entry` in the `Inventory`. An unmatched `Identity` indicates bad `Cargo` – this can be optionally skipped, but otherwise an exception is thrown.
4. The `Entry` details are checked to ensure the maximum allowed instances has not been exceeded – otherwise an exception is thrown
5. The instance `Cargo` is requested and processed by recursing into the import function. If the `Entry` indicates that there can be multiple instances, e.g. an array, the parent `Cargo` container is asked to insert the incoming `Cargo` instance.
6. When the end of the current serialised scope is found, the transported `Cargo` is validated – failure indicates bad `Cargo` and an exception is thrown.

### Implementation Overview <a name="implover"></a>

Serialisation and deserialisation can be implemented through the following steps (an example follows):
1. Subclassing `Item` or `Package`, either directly within the target class/value or through a wrapper class representing the target. An existing wrapper class can be also be used in many cases, e.g. `ValueWrap`, `XMLDateTime` etc.
	- Pros of direct subclassing:
		- Access to private variables/functions;
		- Minimal coding.
	- Pros of wrapper class:
		- Target class is unmodified (keeps functionality focussed, and is essential for 3rd-party code)
		- Multiple schemas can be supported for the same object (where compliance with 3rd-party schemas is required)
2. A subclass of `Package` may need to implement up to 4 key functions (as required - anything subclassed from `Item` typically needs none of them, but there can be exceptions):
	- `bool fillInventory(Inventory& inventory) const`    
The `Inventory` is essentially the serialisation schema and should be populated with one `Entry` for every field (value/object) in the target to be (de)serialised, specifying:
		- `index`: A number used to determine the serialisation order, i.e. index 0 is first
		- `maximum`: The maximum number of instance of the field (can be unlimited for an array)
		- `available`: How many instances are currently in the target, e.g. the size of an array
		- `isAttribute`: True if this field is an attribute. This is primarily important for XML, but can potentially be significant for JSON too
		- `required`: Ignored for deserialisation. Can be `false` for serialisation if the field can be skipped, e.g. it's value matches the field default value.
		- `owner`: The typeid of the field parent type. Used where a serialised element is either a composite or hierarchy of many objects, enabling each field to be bound to its parent.
	- `Cargo::Unique getCargo(const Inventory::Item& item) const`    
This function should return a `Cargo` container to send or receive data associated with the passed `Inventory::Item` (which combines an `Entry` with an `Identity`). The serialisation process will serialise and export the returned cargo. Deserialisation will populate the returned cargo with the imported deserialised data.
	- `void setDefault()`    
Called during deserialisation only, before any deserialised data is retrieved. The target object/value should reset all its fields to their default state. For an array, this probably means clearing any existing content. This function should only be implemented where there is a suitable default state. Values subclassed from `Item` typically do not need this function.
	- `bool validate()`    
Called during deserialisation only, after all the data relevant to the target has been deserialised and populated into the target. The target object/value should return `true` if the deserialised data is valid, but this can also be used as an opportunity for completion processes, e.g. migrating legacy data, caching data for objects that can't be constructed yet, or amalgamating disparate items into a more complex structure. This function can be omitted if none of these processes are required. Sometimes required for subclasses of `Item`, but rarely. 
	- `bool insert(Cargo::Unique&& cargo, const Inventory::Item& item)`   
Required for deserialising arrays/dictionaries. When a new array instance is found, a `Cargo` instance is requested (using *getCargo*) to receive the deserialised data. Once the instance has been read and validated, this *insert* function is called. The *cargo* parameter is the new instance to be inserted, and *item* associates the *cargo* with an inventory entry (essential for objects that hold multiple arrays).
3. A subclass of `Item` may also need to implement the following functions (if it isn't already using an existing `Wrapper`:
	- `bool write(utility::String& dest) const`    
Convert the `Item` value into a string for serialisation. Returning `false` indicates a bad value (`Transport` will throw an exception).
	- `bool read(const utility::String& source)`    
Convert a serialised string back into a value. Returning `false` indicates a bad value (`Transport` will throw an exception)

Any instance of a `Package` subclass, e.g. *someObject*,  can then be serialised into a `String` as JSON:

```cpp
String jsonOutput;
JSONTransport().send(PackageWrap{someObject}, Identity{}, jsonOutput); 
```

…or as XML:

```cpp
String xmlOutput;
XMLTransport().send(PackageWrap{someObject}, Identity{}, xmlOutput); 
```

### Implementation Example <a name="serexample"></a>

The `Object` class of the ActiveLib serialisation module is used as an implementation example (Active/Serialise/Document/Object.h), which is at the heart of the document-based serialisation described in the next section. This object uses the schema-based approach to implement a simple interface for writing anything from simple values to complex data structures based on a fixed, simple schema, essentially a hierarchy starting from a root object that may contain any number of nested objects/values. Any object may also have nested objects/values:
<i>

- Object
	- Value0
	- Value1
	- Object
		- Value0
		- Object
	- Object
		- Value0
		- Value1
		- Value2    
...etc

</i>
The `Object` has the following fields (which will be reflected in the serialisation schema):

- A class name (not necessarily literal, but able to uniquely identify a type)
- An optional tag - differentiates betweem multiple instances of objects of the same class intended for different purposes
- An optional array containing member values
- An optional array containing member objects

It's good practice to make an enumerator for the fields, e.g.:

```cpp
enum FieldIndex {
	typeID,
	tagID,
	val,
	obj,
};
```

Then an `Inventory` (the serialisation schema) can be defined like this:

```cpp
bool Object::fillInventory(Inventory& inventory) const {
	using enum Identity::Role;
	inventory.merge(Inventory{
		{
			{ {"class"}, typeID, attribute },
			{ {"tag"}, tagID, attribute, !tag.empty() },
			{ {"val"}, val, static_cast<uint32_t>(values.size()), std::nullopt, values.size() > 0 },
			{ {"obj"}, obj, static_cast<uint32_t>(objects.size()), std::nullopt, objects.size() > 0 },
		},
	}.withType(&typeid(Object)));
	return true;
}
```

> Note: A meaningful efficiency gain can be made by defining constants for the `Identity` and references to them when defining the inventory. Refer to the `serialise::doc::Object` class implementation for an example. 


The inventory is defined as a series of entries (one per field), each pairing the field's serialised tag/name with a schema/description, specifically:

- An index, unique only within this object
- For single instance fields:
	- The value type, e.g. *attribute* or *element* (mostly relevant to XML, but also useful for JSON in some circumstances
- For arrays:
	- The current number of instances in the field, e.g. an array could be higher than 1
	- The maximum number of instances the field can support (std::nullopt for unlimited)
- *true* if the field is required (*false* means omit from serialisation output, e.g. if it matches the field default value). In the above example, for example, 'tagID' is only required when non-empty
- An optional *typeid* for the parent class/type. Only necessary in hierarchical structures, where the *typeid* ensures entries are paired to the correct level.

A function to provide a value/object instance for transport can look like this:

```cpp
Cargo::Unique Object::getCargo(const Inventory::Item& item) const {
		//Ensure this item is from my inventory
	if (item.second.ownerType != &typeid(Object))
		return nullptr;
		//Then find an object to handle the requested item
	switch (item.second.index) {
		case typeID:
			return std::make_unique<ValueWrap<String>>(docType);
		case tagID:
			return std::make_unique<ValueWrap<String>>(tag);
		case val:
			if (item.second.available < values.size())
				return std::make_unique<XMLValueSetting>(values[item.second.available]);
			return std::make_unique<XMLValueSetting>(m_incoming);
		case obj:
			if (item.second.available < objects.size())
				return std::make_unique<PackageWrap>(objects[item.second.available]);
			return std::make_unique<Object>(String());
		default:
			return nullptr;	//Requested an unknown index
	}
}
```

This function provides dependency injection container functionality, pairing one of its fields with the specified `Identity` and returning a relevant `Cargo` (dependency) object. If the `Identity` is unmatched, *nullptr* should be returned (which can optionally throw a failure exception).

In this example, the function first checks if the identity *typeid* is a match - it has no superclass, so a mismatch is a failure. A switch statement on the identity *index* then extracts the relevant `Cargo`.

The response for *typeID* and *tagID* fields is to return a data reference in a suitable wrapper.
> Note:  
> The template functions for ValueWrap::read and ValueWrap::write can be specialised for serialising custom types, e.g. enumerators.

The response for the array field (*val* and *obj*) is more complex. Both serialisation and deserialisation call this function and set the value of *item.second.available* to the index of the required instance. During serialisation, this will always be an existing index in the array (asking for the nth instance). In deserialisation, the index will be higher than the number of existing array instances (because it wants to add a instance from the deserialised data). Therefore, the function returns either an existing array instance or a newly constructed instance (as required).

> Note:  
> Some objects cannot be constructed until some/all fields are already deserialised. In this case, a wrapper `Cargo` object can be used that collects incoming deserialised fields and then constructs a new instance when the *validate* function is called (or fails the deserialisation process if the collected values are invalid).

When the `Transport` is deserialising and has obtained a new `Cargo` instance with *getCargo*, the first step (before any data is deserialised into the cargo) is to ensure the instance starts in the schema default state by calling *setDefault*, e.g.:

```cpp
void Object::setDefault() {
	docType.clear();
	tag.clear();
	values.clear();
	objects.clear();
}
```

In this case, the default state is for all the fields to be empty. However, some schemas prescribe specific values or states, which can be implemented here. If no other value is deserialised for a field, it will retain this default. Even a wrapper class (that buffers incoming fields before constructing a new object) can use this function to set the buffer values to a default.

Once `Cargo` has been fully deserialised, it is asked to validate the content:

```cpp
bool Object::validate() {
	return !docType.empty();
}
```

The only invariant for this `Cargo` is that *docType* (an identifier for the object class) must not be empty. If *validate* returns *false*, the `Transport` will throw an exception.

> Note:  
> This function can be used for much more than checking content, e.g.:
> - Some objects cannot be constructed until all the data member data is deserialised. A wrapper could cache the required fields and construct a new instance with it in this function.
> - Structural database changes can mean that the members of a class are changed/moved/erased or even that the entire object has moved elsewhere. Suitable wrappers can move or cache legacy data elsewhere (potentially in preparation for a top-level migration process to manage when the deserialisation is complete).
> - A 3rd-party schema might use a completely different structure to internal classes, e.g. a hierarchy serialised as a flat list (or vice-versa). Again, suitable wrappers manage the transition of this data within the *validate* function (as with migration of legacy data).

Following validation, there is one final step for deserialising an object that is an item instance belonging to an array/dictionary - insertion into the parent container:

```cpp
bool Object::insert(Cargo::Unique&& cargo, const Inventory::Item& item) {
	if (item.second.ownerType != &typeid(Object))
		return true;
	switch (item.second.index) {
		case FieldIndex::val:
				//Inserting a new value
			if (auto value = dynamic_cast<XMLValueSetting*>(cargo.get()); value != nullptr)
				values.emplace_back(*value);
			break;
		case FieldIndex::obj:
				//Inserting a new object
			if (auto package = dynamic_cast<Object*>(cargo.get()); package != nullptr)
				objects.emplace_back(std::move(*package));
			break;
		default:
			break;
	}
	return true;
}
```

Note that this function will only be called if the new instance will not cause the receiving object to exceed the maximum bound specified in the inventory entry (schema) - otherwise the `Transport` will throw an exception. An exception will also be thrown if the *insert* function returns false (signalling that the data is bad).

Note also that the incoming `Cargo` will be the object created by the *getCargo* function (for the same `Identity`).

For this example, the `Cargo` has two arrays – one for nested values and the other for nested objects – so it only inspects the identity of the incoming `Cargo` and inserts it into the appropriate array.

> Note:  
> As noted for the preceding *validate* function, the *insert* function can also be used for multiple purpose, e.g. handling legacy data or implementing stuctural changes. Essentially, this function is handing over an object, but does not prescribe any specific action (e.g. the incoming `Cargo` can be discarded).

### Deserialising Polymorphic Objects  <a name="despoly"></a>

Deserialising polymorphic objects can be challenging, particularly if the 'type' identifier is embedded in the serialised data. For example, it is impossible to know what schema to apply (or `Inventory` to specify) until this type is established.

A wrapper object could address this by gathering all the data (including the type) and finally constructing an object in the *validate* function. But this requires detailed knowledge of a potentially wide (and expanding) range of different classes. There is a strategy to mitigate aainst this problem.

- Identify essential deserialisation fields, e.g.:
	- A type identifier
	- Optionally, other parameters which are essential for constructing any type instance, e.g. guids or similar identifiers
- Mark the `Entry` for these fields as *attributes* in the `Inventory`
- Override 2 additional functions in the `Cargo` class
	1. `bool isAttributeFirst() const`  
When this function returns *true*, the deserialisation process will only recognise entries marked as attributes. All other data will be skipped over until either all the attributes have been found or the end of the scope has been reached.    
	2. `bool finaliseAttributes()`   
This function will be called when *isAttributeFirst()* returns true and the attributes have been read. This provides an opportunity for the correct object to be constructed (based on the attributes).
> Note:  
> For XML, this happens naturally (attributes are always first). The JSON standard does not specify any particular order for object members - in some cases the `Transport` may have to parse the same scope twice).   
- Allow *fillInventory* to provide 2 different responses: one before the attributes have been read and another after, allowing the `Inventory` to be expanded based on the object created by *finaliseAttributes*.

This approach allows for the creation of more focussed wrappers to deal with details of a specific class rather than building one monolithic wrapper.
 

A simple example can be seen in the `SerialiseArrayTester` test, which serialises an array of objects subclassed from a common base. Furthermore, these objects can only be (re)constructed with the original guid (required in the base class). Each subclass has a different data set (small in the example, but can scale to much larger data sets).
