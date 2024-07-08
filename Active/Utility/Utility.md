
# Utility

## Contents
1. [Purpose](#purpose)
2. [BufferIn/Out](#buffer)
3. [Cloner](#cloner)
4. [MathFunctions](#maths)
5. [Memory](#memory)
6. [String](#string)

## Purpose <a name="purpose"></a>

Where other modules focus on one specific problem domain, e.g. Geometry, Serialisation etc, Utility is a collection of functions wth the express purpose of supporting the other modules. Therefore, all other modules have a dependency on this one to some extent. The following sections describe the major components.

## BufferIn/Out <a name="buffer"></a>

`BufferIn` and `BufferOut` are designed for specific cases, primarily reading/writing data from/to some source/destination as a serial stream. Random access is possible (but not the primary goal) and a container that both reads and writes is not currently supported. The intended purposes are:
1. Minimising memory overheads for file operations.
2. Minimising reallocations when streaming to String or Memory
3. Anonymising the source/destination for services using streams
4. Facilitating text encoding/decoding or reading/writing a BOM in the stream (as required)

Buffering is also used to optimise some string operations (e.g. eliminating the need to recalculate a starting position based on the character offset in bytes).

`JSONTransport` (in the Serialisation module) is a good example. Both sending and receiving as JSON is expressed in terms of a buffer, e.g.:
```Cpp
	///Send as JSON
void send(Cargo&& cargo, const Identity& identity, BufferOut&& destination, bool isTabbed, bool isLineFeeds, bool isNameSpaces, bool isProlog) const override;
	///Receive from JSON
void receive(Cargo&& cargo, const Identity& identity, BufferIn&& source) const override;
```
In most cases the implicit conversion to a buffer from the source/destination object is fine, e.g.:
```Cpp
String destinationString;
JSONTransport().send(someObject, Identity{}, destinationString);

Memory destinationMemory;
JSONTransport().send(someObject, Identity{}, destinationMemory);

File destinationFile;
JSONTransport().send(someObject, Identity{}, destinationFile);
```
An explicit encoding can be specified where required, e.g. if file content is known to be UTF32:
```Cpp
File sourceFile;
JSONTransport().receive(someObject, Identity{}, BufferIn{sourceFile, UTF32});
```

BufferIn can also be configured to discover the encoding and byte-order by passing std:nullopt for the `DataFormat`, e.g.:
```Cpp
File sourceFile;
JSONTransport().receive(someObject, Identity{}, BufferIn{sourceFile, std::nullopt});
```

Data sent to BufferOut isn't written to the actual destination until either the object is destroyed, the buffer is full or BufferOut::flush() is called. Don't forget to call flush() if the data is expected to be in the destination before BufferOut is destroyed.

BufferIn contains numerous methods for finding/processing content, illustrated with some examples:
1. Seek the first white-space character in the source, pooling all prior characters into a string (`firstWord`):
```Cpp
BufferIn{String{"This is a test"}}.findFirstOf(String::allWhiteSpace, &firstWord);
```
After executing, `firstWord` will contain `"This"`.

2. Seek and skip over any contiguous combination of line endings (CR/LF), discarding any intermediate chars, so reading starts at the following char:
```Cpp
String source{"This is some text\r\n\r\nNext line"};
BufferIn bufferTest{source};
bufferTest.findFirstOf(String::lineTerminator, nullptr, /*isContiguousMatch*/ true, /*isRepeatMatch*/ true, /*isFoundSkipped*/ true);
String firstWord;
bufferTest >> firstWord;
```
After executing, `firstWord` will contain `"Next"`.

3. Seek and skip over any contiguous combination of line endings (CR/LF), pooling any intermediary chars into a string (`firstLine`) including the line ending chars:

```Cpp
String source{"This is some text\r\nNext line"}, firstLine;
BufferIn{source}.findFirstOf(String::lineTerminator, &firstLine, /*isContiguousMatch*/ true, /*isRepeatMatch*/ true, /*isFoundSkipped*/ true, /*isFoundPooled*/ true);
```
After executing, `firstLine` will contain `"This is some text\r\n"`.

Refer also to StackBufferOut. This specialises BufferOut to use a fixed-size buffer allocated as a member of the object rather than on the heap (as BufferOut does). If the StackBufferOut is on the stack, so is all the allocated space. This can be used in cases where heap allocations problematic/expensive.

## Cloner <a name="cloner"></a>

`Cloner` is a simple interface to support cloning (deep copying) of polymorphic objects. Any class intended to support cloning should be derived from `Cloner`, e.g.:
```Cpp
class Foo : public Cloner {};
class Bar : public Foo {
public:
	Bar* clonePtr() const { return new Bar{*this}; }
};
auto foo = std::make_unique<Bar>();
auto foo2 = clone(*foo);	//foo2 is a deep copy of foo (type Bar)
```
The Container classes, e.g. `Vector`, rely on cloning to support deep copying of container content. The `Clonable` concept is provided to test for clonability, e.g.:
```Cpp
template<class T> requires utility::Clonable<T>
class Vector : public std::vector<std::unique_ptr<T>> {
```

The `Mover` interface is conceptually similar to `Cloner`, with the difference that the class supports a deep cloning of an object with move rather than copying, moving the resources from the original to the clone.

## MathFunctions <a name="maths"></a>

A collection of common constants (including unit conversions) and functions. The default units are considered to be metres, but there is nothing in the library that requires this unit. Only the default precision (eps) is explicitly a metric figure (1e-5m = 0.01mm).

Most of the functions are concerned with floating point comparisons using precision. The default precision is set to be just below the highest practical precision for the AEC industry - some engineering sectors will required a much higher precision. Rather than making direct comparisons between floating point values, e.g.:
```Cpp
double someValue = getLength();
if (someValue == 0.0)
```
…use a function with explicit precision, e.g.:
```Cpp
double someValue = getLength();
if (isZero(someValue, precision))
```
This avoids meaninglessly small differences, e.g. 1e-30.

## Memory <a name="memory"></a>

An object defining (and optionally owning/allocating) a block of memory. The `Memory` class also defines some static functions for common memory operations and managing endianness. `Memory` and `BufferIn`/`BufferOut` work cooperatively to eliminate the risk of buffer overruns.

There are 2 scenarios for defining the memory block range:

1. The `Memory` object allocates a new block (resizing as required)  
The `Memory` object owns the allocation and is responsible for releasing it (but can relinquish ownership with the `release()` function). A `BufferOut` object can write freely to this object with expansion on demand to ensure no overrun. `BufferIn` will only read to the end of the allocated region.
```Cpp
Memory dataOut;
BufferOut{dataOut} << "Pi = " << pi << "\n";	//dataOut becomes "Pi = 3.14159\n"
```
2. A static range (location and size) is assigned to `Memory`
The Memory object does not own the block and will not attempt to resize or release it. BufferIn and BufferOut will flag an error at any attempt to read/write out of range.
```Cpp
int32_t num = Memory::toBigEndian(743101568);
String hex;
active::serialise::HexTransport().send(Memory{num}, hex); //hex becomes "2C4AD480"
```

## String <a name="string"></a>

The `String` class is primarily intended to simplify code that has to deal with Unicode, and particularly UTF8 encoding (recognising that it's practically the standard in many contexts). The STL string types are very capable, and this functionality is harnessed by making `String` a wrapper for `std::string` so existing functionality can be fully leveraged, extending it with awareness of Unicode for character positioning. It also provides full access to the underlying `std::string`
for easy interoperability.

String content is internally encoded/validated as UTF-8 (so most `std::string` functions work correctly) but it can be encoded/decoded as UTF8, UTF16, UTF32, ASCII and ISO8859. Character positions are calculated to allow indexing, but the time to find a position averages O(n). It is recommended to use classes like `BufferIn` to analyse by-character content on large blocks of text efficiently.

Unicode code-points are understood by `String`, so functions like `size()` returns the number of code points rather than the number of bytes consumed by the string (the number of bytes can be retrieved with `dataSize()`). Functions that rely on character positioning, e.g. `substr` or `findFirstOf`, also work correctly (based on code points), e.g.:
```Cpp
String exampleStr{u8"½Pint Solutions® Inc Copyright © 2024"};
auto pos = exampleStr.findFirstOf(u8"®©");	//pos becomes 15
exampleStr.erase(*pos, 1); //exampleStr becomes "½Pint Solutions Inc Copyright © 2024"
```
…but fail with `std::string`, e.g.:
```Cpp
std::u8string exampleStr{u8"½Pint Solutions® Inc Copyright © 2024"};
auto pos = exampleStr.find_first_of(u8"®©"); //pos becomes 0
exampleStr.erase(pos, 1); //exampleStr becomes "\xbdPint Solutions® Inc Copyright © 2024"
```
The String class is interoperable with multiple encodings (UTF8, UTF16, UTF32, ISO8859_1) and std::string types, e.g.:
```Cpp
	//Copy and compare encoded string literals
String example1{u8"ようこそ 日本 へ"};	//UTF8
String example2{u"ようこそ 日本 へ"};	//UTF16
String example3{U"ようこそ 日本 へ"};	//UTF32
std::u8string examplestd1{u8"ようこそ 日本 へ"};
std::u16string examplestd2{u"ようこそ 日本 へ"};
std::u32string examplestd3{U"ようこそ 日本 へ"};
if ((example1 == examplestd1) && (example2 == examplestd2) && (example3 == examplestd3))
	std::cout << "Success";
	//Copy and compare std::string types
String copy1 = examplestd1;	//UTF8
String copy2 = examplestd2;	//UTF16
String copy3 = examplestd3;	//UTF32
std::u8string copystd1 = copy1;
std::u16string copystd2 = copy2;
std::u32string copystd3 = copy3;
if ((example1 == copystd1) && (example2 == copystd2) && (example3 == copystd3))
	std::cout << "Success";
```
One significant difference is that a `String` cannot be mutated through the subscript operator. Where you can do this with `std::string`:
```Cpp
std::string test{"Word"};
test[3] = 'k';	//test becomes "Work"
```
…do this with `String`:
```Cpp
String test{"Word"};
test.replace(3, 1, "k");	//test becomes "Work"
```
A 'special' value isn't used to denote non-existent or unspecified positions, e.g. like `string::npos`. Rather, an optional is used for these cases, e.g. if searching for a dot outside the first 5 characters of some text, using std::string could look like this:
```Cpp
if (auto pos = text.find("."); (pos != npos) && (pos > 4))
```
With this string class, the optional response simplifies the syntax:
```Cpp
if (auto pos = text.find("."); pos > 4)
```
The `String` class also provides a range of static functions for validating or converting blocks of text for all supported encoding types. Conversion operators and constructors provide interoperability with a range of common types, e.g. `std::string`, `std::u32string` etc
