/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_STRING
#define ACTIVE_UTILITY_STRING

#include "Active/Utility/DataFormat.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/Memory.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace active::utility {
	
	class BufferIn;
	class BufferOut;
	
	/// A Unicode-aware string class
	/*!
		The String class is a wrapper for std::string and leans heavily on existing functionality it provides, extending it with
		awareness of Unicode encodings for character positioning. This also provides easy access to the underlying std::string
		for easy interoperability with any code working with that type.
		
		String content is internally encoded/validated as UTF-8, but it can be encode/decode UTF8, UTF16, UTF32, ASCII and ISO8859.
		Character positions are calculated to allow indexing, but the time to find a position averages O(n)
		It is recommended to use classes like BufferIn to analyse by-character content on large blocks of text efficiently
	 
		This class does not use a 'special' value to denote non-existent or unspecified positions, e.g. like string::npos
		Rather, an optional is used for these cases, e.g. if searching for a dot outside the first 5 characters of some text,
		using std::string could look like this:
	
			if (auto pos = text.find("."); (pos != nos) && (pos > 4))
		
		With this string class, the optional response simplifies the syntax:
	 
			if (auto pos = text.find("."); pos > 4)
		
		The String class also provides a range of static functions for validating or converting blocks of text for all supported encoding.
		Conversion operators and constructors provide interoperability with a range of common types, e.g. std::string, std::u32string etc
	 
		In line with current C++ standards, any ranges passed by a pointer and length must be valid, e.g. the atart address must not be nullptr,
		or behaviour will be undefined
	*/
	class String {
	public:

		using enum TextEncoding;

		//MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<String>;
			///Shared pointer
		using Shared = std::shared_ptr<String>;
			///Optional
		using Option = std::optional<String>;
			///Class size type
		using size_type = std::string::size_type;
			///Optional size type (used to indicate an unspecified or non-existant position)
		using sizeOption = std::optional<size_type>;
			///Unary predicate for filtering strings
		using Filter = std::function<bool(char32_t)>;
			///Unary functions for processing string characters
		using Function = std::function<std::optional<char32_t>(char32_t)>;
		
		//MARK: - Constants
		
			///Constant to indicate an unspecified or non-existant position in std::string - use std::nullopt in this class
		static constexpr size_type npos = std::string::npos;
			///The line terminating char(s) for the current platform
		static const String lineTerminator;
			///All white space characters
		static const String allWhiteSpace;
			///All line ending characters
		static const String allLineEnding;
			///All digit characters (0-9)
		static const String allDigit;
			///All numeric characters (integer and signed)
		static const String allNumeric;
			///All floating point characters (floating and integer)
		static const String allFloat;
		
		//MARK: - Constructors

		/*!
			Default constructor
		*/
		String() {}
		/*!
			Constructor from an input buffer
			@param source The source input buffer
		*/
		String(const BufferIn&& source);
		/*!
			Constructor from an input char array
			@param source The character array to be copied
			@param howMany The number of bytes to copy (nullopt for full length)
			@param format The source text data format
		*/
		String(const char* source, sizeOption howMany = std::nullopt, DataFormat format = DataFormat{});
#ifndef __CLR_VER
		/*!
			Constructor from an input UTF8 char array
			@param source The character array to be copied
			@param howMany The number of bytes to copy (nullopt for full length)
		*/
		String(const char8_t* source, sizeOption howMany = std::nullopt) : String{reinterpret_cast<const char*>(source), howMany, DataFormat{}} {}
#endif
		/*!
			Constructor from an input UTF16 char array
			@param source The character array to be copied
			@param howMany The number of 16-bit chars to copy (nullopt for null-terminated)
		*/
		String(const char16_t* source, sizeOption howMany = std::nullopt, bool isBigEndian = Memory::defaultEndian) : String{reinterpret_cast<const char*>(source),
				howMany ? String::sizeOption(*howMany * sizeof(char16_t)) : std::nullopt, DataFormat{UTF16, isBigEndian}} {}
		/*!
			Constructor from an input UTF32 char array
			@param source The character array to be copied
			@param howMany The number of 32-bit chars to copy (nullopt for null-terminated)
		*/
		String(const char32_t* source, sizeOption howMany = std::nullopt, bool isBigEndian = Memory::defaultEndian) : String{reinterpret_cast<const char*>(source),
				howMany ? String::sizeOption(*howMany * sizeof(char32_t)) : std::nullopt, DataFormat{UTF32, isBigEndian}} {}
		/*!
			Constructor from an input string
			@param source The string to be copied
			@param howMany The number of chars to copy (nullopt for full length)
			@param format The source text data format
		*/
		String(const std::string& source, sizeOption howMany = std::nullopt, DataFormat format = DataFormat{}) :
				String{source.data(), howMany, format} {}
#ifndef __CLR_VER
		/*!
			Constructor from an input string
			@param source The UTF-8 string to be copied
			@param howMany The number of chars to copy (nullopt for full length)
		*/
		String(const std::u8string& source, sizeOption howMany = std::nullopt) : String(source.data(), howMany) {}
#endif
		/*!
			Constructor from an input string
			@param source The UTF-16 string to be copied
			@param howMany The number of chars to copy (nullopt for full length)
		*/
		String(const std::u16string& source, sizeOption howMany = std::nullopt);
		/*!
			Constructor from an input string
			@param source The UTF-32 string to be copied
			@param howMany The number of chars to copy (nullopt for full length)
		*/
		String(const std::u32string& source, sizeOption howMany = std::nullopt);
		/*!
			Constructor with optional text fill expression and number of repeats
			@param newSize The required number of expression repeats
			@param fillText The expression to fill the string
		*/
		String(size_type newSize, const String& fillText);
		/*!
			Constructor from a char
			@param val A char
		*/
		explicit String(char val) : m_string{val}	{}
		/*!
			Constructor from 16-bit integer
			@param val A 16-bit integer
		*/
		explicit String(int16_t val) { m_string = std::to_string(val); }
		/*!
			Constructor from unsigned 16-bit integer
			@param val An unsigned 16-bit integer
		*/
		explicit String(uint16_t val) { m_string = std::to_string(val); }
		/*!
			Constructor from 32-bit integer
			@param val A 32-bit integer
		*/
		explicit String(int32_t val) { m_string = std::to_string(val); }
		/*!
			Constructor from unsigned 32-bit integer
			@param val An unsigned 32-bit integer
		*/
		explicit String(uint32_t val) { m_string = std::to_string(val); }
		/*!
			Constructor from 64-bit integer
			@param val A 64-bit integer
		*/
		explicit String(int64_t val) { m_string = std::to_string(val); }
		/*!
			Constructor from unsigned 64-bit integer
			@param val An unsigned 64-bit integer
		*/
		explicit String(uint64_t val) { m_string = std::to_string(val); }
#if !defined(WINDOWS) && !defined(__linux__)
		/*!
			Constructor from size_t integer
			@param val A size_t integer
		*/
		explicit String(size_t val) { m_string = std::to_string(static_cast<uint64_t>(val)); }
#endif //WINDOWS
		/*!
			Constructor from double
			@param val A double value
			@param prec The required precision
			@param padZero True to pad the number to the specified precision with zeros
		*/
		explicit String(double val, double prec = math::eps, bool padZero = false);
		/*!
			Copy constructor
			@param source The string to copy
		*/
		String(const String& source);
		/*!
			Move constructor
			@param source The object to move
		*/
		String(String&& source) noexcept;
		/*!
			Destructor
		*/
		~String() {}
		
		//MARK: - Static functions

		/*!
			Get the number of bytes in the specified text (counting only valid UTF8 characters)
			@param text The source text
			@param howMany The number of bytes in the text
			@param charCount The maximum number of (encoded) chars to seek
			@param format The text data format
			@return The number of bytes in the text containing valid UTF8 characters
		*/
		static size_type getValidByteCount(const char* text, sizeOption howMany = std::nullopt, sizeOption charCount = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Get the width of a specified character in bytes
			@param text The source text
			@param howMany The number of bytes in the array
			@param format The text data format
			@return The character width in bytes (nullopt for bad encoding)
		*/
		static sizeOption getCharacterByteCount(const char* text, sizeOption howMany = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Get the number of valid characters found at a specified address
			@param text The source text
			@param format The text data format
			@param howMany The number of bytes in the array (nullopt = null-terminated)
			@return The nummber of characters found (nullopt if bad encoding found)
		*/
		static String::sizeOption getCharacterCount(const char* text, sizeOption howMany = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Get a unicode (UTF-32) character from a specified source
			@param text The source text
			@param howMany The number of bytes in the text
			@param format The source data format
			@return The unicode char paired with the number of bytes consumed from the source (0 = no valid char found)
		*/
		static std::pair<char32_t, size_type> getUnicodeChar(const char* text, sizeOption howMany = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Get a UTF-32 string from a UTF-8 source
			@param text The source UTF-8 text (advances to the byte beyond the last counted character)
			@param howMany The number of bytes in the text (nullopt = null-terminated)
			@param isCountRequired True if the specified number of bytes must be valid (ignored if howMany = nullopt)
			@return The unicode code point for the specified chars (nullopt on failure)
		*/
		static std::optional<std::u32string> toUnicode(const char*& text, String::sizeOption howMany = std::nullopt, bool isCountRequired = false);
		/*!
			Get a UTF-8 string from a UTF-32 source
			@param text The source text
			@param isBigEndian True if byte ordering is big-endian
			@param howMany The number of 32-bit code points in the text (nullopt = null-terminated)
			@param isCountRequired True if the specified number of code points must be valid (ignored if howMany = nullopt)
			@return The UTF-8 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
		*/
		static std::optional<String> fromUnicode(const char32_t*& text, bool isBigEndian, String::sizeOption howMany = std::nullopt, bool isCountRequired = false);
		/*!
			Get a UTF-32 string from a UTF-16 (16-bit) source
			@param text The source text
			@param isBigEndian True if byte ordering is big-endian
			@param howMany The number of words (16-bit values) in the text (nullopt = null-terminated)
			@param isCountRequired True if the specified number of words must be valid (ignored if howMany = nullopt)
			@return The UTF-32 string read from the UTF-16 source (nullopt on error, including failure to meet isCountRequired condition)
		*/
		static std::optional<std::u32string> fromUTF16(const char16_t*& text, bool isBigEndian, String::sizeOption howMany = std::nullopt, bool isCountRequired = false);
		/*!
			Get a UTF-16 string from a UTF-32 source
			@param text The source text
			@param howMany The number of code points in the text (nullopt = null-terminated)
			@param isCountRequired True if the specified number of code points must be valid (ignored if howMany = nullopt)
			@return The UTF-16 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
		*/
		static std::optional<std::u16string> toUTF16(const char32_t*& text, String::sizeOption howMany = std::nullopt, bool isCountRequired = false);
		
		//MARK: - Conversion operators
		
			///Conversion to std::string
		operator std::string() const { return m_string; }
#ifndef __CLR_VER
			///Conversion to std::u8string
		operator std::u8string() const;
#endif
			///Conversion to std::u16string
		operator std::u16string() const;
			///Conversion to std::u32string
		operator std::u32string() const;
			///Conversion to 16-bit integer
		explicit operator int16_t() const { return toInt16().value_or(0); }
			///Conversion to 32-bit integer
		explicit operator int32_t() const { return toInt32().value_or(0); }
			///Conversion to 32-bit unsigned integer
		explicit operator uint32_t() const { return toUInt32().value_or(0); }
			///Conversion to 64-bit integer
		explicit operator int64_t() const { return toInt64().value_or(0); }
			///Conversion to 64-bit unsigned integer
		explicit operator uint64_t() const { return toUInt64().value_or(0); }
			///Conversion to float
		explicit operator float() const { return toFloat().value_or(0.0); }
			///Conversion to double (0 if conversion impossible)
		explicit operator double() const { return toDouble().value_or(0.0); }

		//MARK: - Operators
		
		/*!
			Subscript operator
			@param index The required character position (unchecked - out of bounds behaviour undefined)
			@return The character at the specified index
		*/
		char32_t operator[](size_type index) const;
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		String& operator= (const String& source);
		/*!
			Move assignment operator
			@param source The object to move
			@return A reference to this
		*/
		String& operator= (String&& source) noexcept;
		/*!
			Assignment operator
			@param source A pointer to a char array
			@return A reference to this
		*/
		String& operator= (const char* source);
		/*!
			Three-way comparison operator
			@param ref The string to compare this to
			@return The relationship between this and ref (less, equal, greater)
		*/
		std::strong_ordering operator<=> (const String& ref) const { return compare(ref); }
		/*!
			Equality operator
			@param ref The string to compare this to
			@return True if the strings are identical
		*/
		bool operator== (const String& ref) const { return m_string == ref.m_string; }
		/*!
			Inequality operator
			@param ref The string to compare this to
			@return True if the strings differ
		*/
		bool operator!= (const String& ref) const { return !(*this == ref); }
		/*!
			Addition with assignment operator
			@param source The string to append
			@return A reference to this
		*/
		String& operator+=(const String& source) { return append(source); }
		/*!
			Addition with assignment operator
			@param source The char to append
			@return A reference to this
		*/
		String& operator+=(char source) { return append(source); }

		//MARK: - Functions (const)

		/*!
			Return the number of bytes this string can contain
			@return The number of bytes this string can contain
		*/
		size_type capacity() const;
		/*!
			Get the number of characters in the string
			@return The number of characters in the string
		*/
		size_type size() const;
		/*!
			Get the string length (alias for size)
			@return The number of characters in the string (size)
		*/
		size_type length() const { return size(); }
		/*!
			Return the number of bytes consumed by this string
			@param howMany The number of chars to count (nullopt = for full length)
			@return The number of
		 bytes consumed by the string (NB: String are null terminated, but this is not included in the data length)
		*/
		size_type dataSize(sizeOption howMany = std::nullopt) const;
		/*!
			Determine if the string is empty
			@return True is the string is empty
		*/
		bool empty() const { return m_string.empty(); }
		/*!
			Return a pointer to a C representation of the string data
			@return A pointer to the string data
		*/
		const char* data() const { return m_string.data(); }
		/*!
			Return the string content as a std::string
			@return A std::string representation
		*/
		std::string string() const { return m_string; }
		/*!
			Subscript function
			@param index The required character position (checked - exception thrown if out of bounds)
			@return The character at the specified index
		*/
		char32_t at(size_type index) const;
		/*!
			Apply a function to specified characters in the string
			@param func The character function (the returned value is ignored)
		*/
		void forEach(const Function& func) const;
		/*!
			Get a specified substring of this string
			@param startPos The position of the first character
			@param howMany The number of characters to get (nullopt for full length)
			@return The requested string segment
		*/
		String substr(size_type startPos = 0, sizeOption howMany = std::nullopt) const;
		/*!
			Create an uppercase version of the string
			@return An uppercase version of the string
		*/
		String uppercase() const;
		/*!
			Create a lowercase version of the string
			@return A lowercase version of the string
		*/
		String lowercase() const;
			///Conversion to optional int16_t (nullopt if conversion impossible)
		std::optional<int16_t> toInt16() const { try { return std::stoi(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional int32_t (nullopt if conversion impossible)
		std::optional<int32_t> toInt32() const { try { return std::stol(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional uint32_t (nullopt if conversion impossible)
		std::optional<uint32_t> toUInt32() const { try { return std::stoul(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional int64_t (nullopt if conversion impossible)
		std::optional<int64_t> toInt64() const { try { return std::stoll(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional uint64_t (nullopt if conversion impossible)
		std::optional<uint64_t> toUInt64() const { try { return std::stoull(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional float (nullopt if conversion impossible)
		std::optional<float> toFloat() const { try { return std::stof(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional double (nullopt if conversion impossible)
		std::optional<double> toDouble() const { try { return std::stod(m_string); } catch(...) { return std::nullopt; } }
		/*!
			Write this string to a buffer
			@param buffer The destination buffer
			@param format The required output format
			@param isNullAdded True to add a terminating null
			@param howMany The number of characters to write (nullopt for all)
			@param maxLen The maximum number of chars to write
			@return A reference to the destination
		*/
		const BufferOut& writeTo(const BufferOut& buffer, DataFormat format = UTF8, bool isNullAdded = true,
								 sizeOption howMany = std::nullopt, sizeOption maxLen = std::nullopt) const;
		/*!
			Write this string to a buffer (as internally encoded)
			@param buffer The destination buffer
			@param isNullAdded True to add a terminating null
			@param howMany The number of characters to write (nullopt for all)
			@param maxLen The maximum number of bytes the destination can hold (including terminating null - nullopt for full length)
			@return A reference to the destination
		*/
		const BufferOut& writeUTF8(const BufferOut& buffer, bool isNullAdded = true, sizeOption howMany = std::nullopt, sizeOption maxLen = std::nullopt) const;
		/*!
			Write this string as UTF-16 to a buffer
			@param buffer The destination buffer
			@param isNullAdded True to add a terminating null
			@param isBigEndian True if byte ordering is big-endian
			@param howMany The number of characters to write (nullopt for all)
			@param maxLen The maximum number of chars the destination can hold (including terminating null - nullopt for full length)
			@return A reference to the destination
		*/
		const BufferOut& writeUTF16(const BufferOut& buffer, bool isNullAdded = true, bool isBigEndian = DataFormat::defaultEndian,
									sizeOption howMany = std::nullopt, sizeOption maxLen = std::nullopt) const;
		/*!
			Write this string as UTF-32 to a buffer
			@param buffer The destination buffer
			@param isNullAdded True to add a terminating null
			@param isBigEndian True if byte ordering is big-endian
			@param howMany The number of characters to write (nullopt for all)
			@param maxLen The maximum number of chars the destination can hold (including terminating null - nullopt for full length)
			@return A reference to the destination
		*/
		const BufferOut& writeUTF32(const BufferOut& buffer, bool isNullAdded = true, bool isBigEndian = DataFormat::defaultEndian,
									sizeOption howMany = std::nullopt, sizeOption maxLen = std::nullopt) const;
		/*!
			Determine if the string is entirely alphanumeric
			@param startPos The position to checking from
			@param howMany The number of characters to check (nullopt = to end)
			@return True is the string is alphanumeric
		*/
		bool isAlphaNumeric(size_type startPos = 0, sizeOption howMany = std::nullopt) const;
		/*!
			Determine if the string is entirely letters
			@param startPos The position to checking from
			@param howMany The number of characters to check (nullopt = to end)
			@return True is the string is letters
		*/
		bool isAlpha(size_type startPos = 0, sizeOption howMany = std::nullopt) const;
		/*!
			Determine if the string is entirely numeric
			@param startPos The position to checking from
			@param howMany The number of characters to check (nullopt = to end)
			@return True is the string is numbers
		*/
		bool isNumeric(size_type startPos = 0, sizeOption howMany = std::nullopt) const;
		/*!
			Three-way comparison to a reference string
			@param ref The string to compare this to
			@return The relationship between this and ref (less, equal, greater)
		*/
		std::strong_ordering compare(const String& ref) const;
		/*!
			Find the specified string within this
			@param toFind The string to find
			@param startPos The character to start searching from
			@return The index where a match is found (nullopt = not found)
		*/
		sizeOption find(const String& toFind, size_type startPos = 0) const;
		/*!
			Find the specified string within this using a filter
			@param filter The string filter
			@return The index where a match is found (nullopt = not found)
		*/
		sizeOption findIf(const Filter& filter) const;
		/*!
			Determine if the string contains a substring
			@param toFind The substring to find
			@return True if the substring is found
		*/
		bool contains(const String& toFind) const { return find(toFind).has_value(); }
		/*!
			Determine if this string starts with a substring
			@param toFind The substring to find
			@return True if this string starts with the substring
		*/
		bool startsWith(const String& toFind) const { return m_string.starts_with(toFind.m_string); }
		/*!
			Determine if this string ends with a substring
			@param toFind The substring to find
			@return True if this string ends with the substring
		*/
		bool endsWith(const String& toFind) const { return m_string.ends_with(toFind.m_string); }
		/*!
			Find the first character which is in a specified string
			@param toFind A string of characters to find
			@param startPos The character to begin searching from
			@return The index of the first matching character (nullopt = not found)
		*/
		sizeOption findFirstOf(const String& toFind, size_type startPos = 0) const;
		/*!
			Find the first character not in a specified string
			@param toFind A string of characters to not find
			@param startPos The character to begin searching from
			@return The index of the first non-matching character (nullopt = not found)
		*/
		sizeOption findFirstNotOf(const String& toFind, size_type startPos = 0) const;
		/*!
			Find the last character in this string which is in the specified string
			@param toFind A string of characters to find
			@param lastPos The position of the last character to be compared
			@return The index of the last matching character (nullopt = not found)
		*/
		sizeOption findLastOf(const String& toFind, sizeOption lastPos = std::nullopt) const;
		/*!
			Find the last character in this string which is not in the specified string
			@param toFind A string of characters not to find
			@param lastPos The position of the last character to be compared
			@return The index of the last non-matching character (nullopt = not found)
		*/
		sizeOption findLastNotOf(const String& toFind, sizeOption lastPos = std::nullopt) const;
		/*!
			Find the specified string searching backwards
			@param toFind The string to find
			@param lastPos The position of the last character to be compared
			@return The index of a matching string (nullopt = not found)
		*/
		sizeOption rfind(const String& toFind, sizeOption lastPos = std::nullopt) const;

		//MARK: - Functions (mutating)

		/*!
			Reserve the specified number of bytes for the string to grow into
			@param newSize The number of bytes to reserve
		*/
		void reserve(size_type newSize);
		/*!
			Resize the string and (when grown) pad with the specified character
			@param newSize The required number of characters
			@param padding A padding character (when the sring grows - only the first character is used)
		*/
		void resize(size_type newSize, const String& padding = "");
		/*!
			Clear the string contents
		*/
		void clear() { m_string.clear(); }
		/*!
			Apply a function to specified characters in the string
			@param func The character function (the returned value replaces the input character)
		*/
		void forEach(const Function& func);
		/*!
			Assign a specified string to this
			@param source The string to assign
			@return A reference to this
		*/
		String& assign(const String& source);
		/*!
			Assign a specified string to this
			@param source The character array to be copied
			@param byteCount The maximum number of bytes in the array (nullopt = null-terminated)
			@param charCount The maximum number of (encoded) characters to read (nullopt = as byteCount limit)
			@param format The source text data format
			@return The number of bytes assigned from the source
		*/
		size_type assign(const char* source, sizeOption byteCount = std::nullopt, sizeOption charCount = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Assign a double value to the string (used in special cases where fast conversion is a priority
			@param value The string to assign
			@param decPlaces The number of decimal places
			@return True if the assignment was successful
		*/
		bool assign(double value, uint8_t decPlaces = 5);
		/*!
			Append the specified string to this
			@param source The string to append
			@return A reference to this
		*/
		String& append(const String& source);
		/*!
			Append the specified char to this (NB: don't use this casually - encoding must be assumed and converted accordingly)
			@param source The char to append
			@return A reference to this
		*/
		String& append(char source) {
			if (auto uniChar = getUnicodeChar(&source, 1, ISO8859_1); uniChar.second > 0)
				append(uniChar.first);
			return *this;
		}
		/*!
			Append the specified unicode char to this
			@param source The char to append (assumed to be UTF-32)
			@return A reference to this
		*/
		String& append(char32_t source) {
			const char32_t* sourcePtr = &source;
			if (auto sourceStr = fromUnicode(sourcePtr, Memory::defaultEndian, 1); sourceStr)
				m_string += *sourceStr;
			return *this;
		}
		/*!
			Insert a string into this
			@param pos The insertion point
			@param source The string to insert
			@param start The start point in the source string
			@param howMany The number of chars to insert (nullopt inserts all)
			@return A reference to this
		*/
		String& insert(size_type pos, const String& source, size_type start = 0, sizeOption howMany = std::nullopt);
		/*!
			Replace a specified string segment with another string
			@param pos The position to begin replacing (nullopt = append to end)
			@param num The number of chars to replace (nullopt = to the string end)
			@param source The replacement string
			@param start The start point in the replacement string
			@param howMany The number of chars to extract from the replacement string (nullopt inserts all)
			@return A reference to this
		*/
		String& replace(sizeOption pos, sizeOption num, const String& source, size_type start = 0, sizeOption howMany = std::nullopt);
		/*!
			Replace all instances of a specified expression
			@param toFind The expression to be replaced
			@param replacement The replacement expression
			@return A reference to this
		*/
		String& replaceAll(const String& toFind, const String& replacement = String());
		/*!
			Replace all chars passing a specified filter
			@param filter The filter for chars to be replaced
			@param replacement The replacement expression
			@return A reference to this
		*/
		String& replaceIf(const Filter& filter, const String& replacement = String());
		/*!
			Replace any instances of specified characters
			@param charsToFind The characters to be replaced (NB: each character is treated independently - the order does not matter)
			@param replacement The replacement expression
			@return A reference to this
		*/
		String& replaceAnyOf(const String& charsToFind, const String& replacement = String());
		/*!
			Erase a specified range of characters from a string
			@param pos The position to erasing from
			@param howMany The number of characters to erase (nullopt to erase to end)
			@return A reference to this
		*/
		String& erase(size_type pos = 0, sizeOption howMany = std::nullopt);
		/*!
			Remove the last character from the string
		*/
		void popBack();
		/*!
			Pad the string with a repeated character to reach a specified length (so the existing content is flush with the right)
			@param length The required string length
			@param repeat The character to repeatedly insert util the length is met
			@return A reference to this
		*/
		String& padRight(size_type length, const String& repeat = " ");
		/*!
			Pad the string with a repeated character to reach a specified length (so the existing content is flush with the left)
			@param length The required string length
			@param repeat The character to repeatedly append util the length is met
			@return A reference to this
		*/
		String& padLeft(size_type length, const String& repeat = " ");
		
	private:
			///The string content - NB: The STL representation works for many UTF-8 functions, and this class manages the remainder
		std::string m_string;
	};
	
		
	/*!
		Addition operator
		@param str1 The first string
		@param str2 The second string
		@return The concatonated string
	*/
	inline String operator+(const String& str1, const String& str2) {
		return String{str1}.append(str2);
	}
	
	
	/*!
		Determine if a specified char is white-space
		@param uniChar The char to test
		@return True if a specified char is white-space
	*/
	inline bool isWhiteSpace(char32_t uniChar) {
		switch (uniChar) {
		case U' ': case U'\t': case U'\r': case U'\n':
			return true;
		default:
			return false;
		}
	}
	
	
	/*!
		Determine if a specified char is a line ending
		@param uniChar The char to test
		@return True if a specified char is a line ending
	*/
	inline bool isLineEnding(char32_t uniChar) {
		return ((uniChar == U'\r') || (uniChar == U'\n'));
	}
	
	
	/*!
		Determine if a specified char is numeric (int)
		@param uniChar The char to test
		@return True if a specified char is numeric
	*/
	inline bool isDigit(char32_t uniChar) {
		return ((uniChar <= U'9') && (uniChar >= U'0'));
	}
	
	
	/*!
		Determine if a specified char is numeric (int)
		@param uniChar The char to test
		@return True if a specified char is numeric
	*/
	inline bool isNumeric(char32_t uniChar) {
		return (isDigit(uniChar) || (uniChar == U'-'));
	}
	
	
	/*!
		Determine if a specified char is numeric (int)
		@param uniChar The char to test
		@return True if a specified char is numeric
	*/
	inline bool isFloat(char32_t uniChar) {
		return (isNumeric(uniChar) || (uniChar == U'.'));
	}

}  // namespace active::utility

	///Hashing for String class, e.g. to use as a key in unordered_map
template <>
struct std::hash<active::utility::String> {
	std::size_t operator()(const active::utility::String& k) const {
		return hash<std::string>()(k);	//Just use the hashing provided by std::string
	}
};

#endif	//ACTIVE_UTILITY_STRING
