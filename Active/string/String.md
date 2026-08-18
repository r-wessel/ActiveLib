
# string

## Contents
1. [Summary](#summary)
2. [Why?](#why)
3. [Design considerations](#design)
4. [Interoperability](#interop)
5. [Size/position](#size)
6. [Subscript operator](#subscript)
7. [Safe positioning](#safety)

## Summary <a name="summary"></a>

Unicode-aware string class extending std::string:

- Same interface as std::string
- Interoperability with UTF16, UTF32, ASCII and ISO8859 is supported
- String content is always valid UTF8 by design, i.e. arbitrary data or other encodings cannot be injected
- Size/position values always reference character positions (not bytes) and have overflow checks
- Ranged access and iterators are supported
- Random access is supported, e.g. through the subscript operator

## Why? <a name="why"></a>

 We already have `std::string` – why do we need anything else?

- If you need to interact with string *content*, then you also need to deal with string *encoding*. That broadly means Unicode, but UTF-8 in particular. `std::string` is not designed for this purpose.
- If your requirements for `std::string` do not extend beyond holding a null-terminated series of bytes, i.e. text encoding and the size/position of characters are irrelevant, then `std::string` provides everything you need.
- The intention is not to replace 'std::string', but to extend it with Unicode awareness. `active::string`  has `std::string` at its core and leans heavily on its existing functionality.
- While many `std::string` functions work correctly on UTF8 encoded text, there others that either can't be used at all or can only work with great care. `active::string` is designed to make all these functions safe and easy
- The interface is intended to mirror `std::string` as closely as possible, ideally to be used as a drop-in replacement

## Design considerations <a name="design"></a>

UTF-8 has become the standard for most use cases. Therefore, a key design goal is to maximise performance with UTF-8. Interoperability with other encodings is provided (see next section) but the internal encoding is always UTF-8 on the assumption that this encoding will most commonly be transported.

The penalty for this decision is that an encoded UTF-8 character does not have a fixed width (in bytes), raising several important design considerations:
1. There cannot be a fixed integral representation of any UTF-8 character, e.g. '∞' is 3 bytes (E2 88 9E). Two strategies are employed to echo the semantics of `std::basic_string`:
    - Functions that receive or return a character are implemented with dynamic conversion from `char32_t` (UTF-32), e.g. one method for appending a single character to a string is `basic_string& append(char32_t source)` to support `someString.append(U'∞');`
    - A character reference is implemented with `active::basic_string::char_t`, e.g. an iterator can be dereferenced to mutate a character `*iter = U'∞'`
2. It is not possible for a string iterator to provide a literal reference to a character in a string. Rather, it returns a wrapper that dynamically converts between an integral and UTF-8 representation [NB: there is an option to return a reference, but this incurs memory overheads - see below]
3. Storage access behaves like a `list` rather than a `vector`, i.e. random access into a character is O(n) rather than O(1). This is not seen as a significant problem given that most string algorithms can be efficiently solved with iteration.

Note that std::string operations involving `char` are also supported, but with conversion as required, e.g.:
```Cpp
string text;
	//Append char
text += 'a'; //Appends 0x61
text += '©'; //Appends 0xC2A9
	//Append char32_t
text += U'∞'; //Appends 0xE2889E

```

## Interoperability <a name="interop"></a>
Full interoperability with all STL string types is available for assignment, comparison, concatenation etc, for example:
```Cpp
	//Initialise a variety of std string values
std::string stdStr = "Café-🂱";
std::u8string std8 = u8"Café-🂱";
std::u16string std16= u"Café-🂱";
std::u32string std32= U"Café-🂱";
	//Assign to active::string
active::string activeStr = stdStr;
active::string active8 = std8;
active::string active16 = std16;
active::string active32 = std32;
	//Compare contents to test validity
std::cout << ((activeStr == active8) && (activeStr == active16) && (activeStr == active32) ? "good" : "fail") << "\n"; //-> "good"
std::cout << ((activeStr == stdStr) && (activeStr == std8) && (activeStr == std16) && (activeStr == std32) ? "good" : "fail") << "\n"; //-> "good"
```
This works in both directions, so `active::string` can be readily converted back to the same std string types.

## Size/position <a name="size"></a>

The size of a string is always the number of *characters*, whereas `std::basic_string::size_type` is essentially an index into a contiguous sequence of fixed-width values (which can split unicode characters). Consider the following, where std string types subtly fail:
```Cpp
	//Start with a 6-character string (UTF8 & UTF16 encoding)
const std::u8string wordTest = u8"Café-🂱";
const std::u16string wordTest16= u"Café-🂱";
const std::string::size_type charsToErase = 2;
	//std::u8string test
std::u8string stdin8{wordTest};
stdin8.erase(stdin8.size() - charsToErase, charsToErase); //-> u8"Café-\xf0\x9f" (wrong - bad encoding)
	//std::u16string test
std::u16string stdin16{wordTest16};
stdin16.erase(stdin16.size() - charsToErase, charsToErase); //-> u"Café-" (wrong - only 1 char erased)
	//active::string UTF8 test
active::string actin8{wordTest};
actin8.erase(actin8.size() - charsToErase, charsToErase); //-> u8"Café" (correct)
	//Reassign to u8string
stdin8 = actin8; //-> u8"Café" (correct)
	//active::string UTF16 test
active::string actin16{wordTest16};
actin16.erase(actin16.size() - charsToErase, charsToErase); //-> u8"Café" (correct)
	//Reassign to u16string
stdin16 = actin16; //-> u"Café" (correct)
```
`active::string` eliminates these problems because size/position always references valid character positions. Likewise, it is safe to access characters through the subscript operator:
```Cpp
std::u8string wordTest8 = u8"Café-🂱";
std::u16string wordTest16 = u"Café-🂱";
active::string act8 = wordTest8;
active::string act16 = wordTest16;
wordTest8[5] = U'🂡'; //-> u8"Café\xa1🂱" (wrong char, wrong position, bad encoding)
wordTest16[5] = U'🂡'; //-> u"Café-\xed\xb2\xb1" (wrong char, bad encoding)
act8[5] = U'🂡'; //-> "Café-🂡" (correct)
act16[5] = U'🂡'; //-> "Café-🂡" (correct)
```
Note that `active::string` allows read/write access to a full unicode character, unlike `std::string`, `std::u8string` or `std::u16string`.
## Ranged/iterator access

A string can be accessed via iterators, e.g. in an STL algorithm:

```Cpp
	//std algorithms using iterator - find first instance of 'é' and replace with 'e'
string testWord{u8"Café"};
if (auto found = std::find_if(testWord.begin(), testWord.end(), [](const auto& charVal) {
			return charVal == U'é';
		}); found != testWord.end())
	*found = 'e'; //testWord -> "Cafe"
```
…or in a range-based loop (increment all chars):
```Cpp
	//Range-based loop - increment all letters in text
string series{u8"Café"};
for (auto& charVal : series)
	charVal = charVal + 1; //series -> "Dbgê"
```
[NB: The above code examples do not work with std::string]

## Subscript operator <a name="subscript"></a>

While the interface for the subscript operator can exactly match std::string (when the macro `ACTIVE_STRING_CHAR_REF` is defined), the default does not return a reference:

```Cpp
char_t operator[](size_type index);
```
However, `active::string::char_t` acts like a smart pointer to allow the target character to be mutated:
```Cpp
string word{"Cafe"};
word[3] = U'é'; //-> "Café"
```
If `active::string` must provide the same interface as `std::string`, define the macro `ACTIVE_STRING_CHAR_REF`, but be aware that this incurs memory overheads (so each `string` instance provides a placeholder for a referenced value). Note also that this implementation cannot hold more than one reference to a character in the same string simultaneously.

Throws if the subscript index is out of bounds.

## Safe positioning <a name="safety"></a>

Size/position values are represented by `struct string_size`:
- Wraps `std::string::size_type` and provides all the same functionality
- `end()` is marked with the value `std::string::npos`
- Default value is end()/npos
- Throws on overflow errors, e.g.:
	- Assigning a negative value;
	- Copying to a numeric type that cannot hold the current value;
	- Attempting to increment beyond `end()`.
- Extended with behaviours of `std::optional`:
	- Function `has_value()` (returns false when value is `npos`)
	- Explicit `bool` conversion (returns `has_value()`)
	- Function `value_or(size_type)`
	- All comparisons to `npos` return false (except `npos == npos -> true`)
	- Can be assigned std::nullopt (sets to end()/npos)

