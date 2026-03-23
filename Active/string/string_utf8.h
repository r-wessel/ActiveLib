/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_STRING
#define ACTIVE_STRING

#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 13)) || (defined(__clang__) && (__clang_major__ < 15))
#include <sstream>
#else
#include <format>
#endif
#include <set>
#include <string>

#include "Active/string/text_format.h"

namespace active {
		
		///String position type - is an optional, but can automatically adapt to string::size_type usage, e.g. pos == npos
	class string_position : public std::optional<std::string::size_type> {
	public:
			//MARK: - Types
		
		using base = std::optional<std::string::size_type>;
			///Class size type
		using size_type = std::string::size_type;

			//MARK: - Constants
		
			///Constant to indicate an undefined/missing position in a string
		size_type npos = std::string::npos;

			//MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		string_position() : base{} {}
		/*!
		 Constructor
		 @param nullPos An undefined string position
		 */
		string_position(const std::nullopt_t nullPos) : base{} {}
		/*!
		 Constructor
		 @param pos A literal string position
		 */
		template<typename T> requires std::is_arithmetic_v<T>
		string_position(const T pos) { base::operator=(pos); }
		/*!
		 Copy constructor
		 @param source The object to copy
		 */
		string_position(const string_position& source) : base{} {
			if (source && (*source != npos))
				base::operator=(*source);
		}

			//MARK: - Operators
			
		/*!
		 Equality operator
		 @param ref The object to compare
		 @return True if this and ref are equal
		 */
		bool operator== (const string_position& ref) const {
			auto state = operator bool();
			if (state != ref.operator bool())
				return false;
			return !state || (**this == *ref);
		}
		template<typename T> requires std::is_arithmetic_v<T>
		bool operator== (const T ref) const { return (operator bool()) ? (**this == ref) : (ref == npos); }
		/*!
		 Equality operator
		 @param ref The object to compare
		 @return True if this and ref are equal
		 */
		bool operator!= (const string_position& ref) const { return !(*this == ref); }
		template<typename T> requires std::is_arithmetic_v<T>
		bool operator!= (const T ref) const { return !(*this == ref); }
		/*!
		 Less-than operator
		 @param ref The object to compare
		 @return True if this is less than ref
		 */
		bool operator< (const string_position& ref) const { return operator bool() && (**this < ref); }
		template<typename T> requires std::is_arithmetic_v<T>
		bool operator< (const T ref) const { return operator bool() && (**this < ref); }
		bool operator<= (const string_position& ref) const { return operator bool() && (**this <= ref); }
		template<typename T> requires std::is_arithmetic_v<T>
		bool operator<= (const T ref) const { return operator bool() && (**this <= ref); }
		/*!
		 Greater-than operator
		 @param ref The object to compare
		 @return True if this is greater than ref
		 */
		bool operator> (const string_position& ref) const { return operator bool() && (**this > ref); }
		template<typename T> requires std::is_arithmetic_v<T>
		bool operator> (const T ref) const { return operator bool() && (**this > ref); }
		bool operator>= (const string_position& ref) const { return operator bool() && (**this >= ref); }
		template<typename T> requires std::is_arithmetic_v<T>
		bool operator>= (const T ref) const { return operator bool() && (**this >= ref); }

			//MARK: - Conversion operators
		
			///Cponversion to `bool`
		explicit operator bool() const { return base::has_value() && (**this != npos); }
			///Cponversion to `size_type`
		operator std::string::size_type() const { return base::has_value() ? **this : npos; }
	};

		
	/// A Unicode-aware string class
	/*!
		The `active::basic_string` template class is a wrapper for `std::string` and leans heavily on existing functionality it provides, extending
		it with awareness of Unicode encodings for character positioning. This also provides easy access to the underlying `std::string`
		for easy interoperability with any code working with that type.
		
		String content is internally encoded/validated as UTF-8, but it can encode/decode to/from UTF16, UTF32, ASCII and ISO8859.
		All content must be valid UTF-8, i.e. you cannot embed arbitrary binary data or null characters in a `String` (use `Memory` instead).
		Character positions are calculated to allow indexing, but the time to find a position averages O(n)
		It is recommended to use classes like `BufferIn` to analyse by-character content on large blocks of text efficiently
	 
		This class does not use a 'special' value to denote non-existent or unspecified positions, e.g. like `string::npos`
		Rather, an optional is used for these cases, e.g. if searching for a dot outside the first 5 characters of some text,
		using std::string could look like this:
	
			if (auto pos = text.find("."); (pos != nos) && (pos > 4))
		
		With this `String` class, the optional response simplifies the syntax:
	 
			if (auto pos = text.find("."); pos > 4)
		
		The `basic_string` class also provides a range of static functions for validating or converting blocks of text for all supported encoding.
		Conversion operators and constructors provide interoperability with a range of common types, e.g. `std::string`, `std::u32string` etc
	*/
	template <class Alloc = std::allocator<char>>
	class basic_string {
	public:

		using enum text_encoding;

		//MARK: - Types
		
		using base = std::basic_string<char, std::char_traits<char>, Alloc>;
			///Class size type
		using size_type = string_position::size_type;
			///Unary predicate for filtering strings
		using Filter = std::function<bool(char32_t)>;
			///Unary functions for processing string characters
		using Function = std::function<std::optional<char32_t>(char32_t)>;
		
		//MARK: - Constants
		
			///Constant to indicate an unspecified or non-existant position in std::string
		static constexpr size_type npos = base::npos;
		
		static inline string_position no_pos{std::nullopt};
		
			///Default length precision (0.01mm)
		static constexpr double eps = 1e-5;
			///The line terminating char(s) for the current platform
#ifdef WINDOWS
		static inline const basic_string lineTerminator{"\r\n"};
#else
		static inline const basic_string lineTerminator{"\n"};
#endif
			///All white space characters
		static inline const basic_string allWhiteSpace{" \t\r\n"};
			///All line ending characters
		static inline const basic_string allLineEnding{"\r\n"};
			///All digit characters (0-9)
		static inline const basic_string allDigit{"0123456789"};
			///All numeric characters (floating and integer)
		static inline const basic_string allNumeric{"0123456789-"};
			///All numeric characters (floating and integer)
		static inline const basic_string allFloat{"0123456789.-"};

		/*!
		 Get a UTF-8 string from a UTF-32 source
		 @param text The source text (when valid, points to the next byte beyond the found character on return)
		 @param is_big_endian True if byte ordering is big-endian
		 @param howMany The number of 32-bit code points in the text (nullopt = null-terminated)
		 @param isCountRequired True if the specified number of code points must be valid (ignored if howMany = nullopt)
		 @return The UTF-8 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
		 */
		static std::optional<basic_string> from_unicode(const char32_t*& text, bool is_big_endian,
														string_position howMany = no_pos, bool isCountRequired = false);
		/*!
		 Make an STL string from a char array. NB: always returns a result, but result will only include valid chars
		 @param target An STL string to be populated with valid characters found
		 @param text The source text
		 @param howMany The number of bytes in the text (nullopt = null-terminated)
		 @param charCount The maximum number of (encoded) chars to read (npos = no limit)
		 @param format The text data format
		 @return The number of source bytes used in the populated string
		 */
		size_type make_string(base& target, const char* text, string_position howMany = no_pos,
							  string_position charCount = no_pos, text_format format = text_format{});
		/*!
		 Split the specified text into single and multi byte chars
		 @param source The source text
		 @return An array of single and multi-byte chars from the source string (nullopt on failure)
		*/
		static std::vector<basic_string> split_single_chars(const basic_string& source);

			//MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		basic_string() {}
		/*!
		 Constructor from an input char array
		 @param source The character array to be copied
		 @param howMany The number of bytes to copy (nullopt fornull-terminated)
		 @param format The source text data format
		 */
		basic_string(const char* source, string_position howMany = no_pos, text_format format = text_format{});
#ifndef __CLR_VER
		/*!
		 Constructor from an input UTF8 char array
		 @param source The character array to be copied
		 @param howMany The number of bytes to copy (nullopt for null-terminated)
		 */
		basic_string(const char8_t* source, string_position howMany = no_pos) : basic_string{reinterpret_cast<const char*>(source), howMany, text_format{}} {}
#endif // !__CLR_VER
		/*!
		 Constructor from an input UTF16 char array
		 @param source The character array to be copied
		 @param howMany The number of 16-bit chars to copy (nullopt for null-terminated)
		 */
		basic_string(const char16_t* source, string_position howMany = no_pos, bool is_big_endian = text_format::defaultEndian) : basic_string{reinterpret_cast<const char*>(source),
			howMany ? string_position(*howMany * sizeof(char16_t)) : no_pos, text_format{UTF16, is_big_endian}} {}
		/*!
		 Constructor from an input UTF32 char array
		 @param source The character array to be copied
		 @param howMany The number of 32-bit chars to copy (nullopt for null-terminated)
		 */
		basic_string(const char32_t* source, string_position howMany = no_pos, bool is_big_endian = text_format::defaultEndian) : basic_string{reinterpret_cast<const char*>(source),
			howMany ? string_position(*howMany * sizeof(char32_t)) : no_pos, text_format{UTF32, is_big_endian}} {}
		/*!
		 Constructor from an input string
		 @param source The string to be copied
		 @param howMany The number of chars to copy (nullopt for full length)
		 @param format The source text data format
		 */
		basic_string(const base& source, string_position howMany = no_pos, text_format format = text_format{}) :
		basic_string{source.data(), howMany, format} {}
#ifndef __CLR_VER
		/*!
		 Constructor from an input string
		 @param source The UTF-8 string to be copied
		 @param howMany The number of chars to copy (nullopt for full length)
		 */
		basic_string(const std::u8string& source, string_position howMany = no_pos) : basic_string(source.data(), howMany) {}
#endif // !__CLR_VER
		/*!
		 Constructor from an input string
		 @param source The UTF-16 string to be copied
		 @param howMany The number of chars to copy (nullopt for full length)
		 */
		basic_string(const std::u16string& source, string_position howMany = no_pos);
		/*!
		 Constructor from an input string
		 @param source The UTF-32 string to be copied
		 @param howMany The number of chars to copy (nullopt for full length)
		 */
		basic_string(const std::u32string& source, string_position howMany = no_pos);
		/*!
		 Constructor with optional text fill expression and number of repeats
		 @param newSize The required number of expression repeats
		 @param fillText The expression to fill the string
		 */
		basic_string(size_type newSize, const basic_string& fillText);
		/*!
		 Constructor from a char
		 @param val A char
		 */
		explicit basic_string(char val) : m_string{val}	{}
		/*!
		 Constructor from 16-bit integer
		 @param val A 16-bit integer
		 */
		explicit basic_string(int16_t val) { m_string = std::to_string(val); }
		/*!
		 Constructor from unsigned 16-bit integer
		 @param val An unsigned 16-bit integer
		 */
		explicit basic_string(uint16_t val) { m_string = std::to_string(val); }
		/*!
		 Constructor from 32-bit integer
		 @param val A 32-bit integer
		 */
		explicit basic_string(int32_t val) { m_string = std::to_string(val); }
		/*!
		 Constructor from unsigned 32-bit integer
		 @param val An unsigned 32-bit integer
		 */
		explicit basic_string(uint32_t val) { m_string = std::to_string(val); }
		/*!
		 Constructor from 64-bit integer
		 @param val A 64-bit integer
		 */
		explicit basic_string(int64_t val) { m_string = std::to_string(val); }
		/*!
		 Constructor from unsigned 64-bit integer
		 @param val An unsigned 64-bit integer
		 */
		explicit basic_string(uint64_t val) { m_string = std::to_string(val); }
#if !defined(WINDOWS) && !defined(__linux__)
		/*!
		 Constructor from string::size_type integer
		 @param val A string::size_type integer
		 */
		explicit basic_string(size_type val) { m_string = std::to_string(static_cast<uint64_t>(val)); }
#endif // !defined(WINDOWS) && !defined(__linux__)
		/*!
		 Constructor from double
		 @param val A double value
		 @param prec The required precision
		 @param padZero True to pad the number to the specified precision with zeros
		 */
		explicit basic_string(double val, double prec = eps, bool padZero = false);
		/*!
		 Copy constructor
		 @param source The string to copy
		 */
		basic_string(const basic_string& source) : m_string{source.m_string} {}
		/*!
		 Move constructor
		 @param source The object to move
		 */
		basic_string(basic_string&& source) noexcept : m_string{std::move(source.m_string)} {}
		/*!
		 Destructor
		 */
		~basic_string() {}
		
			//MARK: - Conversion operators
		
			///Conversion to std::string
		operator base() const { return m_string; }
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
		basic_string& operator= (const basic_string& source);
		/*!
		 Move assignment operator
		 @param source The object to move
		 @return A reference to this
		 */
		basic_string& operator= (basic_string&& source) noexcept;
		/*!
		 Assignment operator
		 @param source A pointer to a char array
		 @return A reference to this
		 */
		basic_string& operator= (const char* source);
		/*!
		 Three-way comparison operator
		 @param ref The string to compare this to
		 @return The relationship between this and ref (less, equal, greater)
		 */
		std::strong_ordering operator<=> (const basic_string& ref) const { return compare(ref); }
		/*!
		 Addition with assignment operator
		 @param source The string to append
		 @return A reference to this
		 */
		basic_string& operator+=(const basic_string& source) { return append(source); }
		/*!
		 Addition with assignment operator
		 @param source The char to append
		 @return A reference to this
		 */
		basic_string& operator+=(char source) { return append(source); }
		
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
		 bytes consumed by the string (NB: basic_string are null terminated, but this is not included in the data length)
		 */
		size_type dataSize(string_position howMany = no_pos) const;
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
		base string() const { return m_string; }
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
		 Apply a function to specified characters in the string in reverse order
		 @param func The character function (the returned value is ignored)
		 */
		void rforEach(const Function& func) const;
		/*!
		 Get a specified substring of this string
		 @param startPos The position of the first character
		 @param howMany The number of characters to get (nullopt for full length)
		 @return The requested string segment
		 */
		basic_string substr(size_type startPos = 0, string_position howMany = no_pos) const;
		/*!
		 Create an uppercase version of the string
		 @return An uppercase version of the string
		 */
		basic_string uppercase() const;
		/*!
		 Create a lowercase version of the string
		 @return A lowercase version of the string
		 */
		basic_string lowercase() const;
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
		 Determine if the string is entirely alphanumeric
		 @param startPos The position to checking from
		 @param howMany The number of characters to check (nullopt = to end)
		 @return True is the string is alphanumeric
		 */
		bool isAlphaNumeric(size_type startPos = 0, string_position howMany = no_pos) const;
		/*!
		 Determine if the string is entirely letters
		 @param startPos The position to checking from
		 @param howMany The number of characters to check (nullopt = to end)
		 @return True is the string is letters
		 */
		bool isAlpha(size_type startPos = 0, string_position howMany = no_pos) const;
		/*!
		 Determine if the string is entirely numeric
		 @param startPos The position to checking from
		 @param howMany The number of characters to check (nullopt = to end)
		 @return True is the string is numbers
		 */
		bool isNumeric(size_type startPos = 0, string_position howMany = no_pos) const;
		/*!
		 Three-way comparison to a reference string
		 @param ref The string to compare this to
		 @return The relationship between this and ref (less, equal, greater)
		 */
		std::strong_ordering compare(const basic_string& ref) const;
		/*!
		 Find the specified string within this
		 @param toFind The string to find
		 @param startPos The character to start searching from
		 @return The index where a match is found (nullopt = not found)
		 */
		string_position find(const basic_string& toFind, size_type startPos = 0) const;
		/*!
		 Find the specified string within this using a filter
		 @param filter The string filter
		 @return The index where a match is found (nullopt = not found)
		 */
		string_position findIf(const Filter& filter) const;
		/*!
		 Find the specified string within this searching in reverse and using a filter
		 @param filter The string filter
		 @param lastPos The position to begin filtering from
		 @param howMany The number of chars to filter
		 @return The index where a match is found (nullopt = not found)
		 */
		string_position rfindIf(const Filter& filter, string_position lastPos = no_pos, string_position howMany = no_pos) const;
		/*!
		 Determine if the string contains a substring
		 @param toFind The substring to find
		 @return True if the substring is found
		 */
		bool contains(const basic_string& toFind) const { return find(toFind).has_value(); }
		/*!
		 Determine if this string starts with a substring
		 @param toFind The substring to find
		 @return True if this string starts with the substring
		 */
		bool startsWith(const basic_string& toFind) const { return m_string.starts_with(toFind.m_string); }
		/*!
		 Determine if this string ends with a substring
		 @param toFind The substring to find
		 @return True if this string ends with the substring
		 */
		bool endsWith(const basic_string& toFind) const { return m_string.ends_with(toFind.m_string); }
		/*!
		 Find the first character which is in a specified string
		 @param toFind A string of characters to find
		 @param startPos The character to begin searching from
		 @return The index of the first matching character (nullopt = not found)
		 */
		string_position findFirstOf(const basic_string& toFind, size_type startPos = 0) const;
		/*!
		 Find the first character not in a specified string
		 @param toFind A string of characters to not find
		 @param startPos The character to begin searching from
		 @return The index of the first non-matching character (nullopt = not found)
		 */
		string_position findFirstNotOf(const basic_string& toFind, size_type startPos = 0) const;
		/*!
		 Find the last character in this string which is in the specified string
		 @param toFind A string of characters to find
		 @param lastPos The position of the last character to be compared
		 @return The index of the last matching character (nullopt = not found)
		 */
		string_position findLastOf(const basic_string& toFind, string_position lastPos = no_pos) const;
		/*!
		 Find the last character in this string which is not in the specified string
		 @param toFind A string of characters not to find
		 @param lastPos The position of the last character to be compared
		 @return The index of the last non-matching character (nullopt = not found)
		 */
		string_position findLastNotOf(const basic_string& toFind, string_position lastPos = no_pos) const;
		/*!
		 Find the specified string searching backwards
		 @param toFind The string to find
		 @param lastPos The position of the last character to be compared
		 @return The index of a matching string (nullopt = not found)
		 */
		string_position rfind(const basic_string& toFind, string_position lastPos = no_pos) const;
		
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
		void resize(size_type newSize, const basic_string& padding = "");
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
		 Apply a function to specified characters in the string in reverse order
		 @param func The character function (the returned value replaces the input character)
		 */
		void rforEach(const Function& func);
		/*!
		 Assign a specified string to this
		 @param source The string to assign
		 @return A reference to this
		 */
		basic_string& assign(const basic_string& source);
		/*!
		 Assign a specified string to this
		 @param source The character array to be copied
		 @param byteCount The maximum number of bytes in the array (nullopt = null-terminated)
		 @param charCount The maximum number of (encoded) characters to read (nullopt = as byteCount limit)
		 @param format The source text data format
		 @return The number of bytes assigned from the source
		 */
		size_type assign(const char* source, string_position byteCount = no_pos,
						 string_position charCount = no_pos, text_format format = text_format{});
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
		basic_string& append(const basic_string& source);
		/*!
		 Append the specified char to this (NB: don't use this casually - encoding must be assumed and converted accordingly)
		 @param source The char to append
		 @return A reference to this
		 */
		basic_string& append(char source);
		/*!
		 Append the specified unicode char to this
		 @param source The char to append (assumed to be UTF-32)
		 @return A reference to this
		 */
		basic_string& append(char32_t source) {
			const char32_t* sourcePtr = &source;
			if (auto sourceStr = from_unicode(sourcePtr, text_format::defaultEndian, 1); sourceStr)
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
		basic_string& insert(size_type pos, const basic_string& source, size_type start = 0, string_position howMany = no_pos);
		/*!
		 Replace a specified string segment with another string
		 @param pos The position to begin replacing (nullopt = append to end)
		 @param num The number of chars to replace (nullopt = to the string end)
		 @param source The replacement string
		 @param start The start point in the replacement string
		 @param howMany The number of chars to extract from the replacement string (nullopt inserts all)
		 @return A reference to this
		 */
		basic_string& replace(string_position pos, string_position num, const basic_string& source,
							  size_type start = 0, string_position howMany = no_pos);
		/*!
		 Replace all instances of a specified expression
		 @param toFind The expression to be replaced
		 @param replacement The replacement expression
		 @return A reference to this
		 */
		basic_string& replaceAll(const basic_string& toFind, const basic_string& replacement = basic_string());
		/*!
		 Replace all chars passing a specified filter
		 @param filter The filter for chars to be replaced
		 @param replacement The replacement expression
		 @return A reference to this
		 */
		basic_string& replaceIf(const Filter& filter, const basic_string& replacement = basic_string());
		/*!
		 Replace any instances of specified characters
		 @param charsToFind The characters to be replaced (NB: each character is treated independently - the order does not matter)
		 @param replacement The replacement expression
		 @return A reference to this
		 */
		basic_string& replaceAnyOf(const basic_string& charsToFind, const basic_string& replacement = basic_string());
		/*!
		 Erase a specified range of characters from a string
		 @param pos The position to erasing from
		 @param howMany The number of characters to erase (nullopt to erase to end)
		 @return A reference to this
		 */
		basic_string& erase(size_type pos = 0, string_position howMany = no_pos);
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
		basic_string& padRight(size_type length, const basic_string& repeat = " ");
		/*!
		 Pad the string with a repeated character to reach a specified length (so the existing content is flush with the left)
		 @param length The required string length
		 @param repeat The character to repeatedly append util the length is met
		 @return A reference to this
		 */
		basic_string& padLeft(size_type length, const basic_string& repeat = " ");
		
		private:
			///The string content - NB: The STL representation works for many UTF-8 functions, and this class manages the remainder
		base m_string;
	};

	using string = active::basic_string<>;
	using String = active::basic_string<>;

}

namespace active::string_function {
	
	// MARK: - Common string functions
	
	/*!
	 Get the number of bytes in the specified text (counting only valid UTF8 characters)
	 @param text The source text
	 @param howMany The number of bytes in the text
	 @param charCount The maximum number of (encoded) chars to seek
	 @param format The text data format
	 @return The number of bytes in the text containing valid UTF8 characters
	 */
	string::size_type getValidByteCount(const char* text, string_position howMany = string::no_pos,
										string_position charCount = string::no_pos, text_format format = text_format{});
	/*!
	 Get the width of a specified character in bytes
	 @param text The source text
	 @param howMany The number of bytes in the array
	 @param format The text data format
	 @return The character width in bytes (nullopt for bad encoding)
	 */
	std::optional<unsigned char> getCharacterByteCount(const char* text, string_position howMany = string::no_pos,
													   text_format format = text_format{});
	/*!
	 Return the length of a string in bytes, limited by a character count
	 @param text The source text
	 @param howMany The number of characters to count (nullopt = null-terminated)
	 @param isCountRequired True if the number of characters must exist in the text (unless howMany = nullopt)
	 @param format The text data format
	 @return The number of bytes in the char array (nullopt if isCountRequired and howMany not reached)
	 */
	string_position getByteCountCharLimited(const char* text, string_position howMany = string::no_pos,
												  bool isCountRequired = false, text_format format = text_format{});
	/*!
	 Get the number of valid characters found at a specified address
	 @param text The source text
	 @param format The text data format
	 @param howMany The number of bytes in the array (nullopt = null-terminated)
	 @return The nummber of characters found (nullopt if bad encoding found)
	 */
	string_position getCharacterCount(const char* text, string_position howMany = string::no_pos,
											text_format format = text_format{});
	/*!
	 Calculate a required number of characters base on a specified byte count, word size and (optional) character limit
	 @param wordSize The character word size, e.g. UTF16 = 2, UTF32 = 4
	 @param howMany The number of bytes (no_pos = null-terminated)
	 @param charCount The maximum number of (encoded) chars to read (npos = no limit)
	 @return The required number of characters
	 */
	string_position getCharCount(unsigned char wordSize, string_position howMany = string::no_pos,
							   string_position charCount = string::no_pos);
	/*!
	 Determine the the number of characters in a char array
	 @param text The source text
	 @param howMany The number of bytes in the text (nullopt = null-terminated)
	 @param format The text data format
	 @return The number of characters in the array
	 */
	string::size_type getStringLength(const char* text, string_position howMany = string::no_pos,
									  text_format format = text_format{});
	/*!
	 Determine if a 16-bit (UTF16) character code is within the BMP
	 @param code The character code
	 @return True if a 16-bit (UTF16) character code is within the BMP
	 */
	inline bool isWithinBMP(char16_t code) { return ((code < 0xD800) || (code >= 0xE000)); }
	/*!
	 Determine if a 32-bit (UTF32) character code is valid
	 @param uniChar The character code
	 @return True if a 32-bit (UTF32) character code is valid
	 */
	inline bool isValidUnicode(char32_t uniChar) { return ((uniChar <= 0x10FFFF) && ((uniChar > 0xDFFF) || (uniChar < 0xD800))); }
	/*!
	 Get a UTF-32 char from a UTF-8 source
	 @param text The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	 @param howMany The number of available bytes in the source
	 @return A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
	 */
	std::pair<char32_t, unsigned char> getUTF32CharFromUTF8(const char*& text, string::size_type howMany);
	/*!
	 Get a UTF-32 char from a UTF-16 source
	 @param text The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	 @param is_big_endian True if byte ordering is big-endian
	 @param howMany The number of available bytes in the source
	 @return A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
	 */
	std::pair<char32_t, unsigned char> getUTF32CharFromUTF16(const char16_t*& text, bool is_big_endian, string::size_type howMany);
	/*!
	 Get a unicode (UTF-32) character from a specified source
	 @param text The source text
	 @param howMany The number of bytes in the text
	 @param format The source data format
	 @return The unicode char paired with the number of bytes consumed from the source (0 = no valid char found)
	 */
	std::pair<char32_t, unsigned char> getUnicodeChar(const char* text, string_position howMany = string::no_pos,
													  text_format format = text_format{});
	/*!
	 Get a UTF-32 string from a UTF-8 source
	 @param text The source UTF-8 text (advances to the byte beyond the last counted character)
	 @param howMany The number of bytes in the text (nullopt = null-terminated)
	 @param isCountRequired True if the specified number of bytes must be valid (ignored if howMany = nullopt)
	 @return The unicode code point for the specified chars (nullopt on failure)
	 */
	std::optional<std::u32string> toUnicode(const char*& text, string_position howMany = string::no_pos, bool isCountRequired = false);
	/*!
	 Get a UTF-32 string from a UTF-16 (16-bit) source
	 @param text The source text (when valid, points to the next byte beyond the found character on return)
	 @param is_big_endian True if byte ordering is big-endian
	 @param howMany The number of words (16-bit values) in the text (nullopt = null-terminated)
	 @param isCountRequired True if the specified number of words must be valid (ignored if howMany = nullopt)
	 @return The UTF-32 string read from the UTF-16 source (nullopt on error, including failure to meet isCountRequired condition)
	 */
	std::optional<std::u32string> fromUTF16(const char16_t*& text, bool is_big_endian,
											string_position howMany = string::no_pos, bool isCountRequired = false);
	/*!
	 Get a UTF-16 string from a UTF-32 source
	 @param text The source text (when valid, points to the next byte beyond the found character on return)
	 @param howMany The number of code points in the text (nullopt = null-terminated)
	 @param isCountRequired True if the specified number of code points must be valid (ignored if howMany = nullopt)
	 @return The UTF-16 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
	 */
	std::optional<std::u16string> toUTF16(const char32_t*& text, string_position howMany = string::no_pos, bool isCountRequired = false);
	/*!
	 Collect the byte size of each (valid) character from a string into an array
	 @param text The source text
	 @param howMany The number of characters to collect, taken as a maximum rather than a requirement (nullopt = null-terminated)
	 @param format The text data format (collection will stop if a character not matching the encoding is found)
	 @return An array containing the byte size of each character found (nullopt if no valid chars found)
	 */
	std::vector<unsigned char> collectCharByteCount(const char* text, string_position howMany = string::no_pos,
													text_format format = text_format{});
	/*!
	 Calculate the byte offsets for a start and number of chars within a string
	 @param text The source text
	 @param startPos The start character
	 @param howMany The number of characters to measure (nullopt = null-terminated)
	 @param isHowManyChars True if the returned second value should be the char count rather than the byte count
	 @return Byte offset to the start character paired with byte/char offset from the start to the end of the last character (nullopt on failure)
	 */
	std::optional<std::pair<string::size_type, string::size_type>> getByteOffsets(const char* text, string::size_type startPos = 0,
																				  string_position howMany = string::no_pos,
																				  bool isHowManyChars = false);
	
}

namespace active {
		
	/*!
	 Equality operator
	 @param str1 The first string
	 @param str2 The second string
	 @return True if the strings are identical
	 */
	template <class Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const basic_string<Alloc>& str2) {
		switch (str1.empty() + (2 * str2.empty())) {
			case 0: [[likely]]
				return std::strcmp(str1.data(), str2.data()) == 0;
			case 3:
				return true;
			default:
				break;
		}
		return false;
	}

	template <class Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::u8string& str2) {
		return str1 == basic_string{str2};
	}

	template <class Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::u16string& str2) {
		return str1 == basic_string{str2};
	}

	template <class Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::u32string& str2) {
		return str1 == basic_string{str2};
	}

	template <class Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const char* str2) {
		return str1 == basic_string{str2};
	}

	template <class Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const char8_t* str2) {
		return str1 == basic_string{str2};
	}

	
	/*!
	 Inequality operator
	 @param str1 The first string
	 @param str2 The second string
	 @return True if the strings differ
	 */
	template <class Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const basic_string<Alloc>& str2) {
		return !(str1 == str2);
	}

	template <class Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::u8string& str2) {
		return !(str1 == basic_string{str2});
	}

	template <class Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::u16string& str2) {
		return !(str1 == basic_string{str2});
	}

	template <class Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::u32string& str2) {
		return !(str1 == basic_string{str2});
	}
	
	template <class Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const char* str2) {
		return !(str1 == basic_string{str2});
	}

	template <class Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const char8_t* str2) {
		return !(str1 == basic_string{str2});
	}


	/*!
		Addition operator
		@param str1 The first string
		@param str2 The second string
		@return The concatonated string
	*/
	template <class Alloc>
	inline basic_string<Alloc> operator+(const basic_string<Alloc>& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <class Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::u8string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <class Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::u16string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <class Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::u32string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <class Alloc>
	inline bool operator+ (const std::u8string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <class Alloc>
	inline bool operator+ (const std::u16string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <class Alloc>
	inline bool operator+ (const std::u32string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <class Alloc>
	inline basic_string<Alloc> operator+(const basic_string<Alloc>& str1, const char* str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <class Alloc>
	inline basic_string<Alloc> operator+(const basic_string<Alloc>& str1, const char8_t* str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <class Alloc>
	inline basic_string<Alloc> operator+(const char* str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <class Alloc>
	inline basic_string<Alloc> operator+(const char8_t* str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(str2);
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


	// MARK: - Constructors

	/*--------------------------------------------------------------------
		Constructor

		source: The character array to be copied
		howMany: The number of bytes to copy (nullopt for full length)
		encoding: The character encoding type
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::basic_string(const char* source, string_position howMany, text_format format) {
		 make_string(m_string, source, howMany, no_pos, format);
	} //basic_string<Alloc>::basic_string


	/*--------------------------------------------------------------------
		Constructor from an input string
	 
		source: The UTF-16 string to be copied
		howMany: The number of chars to copy (nullopt for full length)
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::basic_string(const std::u16string& source, string_position howMany) {
		const char16_t* text = source.data();
			//First convert to UTF-32
		if (auto string32 = string_function::fromUTF16(text, text_format::defaultEndian, howMany); string32) {
				//Then convert UTF-32 to UTF-8
			const char32_t* text32 = string32->data();
				//NB: We are assuming that the incoming u16string byte ordering is the platform default
			if (auto uniString = from_unicode(text32, text_format::defaultEndian); uniString)
				m_string = uniString->m_string;
		}
	} //basic_string<Alloc>::basic_string


	/*--------------------------------------------------------------------
		Constructor from an input string
	 
		source: The UTF-32 string to be copied
		howMany: The number of chars to copy (nullopt for full length)
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::basic_string(const std::u32string& source, string_position howMany) {
		const char32_t* text = source.data();
			//NB: We are assuming that the incoming u32string byte ordering is the platform default
		if (auto uniString = from_unicode(text, text_format::defaultEndian, howMany); uniString)
			m_string = uniString->m_string;
	} //basic_string<Alloc>::basic_string


	/*--------------------------------------------------------------------
		Constructor with optional text fill expression and number of repeats
	 
		newSize: The required number of expression repeats
		fillText: The expression to fill the resized string
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::basic_string(size_type newSize, const basic_string& fillText) {
		m_string = std::string{};
		basic_string<Alloc>::resize(newSize, fillText);
	} //basic_string<Alloc>::basic_string


	/*--------------------------------------------------------------------
		Constructor from double
	 
		val: A double value
		prec: The required precision
		padZero: True to pad the number to the specified precision with zeros
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::basic_string(double val, double prec, bool padZero) {
			//std::format is faster than stringstream - use where available
#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 13)) || (defined(__clang__) && (__clang_major__ < 15))
		std::ostringstream text;
		text.setf(std::ios::fixed, std::ios::floatfield);
		auto dp = static_cast<string::size_type>(math::round(std::max(0.0, -log10(prec)), 1.0));
		text.precision(dp);
		text << val;
		m_string = text.str();
#else
		int32_t dec = std::max(0, static_cast<int32_t>(-log10(prec)));
		m_string = std::format("{:.{}f}", val, dec);
#endif
			//Padding is added by default, so strip it when unwanted
		if (!padZero) {
			if (auto pointPos = m_string.find('.'); pointPos != npos) {
				auto lastZero = m_string.find_last_not_of("0");
				m_string = substr(0, (pointPos == lastZero) ? pointPos : lastZero + 1);
			}
		}
	} //basic_string<Alloc>::basic_string

		
	/*--------------------------------------------------------------------
		Get a UTF-8 string from a UTF-32 source
	 
		text: The source text
		is_big_endian: True if byte ordering is big-endian
		howMany: The number of 32-bit code points in the text (nullopt = null-terminated)
		isCountRequired: True if the specified number of  code points must be valid (ignored if howMany = nullopt)
	 
		return: The UTF-8 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	std::optional<basic_string<Alloc>> basic_string<Alloc>::from_unicode(const char32_t*& text, bool is_big_endian,
																		 string_position howMany, bool isCountRequired) {
		if (howMany == 0)
			return std::optional(basic_string{});	//An empty string is not an error, so we don't return no_pos
		basic_string result;
		for ( ; *text != 0; ++text) {
			if (howMany) {
				if (howMany == 0)
					break;
				*howMany -= 1;
			}
			char32_t code = *text;
				//Deal with 7-bit codes first
			if (code < 0x80) {
				result.m_string += static_cast<char>(code);
				continue;
			}
				//Weed out invalid codes
			if ((code > 0x10FFFF) || ((code >= 0x110000) && (code <= 0x1FFFFF)) || ((code >= 0xD800) && (code <= 0xDFFF)))
				return std::nullopt;	//Bad encoding
			char buffer[4] = {0};
			unsigned char mask = 0x80;
			string::size_type offset = 3;
			do {
				mask >>= 1;
				mask |= 0x80;
				buffer[offset] = (static_cast<char>(code) & 0x3F) | 0x80;
				--offset;
				code >>= 6;
			} while (code > static_cast<char32_t>((mask ^ 0xFF) >> 1));
			buffer[offset] = static_cast<char>(code) | mask;
			result.m_string.append(buffer + offset, 4 - offset);
		}
		return (isCountRequired && (howMany > 0)) ? std::nullopt : std::optional(result);
	} //basic_string<Alloc>::from_unicode

	
	/*--------------------------------------------------------------------
		Make an STL string from a char array. NB: always returns a result, but result will only include valid chars
		
		target: An STL string to be populated with valid characters found
		text: The source text
		howMany: The number of bytes in the text (nullopt = null-terminated)
		charCount: The maximum number of (encoded) chars to read (npos = no limit)
		format: The text data format
		
		return: The number of source bytes used in the populated string
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::make_string(base& target, const char* text, string_position howMany,
																	string_position charCount, text_format format) {
		target.clear();
		if (text == nullptr)
			return 0;
		size_type dataSize = 0;
		switch (format.encoding) {
			case UTF8: case ascii: {
				dataSize = string_function::getValidByteCount(text, howMany, charCount, format.encoding);
				if (dataSize < 1)
					break;
					//Any valid bytes can be consumed directly by std::string constructor
				target.assign(text, dataSize);
				break;
			}
			case UTF16: {
				const auto* source = reinterpret_cast<const char16_t*>(text);
				if (auto string32 = string_function::fromUTF16(source, format.is_big_endian,
															   string_function::getCharCount(sizeof(char16_t), howMany, charCount)); string32) {
					const char32_t* source32 = string32->data();
					if (auto uniString = from_unicode(source32, format.is_big_endian); uniString) {
						target = *uniString;
						dataSize = target.size() * sizeof(char16_t);
					}
				}
				break;
			}
			case UTF32: {
				const auto* source = reinterpret_cast<const char32_t*>(text);
				if (auto uniString = from_unicode(source, format.is_big_endian,
												  string_function::getCharCount(sizeof(char32_t),howMany, charCount)); uniString) {
					target = *uniString;
					dataSize = target.size() * sizeof(char32_t);
				}
				break;
			}
			case ISO8859_1:
				dataSize = string_function::getValidByteCount(text, howMany, charCount, format);
				if (dataSize < 1)
					break;
				const auto* source = reinterpret_cast<const unsigned char*>(text);
				for ( ; dataSize--; ++source) {
					if (*source < 0x80)
						target += *source;
					else if (*source < 0xC0) {
						target += static_cast<const unsigned char>(0xC2);
						target += *source;
					} else {
						target += static_cast<const unsigned char>(0xC3);
						target += (*source - 0x40);
					}
				}
				break;
		}
		return dataSize;
	} //basic_string<Alloc>::make_string
	
	
	/*--------------------------------------------------------------------
		Split the specified text into single and multi byte chars
	 
		source: The source text
	 
		return: An array of single and multi-byte chars from the source string (nullopt on failure)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	std::vector<basic_string<Alloc>> basic_string<Alloc>::split_single_chars(const basic_string& source) {
		std::vector<basic_string> result;
		auto charBytes = string_function::collectCharByteCount(source.data());
		if (charBytes.empty())
			return result;
			//The first string in the result is reserved for single-byte chars
		std::string singleChars;
		std::set<basic_string> multiChar;
		const auto* text = source.data();
			//Iterate through all chars, either appending to the single-bytes chars item or collecting in a set of unique multi-byte chars
		for (auto& charSize : charBytes) {
			if (charSize == 1) {
					//Make sure we don't have the char already
				if (singleChars.find(text[0]) == std::string::npos)
					singleChars += text[0];
			} else
				multiChar.insert(basic_string{text, charSize});
				//Move to the next char
			text += charSize;
		}
		result.reserve(multiChar.size() + 1);
		result.emplace_back(basic_string{singleChars});
			//Append the unique multi-byte chars to the result
		for (const auto& i : multiChar)
			result.emplace_back(i);
		return result;
	} //basic_string<Alloc>::split_single_chars
	
	//MARK: - Conversion operators

#ifndef __CLR_VER
	/*--------------------------------------------------------------------
		Conversion to std::u16string
	 
		return: The equivalent std::u16string (UTF-16)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::operator std::u8string() const {
		return std::u8string{reinterpret_cast<const char8_t*>(data())};
	} //basic_string<Alloc>::operator std::u8string
#endif // !__CLR_VER


	/*--------------------------------------------------------------------
		Conversion to std::u16string
	 
		return: The equivalent std::u16string (UTF-16)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::operator std::u16string() const {
		const auto* text = data();
		if (auto uniString = string_function::toUnicode(text); uniString) {
			const char32_t* text32 = uniString->data();
			if (auto uniString16 = string_function::toUTF16(text32); uniString16)
				return *uniString16;
		}
		return std::u16string{};
	} //basic_string<Alloc>::operator std::u16string


	/*--------------------------------------------------------------------
		Conversion to std::u32string
	 
		return: The equivalent std::u32string (UTF-32)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::operator std::u32string() const {
		const auto* text = data();
		auto uniString = string_function::toUnicode(text);
		return uniString ? *uniString : std::u32string{};
	} //basic_string<Alloc>::operator std::u32string

	// MARK: - Operators

	/*--------------------------------------------------------------------
		Subscript operator
		
		index: The required character position (unchecked - out of bounds behaviour undefined)
		
		return: The character at the specified index
	  --------------------------------------------------------------------*/
	template <class Alloc>
	char32_t basic_string<Alloc>::operator[](size_type index) const {
		auto offsets = string_function::getByteOffsets(data(), index, 1);
		auto source = data() + offsets->first;
		return string_function::getUTF32CharFromUTF8(source, offsets->second).first;
	} //basic_string<Alloc>::operator[]


	/*--------------------------------------------------------------------
		Assignment operator

		source: The object to copy

		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator= (const basic_string& source) {
		if (&source == this)
			return *this;
		return assign(source);
	} //basic_string<Alloc>::operator=


	/*--------------------------------------------------------------------
		Move assignment operator
	 
		source: The object to move
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator= (basic_string&& source) noexcept {
		m_string = std::move(source.m_string);
		return *this;
	} //basic_string<Alloc>::operator=


	/*--------------------------------------------------------------------
		Assignment operator

		source: A pointer to a char array

		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator= (const char* source) {
		m_string.clear();
		make_string(m_string, source);
		return *this;
	} //basic_string<Alloc>::operator=

	//MARK: - Functions (const)

	/*--------------------------------------------------------------------
		Return the number of bytes this string can contain

		return: The number of bytes this string can contain
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::capacity() const {
		return static_cast<basic_string<Alloc>::size_type>(m_string.capacity());
	} //basic_string<Alloc>::capacity


	/*--------------------------------------------------------------------
		Get the number of characters in the string
	 
		return: The number of characters in the string
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::size() const {
		return string_function::getStringLength(m_string.data());
	} //basic_string<Alloc>::size


	/*--------------------------------------------------------------------
		Return the number of bytes consumed by this string

		howMany: The number of chars to count (nullopt = for full length)
	 
		return: The number of bytes consumed by the string (NB: Strings are null terminated, but this is not included in the data length)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::dataSize(string_position howMany) const {
		if (!howMany)
			return static_cast<basic_string<Alloc>::size_type>(m_string.size());
		if (auto byteCount = string_function::getByteCountCharLimited(m_string.data(), howMany); byteCount)
			return *byteCount;
		return 0;
	} //basic_string<Alloc>::dataSize


	/*--------------------------------------------------------------------
		Subscript operator
		
		index: The required character position (unchecked - out of bounds behaviour undefined)
		
		return: The character at the specified index
	  --------------------------------------------------------------------*/
	template <class Alloc>
	char32_t basic_string<Alloc>::at(size_type index) const {
		auto offsets = getByteOffsets(data(), index, 1);
		if (!offsets)
			throw std::out_of_range("");
		auto source = data() + offsets->first;
		return getUTF32CharFromUTF8(source, offsets->second).first;
	} //basic_string<Alloc>::at


	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string
	 
		func: The character function (the returned value is ignored)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	void basic_string<Alloc>::forEach(const Function& func) const {
		auto remaining = dataSize();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = getUTF32CharFromUTF8(text, remaining); consumed != 0) {
				remaining -= consumed;
				func(incoming);
			} else
				break;
		}
	} //basic_string<Alloc>::forEach

	
	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string in reverse order
	 
		func: The character function (the returned value is ignored)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	void basic_string<Alloc>::rforEach(const Function& func) const {
		
	}
	

	/*--------------------------------------------------------------------
		Get a specified segment of this string

		startPos: The first character of the segment
		howMany: The number of characters in the segment (nullopt for full length)

		return: The requested string segment
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc> basic_string<Alloc>::substr(size_type startPos, string_position howMany) const {
		auto offsets = string_function::getByteOffsets(data(), startPos, howMany);
		if (!offsets)
			return {};
		return m_string.substr(offsets->first, offsets->second);
	} //basic_string<Alloc>::substr


	/*--------------------------------------------------------------------
		Create an uppercase version of the string
	 
		return: An uppercase version of the string
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc> basic_string<Alloc>::uppercase() const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty())
			return *this;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!isWithinBMP(*i)) {
				++i;
				continue;
			}
#endif // WINDOWS
			*i = std::toupper(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8"));	//NB: Need to consider if locale is appropriate
		}
		return uniString;
	} //basic_string<Alloc>::lowercase


	/*--------------------------------------------------------------------
		Create a lowercase version of the string
	 
		return: A lowercase version of the string
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc> basic_string<Alloc>::lowercase() const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty())
			return *this;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!isWithinBMP(*i)) {
				++i;
				continue;
			}
#endif // WINDOWS
			*i = std::tolower(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8"));	//NB: Need to consider if locale is appropriate
		}
		return uniString;
	} //basic_string<Alloc>::lowercase


	/*--------------------------------------------------------------------
		Determine if the string is entirely alphanumeric
	 
		startPos: The position to checking from
		howMany: The number of characters to check (nullopt = to end)

		return: True if the string is alphanumeric
	  --------------------------------------------------------------------*/
	template <class Alloc>
	bool basic_string<Alloc>::isAlphaNumeric(size_type startPos, string_position howMany) const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty())
			return false;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!isWithinBMP(*i))
				return false;
#endif
			if (!std::isalnum(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8")))	//NB: Need to consider if locale is appropriate
				return false;
		}
		return true;
	} //basic_string<Alloc>::isAlphaNumeric


	/*--------------------------------------------------------------------
		Determine if the string is entirely letters
	  
		startPos: The position to checking from
		howMany: The number of characters to check (nullopt = to end)

		return: True if the string is letters
	  --------------------------------------------------------------------*/
	template <class Alloc>
	bool basic_string<Alloc>::isAlpha(size_type startPos, string_position howMany) const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty())
			return false;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!isWithinBMP(*i))
				return false;
#endif
			if (!std::isalpha(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8")))	//NB: Need to consider if locale is appropriate
				return false;
		}
		return true;
	} //basic_string<Alloc>::isAlpha


	/*--------------------------------------------------------------------
		Determine if the string is entirely numbers
	 
		startPos: The position to checking from
		howMany: The number of characters to check (nullopt = to end)
	 
		return: True if the string is numbers
	  --------------------------------------------------------------------*/
	template <class Alloc>
	bool basic_string<Alloc>::isNumeric(size_type startPos, string_position howMany) const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty())
			return false;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!isWithinBMP(*i))
				return false;
#endif
			if (!std::iswdigit(static_cast<wchar_t>(*i)))
				return false;
		}
		return true;
	} //basic_string<Alloc>::isNumeric


	/*--------------------------------------------------------------------
		Three-way comparison to a reference string
	 
		ref: The string to compare this to
	 
		return: The relationship between this and ref (less, equal, greater)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	std::strong_ordering basic_string<Alloc>::compare(const basic_string& ref) const {
		std::u32string myString{*this}, refString{ref};
		return myString <=> refString;
	} //basic_string<Alloc>::compare


	/*--------------------------------------------------------------------
		Find the specified string within this

		toFind: The string to find
		startPos: The character to start searching from

		return: The index where a match is found (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::find(const basic_string& toFind, size_type startPos) const {
		auto startByte = (startPos == 0) ? string_position{0} : string_function::getByteCountCharLimited(m_string.data(), startPos, true);
		if (!startByte)
			return no_pos;
		auto foundPos = m_string.find(toFind.m_string, *startByte);
		if (foundPos == npos)
			return no_pos;
		return string_function::getCharacterCount(m_string.data(), foundPos);
	} //basic_string<Alloc>::find


	/*--------------------------------------------------------------------
		Find the specified string within this using a filter
	 
		filter: The string filter
	 
		return: The index where a match is found (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::findIf(const Filter& filter) const {
		auto remaining = dataSize();
		auto text = data();
		for (size_type index = 0; ; ++index) {
			if (auto [incoming, consumed] = string_function::getUTF32CharFromUTF8(text, remaining); consumed != 0) {
				if (filter(incoming))
					return index;
				remaining -= consumed;
			} else
				break;
		}
		return no_pos;
	} // basic_string<Alloc>::findIf

	
	/*--------------------------------------------------------------------
		Find the specified string within this searching in reverse and using a filter
	 
		filter: The string filter
		lastPos: The position to begin filtering from
		howMany: The number of chars to filter
	 
		return: The index where a match is found (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::rfindIf(const Filter& filter, string_position lastPos, string_position howMany) const {
		auto charBytes = string_function::collectCharByteCount(data(), lastPos);
		if (charBytes.empty())
			return no_pos;
			//Add up all the bytes in the string (total number of bytes used by the string)
		auto lastByte = std::reduce(charBytes.begin(), charBytes.end());
		size_type index = charBytes.size();
		size_type minIndex = (howMany && (*howMany >= index)) ? index - *howMany : 0;
		const char* text = data() + lastByte;
		for (auto iter = charBytes.rbegin(); (iter != charBytes.rend()) && (index-- != minIndex); text -= *iter, ++iter) {
			text -= *iter;
			if (auto [incoming, consumed] = string_function::getUTF32CharFromUTF8(text, *iter); consumed != 0) {
				if (filter(incoming))
					return index;
			} else
				break;
		}
		return no_pos;
	} //basic_string<Alloc>::rfindIf
	

	/*--------------------------------------------------------------------
		Find the first character which is in a specified string

		toFind: A string of characters to find
		startPos: The character to begin searching from

		return: The index of the first matching character (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::findFirstOf(const basic_string& toFind, size_type startPos) const {
		auto splitString = split_single_chars(toFind);
		if (splitString.empty())
			return no_pos;
		auto firstPos = npos;
		auto startByte = (startPos == 0) ? string_position{0} : string_function::getByteCountCharLimited(m_string.data(), startPos, true);
		if (!startByte)
			return no_pos;
		bool isFirst = true;
		for (auto& iter : splitString) {
			auto nextPos = isFirst ? m_string.find_first_of(iter.m_string, *startByte) : m_string.find(iter.m_string, *startByte);
			isFirst = false;
			if (nextPos < firstPos) {
				firstPos = nextPos;
				if (firstPos == 0)
					return 0;
			}
		}
		return (firstPos == npos) ? no_pos : string_function::getCharacterCount(data(), firstPos);
	} //basic_string<Alloc>::findFirstOf


	/*--------------------------------------------------------------------
		Find the first character not in a specified string

		toFind: A string of characters to not find
		startPos: The character to begin searching from

		return: The index of the first non-matching character (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::findFirstNotOf(const basic_string& toFind, size_type startPos) const {
		auto splitString = split_single_chars(toFind);
		if (splitString.empty())
			return no_pos;
		auto charBytes = string_function::collectCharByteCount(data());
		if (charBytes.empty())
			return no_pos;
		std::vector<size_type> minPos(splitString.size());
		size_type startByte = 0;
		for (int sizeIndex = 0; sizeIndex < charBytes.size(); ++sizeIndex) {
			auto size = charBytes[sizeIndex];
			if (sizeIndex < startPos) {
				startByte += size;
				continue;
			}
			size_type lowest = npos;
			for (int charIndex = 0; charIndex < splitString.size(); ++charIndex) {
				if (minPos[charIndex] > startByte)
					continue;
				minPos[charIndex] = (charIndex == 0) ?
						m_string.find_first_of((splitString)[charIndex].m_string, startByte) :
						m_string.find((splitString)[charIndex].m_string, startByte);
				if (minPos[charIndex] < lowest)
					lowest = minPos[charIndex];
			}
			if (lowest > startByte)
				break;
			startByte += size;
			++startPos;
		}
		return startPos < length() ? string_position{startPos} : no_pos;
	} //basic_string<Alloc>::findFirstNotOf


	/*--------------------------------------------------------------------
		Find the last character in this string which is in the specified string

		toFind: A string of characters to find
		lastPos: The position of the last character to be compared

		return: The index of the last matching character (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::findLastOf(const basic_string& toFind, string_position lastPos) const {
		std::u32string searchPattern{toFind};
		return rfindIf([&searchPattern](char32_t testChar) -> bool {
			return searchPattern.find(testChar) != std::u32string::npos;
		});
	} //basic_string<Alloc>::findLastOf


	/*--------------------------------------------------------------------
		Find the last character in this string which is not in the specified string

		toFind: A string of characters not to find
		lastPos: The character to begin searching from (nullopt for string end)

		return: The index of the last non-matching character (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::findLastNotOf(const basic_string& toFind, string_position lastPos) const {
		std::u32string searchPattern{toFind};
		return rfindIf([&searchPattern](char32_t testChar) -> bool {
			return searchPattern.find(testChar) == std::u32string::npos;
		});
	} //basic_string<Alloc>::findLastNotOf


	/*--------------------------------------------------------------------
		Find the specified string searching backwards

		toFind: The string to find
		lastPos: The position of the last character to be compared

		return: The index of a matching string (nullopt = not found)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	string_position basic_string<Alloc>::rfind(const basic_string& toFind, string_position lastPos) const {
		auto endChar = string_function::getByteCountCharLimited(data(), lastPos, true);
		if (!endChar)
			return no_pos;
		auto foundPos = m_string.rfind(toFind.m_string, *endChar);
		return (foundPos == npos) ? no_pos : string_function::getCharacterCount(data(), foundPos);
	} //basic_string<Alloc>::rfind

	//MARK: - Functions (mutating)

	/*--------------------------------------------------------------------
		Reserve the specified number of bytes for the string to grow into

		newSize: The number of bytes to reserve
	  --------------------------------------------------------------------*/
	template <class Alloc>
	void basic_string<Alloc>::reserve(size_type newSize) {
		m_string.reserve(newSize);
	} //basic_string<Alloc>::reserve


	/*--------------------------------------------------------------------
		Resize the string and (when grown) pad with the specified character
	 
		newSize: The required number of characters
		padding: A padding character (when the sring grows - only the first character is used)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	void basic_string<Alloc>::resize(size_type newSize, const basic_string& padding) {
		if (newSize == 0) {
			clear();
			return;
		}
		auto currentSize = size();
		if (currentSize > newSize) {
			m_string = substr(0, newSize).m_string;
			return;
		}
		auto padChar = (padding.empty()) ? " " : padding.substr(0, 1);
		for (auto index = newSize - currentSize; index--; )
			append(padChar);
	} //basic_string<Alloc>::resize


	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string
	 
		func: The character function (the returned value replaces the input character)
	  --------------------------------------------------------------------*/
	template <class Alloc>
	void basic_string<Alloc>::forEach(const Function& func) {
		std::u32string result;
		auto remaining = dataSize();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = getUTF32CharFromUTF8(text, remaining); consumed != 0) {
				remaining -= consumed;
				if (auto nextChar = func(incoming); nextChar)
					result += *nextChar;
			} else
				break;
		}
		*this = result;
	} //basic_string<Alloc>::forEach


	/*--------------------------------------------------------------------
		Assign a specified string to this
	 
		source: The string to assign
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::assign(const basic_string& source) {
		m_string = source.m_string;
		return *this;
	} //basic_string<Alloc>::assign


	/*--------------------------------------------------------------------
		Assign a specified string to this
	 
		source: The character array to be copied
		byteCount: The maximum number of bytes in the array (nullopt = null-terminated)
		charCount: The maximum number of (encoded) characters to read (nullopt = as byteCount limit)
		format: The source text data format

		return: The number of bytes assigned from the source
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::assign(const char* source, string_position byteCount, string_position charCount, text_format format) {
		m_string.clear();
		return make_string(m_string, source, byteCount, charCount, format);
	} //basic_string<Alloc>::assign


	/*--------------------------------------------------------------------
		Assign a double value to the string (used in special cases where fast conversion is a priority
	 
		value: The string to assign
		decPlaces: The number of decimal places
	 
		return: True if the assignment was successful
	  --------------------------------------------------------------------*/
	template <class Alloc>
	bool basic_string<Alloc>::assign(double value, uint8_t decPlaces) {
#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 13)) || (defined(__clang__) && (__clang_major__ < 15))
		m_string = std::move(basic_string{value, pow(10.0, decPlaces)}.m_string);
#else
		constexpr size_type bufferLen = 40;
		m_string.resize(bufferLen);
		auto result = std::to_chars(m_string.data(), m_string.data() + bufferLen, value, std::chars_format::fixed, decPlaces);
		if (result.ec != std::errc())
			return false;
		*result.ptr = '\0';
		m_string.resize(result.ptr - m_string.data());
#endif
		return true;
	} //basic_string<Alloc>::assign


	/*--------------------------------------------------------------------
		Append the specified string to this

		source: The string to append

		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(const basic_string& source) {
		m_string.append(source.m_string);
		return *this;
	} //basic_string<Alloc>::append

	
	/*--------------------------------------------------------------------
		Append the specified char to this (NB: don't use this casually - encoding must be assumed and converted accordingly)
	 
		source: The char to append
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(char source) {
		if (auto uniChar = string_function::getUnicodeChar(&source, 1, ISO8859_1); uniChar.second > 0)
			append(uniChar.first);
		return *this;
	} //basic_string<Alloc>::append
	

	/*--------------------------------------------------------------------
		Insert a string into this
		
		pos: The insertion point
		source: The string to insert
		start: The start point in the source string
		howMany: The number of chars to insert (nullopt inserts all)
		
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::insert(size_type pos, const basic_string& source, size_type start, string_position howMany) {
		return replace(pos, 0, source, start, howMany);
	} //basic_string<Alloc>::insert


	/*--------------------------------------------------------------------
		Replace a specified string segment with another string
		
		pos: The position to begin replacing
		num: The number of chars to replace
		source: The replacement string
		start: The start point in the replacement string
		howMany: The number of chars to extract from the replacement string (nullopt inserts all)
		
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replace(string_position pos, string_position num, const basic_string& source,
													  size_type start, string_position howMany) {
		basic_string toAppend(source.substr(start, howMany));
		size_type myLength = length();
		if (!pos || (pos >= myLength))
			return append(toAppend);
		if (num && ((*pos + *num) < myLength))
			toAppend += substr(*pos + *num);
		resize(*pos);
		return append(toAppend);
	} //basic_string<Alloc>::replace


	/*--------------------------------------------------------------------
		Replace all instances of a specified expression
	 
		toFind: The expression to be replaced
		replacement: The replacement expression
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replaceAll(const basic_string& toFind, const basic_string& replacement) {
		std::u32string searchPattern{toFind}, substitute{replacement}, result;
		bool found = false;
		std::u32string::size_type matched = 0;
		const auto searchSize = searchPattern.size();
		auto remaining = dataSize();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = string_function::getUTF32CharFromUTF8(text, remaining); consumed != 0) {
				remaining -= consumed;
				result += incoming;
				if (searchPattern[matched] == incoming) {
					++matched;
					if (matched != searchSize)
						continue;
					found = true;
					result.resize(result.size() - searchSize);
					result += substitute;
				}
				matched = 0;
			} else
				break;
		}
		if (found)
			*this = result;
		return *this;
	} //basic_string<Alloc>::replaceAll


	/*--------------------------------------------------------------------
		Replace all chars passing a specified filter
	 
		filter: The filter for chars to be replaced
		replacement: The replacement expression
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replaceIf(const Filter& filter, const basic_string& replacement) {
		std::u32string substitute{replacement}, result;
		bool found = false;
		auto remaining = dataSize();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = string_function::getUTF32CharFromUTF8(text, remaining); consumed != 0) {
				remaining -= consumed;
				if (filter(incoming)) {
					found = true;
					result += substitute;
				} else
					result += incoming;
			} else
				break;
		}
		if (found)
			*this = result;
		return *this;
	} //basic_string<Alloc>::replaceAll


	/*--------------------------------------------------------------------
		Replace any instances of specified characters
	 
		charsToFind: The characters to be replaced (NB: each character is treated independently - the order does not matter)
		replacement: The replacement expression
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replaceAnyOf(const basic_string& charsToFind, const basic_string& replacement) {
		std::u32string searchPattern{charsToFind}, substitute{replacement}, result;
		auto filter = [&searchPattern](char32_t testChar) -> bool {
			return searchPattern.find(testChar) != npos;
		};
		bool found = false;
		auto remaining = dataSize();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = string_function::getUTF32CharFromUTF8(text, remaining); consumed != 0) {
				remaining -= consumed;
				if (filter(incoming)) {
					found = true;
					result += substitute;
				} else
					result += incoming;
			} else
				break;
		}
		if (found)
			*this = result;
		return *this;
	} //basic_string<Alloc>::replaceAnyOf


	/*--------------------------------------------------------------------
		Erase a specified range of characters from a string
		
		pos: The position to erasing from
		howMany: The number of characters to erase (nullopt to erase all)
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::erase(size_type pos, string_position howMany) {
		return replace(pos, howMany, basic_string{});
	} //basic_string<Alloc>::erase


	/*--------------------------------------------------------------------
		Remove the last character from the string
	 --------------------------------------------------------------------*/
	template <class Alloc>
	void basic_string<Alloc>::popBack() {
		if (!empty())
			erase(length() - 1);
	} //basic_string<Alloc>::pop_back


	/*--------------------------------------------------------------------
		Pad the string with a repeated character to reach a specified length (so the existing content is flush with the right)
	 
		length: The required string length
		repeat: The character to repeatedly insert until the length is met
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::padRight(size_type length, const basic_string& repeat) {
		if ((length <= size()) || repeat.empty())
			return *this;
		auto required = length - size(),
				fillSize = repeat.size();
		auto toInsert = required / fillSize;
		if (toInsert > 0)
			insert(0, basic_string(toInsert, repeat.m_string));
		return *this;
	} //basic_string<Alloc>::padRight


	/*--------------------------------------------------------------------
		Pad the string with a repeated character to reach a specified length (so the existing content is flush with the left)
	 
		length: The required string length
		repeat: The character to repeatedly append until the length is met
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <class Alloc>
	basic_string<Alloc>& basic_string<Alloc>::padLeft(size_type length, const basic_string& repeat) {
		if ((length <= size()) || repeat.empty())
			return *this;
		auto required = size() - length,
				fillSize = repeat.size();
		auto toAppend = required / fillSize;
		if (toAppend > 0)
			append(basic_string(toAppend, repeat.m_string));
		return *this;
	} //basic_string<Alloc>::padLeft
	
}  // namespace active

	///Hashing for `basic_string`, e.g. to use as a key in `unordered_map`
template <class Alloc>
struct std::hash<active::basic_string<Alloc>> {
	size_t operator()(const active::basic_string<Alloc>& k) const {
		return hash<std::string>()(k);	//Just use the hashing provided by std::string
	}
};

#endif	//ACTIVE_STRING
