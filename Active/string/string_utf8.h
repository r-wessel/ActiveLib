/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_STRING
#define ACTIVE_STRING

#include "Active/string/string_size.h"
#include "Active/string/text_format.h"

#include <functional>
#include <memory>
#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 13)) || (defined(__clang__) && (__clang_major__ < 15))
#include <sstream>
#else
#include <format>
#endif
#include <set>

namespace active {
		
	/*!
	 A Unicode-aware string class
	 
	 The `active::basic_string` template class is a wrapper for `std::string` and leans heavily on existing functionality it provides, extending
	 it with awareness of Unicode encodings for character positioning. This also provides easy access to the underlying `std::string`
	 for easy interoperability with any code working with that type.
	 
	 String content is internally encoded/validated as UTF-8, but it can encode/decode to/from UTF16, UTF32, ASCII and ISO8859.
	 All content must be valid UTF-8, i.e. you cannot embed arbitrary binary data or null characters in a `string`.
	 Character positions are calculated to allow indexing, but the time to find a position is O(n)
	 It is recommended to use classes like `BufferIn` to analyse by-character content on large blocks of text efficiently
	 
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
		using size_type = string_size::size_type;
			///Unary predicate for filtering strings
		using Filter = std::function<bool(char32_t)>;
			///Unary functions for processing string characters
		using Function = std::function<std::optional<char32_t>(char32_t)>;
		
			///Character referencing type
		struct char_t {
				///Constructors
			constexpr char_t() {}
			constexpr char_t(const char_t& source) : m_string{source.m_string}, m_position{source.m_position} {}
				///Conversion operator
			constexpr operator char32_t() const {
				if (m_string == nullptr) [[unlikely]]
					throw std::out_of_range("");
				return m_string->at(m_position);
			}
				///Assignment operators
			constexpr char_t& operator=(char32_t source) {
				if (m_string == nullptr) [[unlikely]]
					throw std::out_of_range("");
				if (basic_string replacement{&source, 1}; !replacement.empty()) [[likely]]
					m_string->replace(m_position, 1, basic_string{&source, 1});
				else
					throw std::out_of_range("");
				return *this;
			}
			constexpr char_t& operator=(const char_t& source) {
				if (this != &source) { [[likely]]
					m_string = source.m_string;
					m_position = source.m_position;
				}
				return *this;
			}
			constexpr basic_string string() const {
				if (m_string == nullptr) [[unlikely]]
					throw std::out_of_range("");
				return m_string->substr(m_position, 1);
			}

		private:
			friend class basic_string;
			
			char_t(basic_string* string, string_size pos = 0) : m_string{string}, m_position{pos} {}
				///The target string
			basic_string* m_string = nullptr;
				///The char position in the string
			string_size m_position;
		};
		
		/*!
		 Iterator type
		 @tparam Str The string type
		 @tparam Char The character type
		 @tparam Offset The string position increment amount
		 */
		template<typename Str, typename Char, std::ptrdiff_t Offset = 1>
		class iter_t {
		public:
				///Types
			using difference_type = std::ptrdiff_t;
			using value_type = Char;
			
				///Constructors
			constexpr iter_t() {}
			constexpr iter_t(const iter_t& source) : m_string{source.m_string}, m_pos{source.m_pos}, m_size{source.m_size} {}
			
				///Assignment operator
			constexpr iter_t& operator=(const iter_t& other) {
				if (this != &other) [[likely]] {
					m_string = other.m_string;
					m_pos = other.m_pos;
					m_size = other.m_size;
				}
				return *this;
			}
				///Dereference operator
			constexpr const value_type& operator*() const { return get_char(); }
			constexpr value_type& operator*() { return get_char(); }
			constexpr const value_type* operator->() const { return &get_char(); }
			constexpr value_type* operator->() { return &get_char(); }
				///Increment operator
			constexpr iter_t& operator++() {
				m_pos += Offset;
				return *this;
			}
				///Post-increment operator
			constexpr iter_t operator++(int) {
				auto tmp = *this;
				++*this;
				return tmp;
			}
				///Decrement operator
			constexpr iter_t& operator--() {
				m_pos -= Offset;
				return *this;
			}
				///Post-decrement operator
			constexpr iter_t operator--(int) {
				auto tmp = *this;
				--*this;
				return tmp;
			}
				///Equality operator
			constexpr bool operator==(const iter_t& ref) const {
				if (is_end())
					return ref.is_end();
				return (ref.is_end()) ? false : ((m_string == ref.m_string) && (m_pos == ref.m_pos));
			}
				///Equality operator
			constexpr bool operator!=(const iter_t& ref) const {
				return !(*this == ref);
			}
				///Addition with assignment operator
			constexpr iter_t& operator+=(difference_type n) {
				m_pos += (n * Offset);
				return *this;
			}
				///Addition operator
			constexpr iter_t operator+(difference_type n) const {
				auto result = *this;
				result += n;
				return result;
			}
				///Subtraction with assignment operator
			constexpr iter_t& operator-=(difference_type n) {
				m_pos -= (n * Offset);
				return *this;
			}
				///Subtraction operator
			constexpr iter_t operator-(difference_type n) const {
				auto result = *this;
				result -= n;
				return result;
			}
				///Subtraction operator
			constexpr difference_type operator-(const iter_t& other) const { return m_pos - other.m_pos; }
				///Subscript operator
			constexpr value_type& operator[](difference_type n) const {
				auto offset = n * Offset;
				m_pos += offset;
				get_char();
				m_pos -= offset;
				return m_char;
			}
				///Less-than operator
			constexpr bool operator<(const iter_t& other) const {
				if (is_end())
					return false;
				return other.is_end() ? true : m_pos < other.m_pos;
			}
				///Less-than-or-equal operator
			constexpr bool operator<=(const iter_t& other) const { return !(*this > other) ; }
				///Greater-than operator
			constexpr bool operator>(const iter_t& other) const {
				if (other.is_end())
					return false;
				return is_end() ? true : m_pos > other.m_pos;
			}
				///Greater-than-or-equal operator
			constexpr bool operator>=(const iter_t& other) const { return !(*this < other); }
			
		private:
			friend basic_string;
			
				///Constructor
			constexpr iter_t(Str* source, difference_type pos = 0) : m_string{source}, m_pos(pos) {
				if (source != nullptr)
					m_size = source->size();
			}
				///Get the referenced character
			constexpr Char& get_char() const {
				if (is_end())
					throw std::out_of_range("");
				m_char = char_t{const_cast<Str*>(m_string), m_pos};
				return m_char;
			}
				///Determine if the iterator is at end()
			constexpr bool is_end() const { return !m_pos || (m_pos >= m_size); }

				///The target string
			Str* m_string = nullptr;
				///The iterator position in the string
			string_size m_pos;
				///The string size (chars, not bytes)
			string_size m_size;
				///The character currently pointed to
			mutable char_t m_char;
		};
		
			///Iterator
		using iterator = iter_t<basic_string, char_t>;
			///Const iterator
		using const_iterator = iter_t<const basic_string, const char_t>;
			///Reverse iterator
		using reverse_iterator = iter_t<basic_string, char_t, -1>;
			///Reverse const iterator
		using const_reverse_iterator = iter_t<const basic_string, const char_t, -1>;

		//MARK: - Constants
		
			///Constant to indicate an unspecified or non-existant position in std::string
		static constexpr size_type npos = base::npos;
		
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
		 @param howMany The number of 32-bit code points in the text (default = null-terminated)
		 @param isCountRequired True if the specified number of code points must be valid (default = ignore)
		 @return The UTF-8 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
		 */
		static std::optional<basic_string> from_unicode(const char32_t*& text, bool is_big_endian,
				string_size howMany = {}, bool isCountRequired = false);
		/*!
		 Make an STL string from a char array. NB: always returns a result, but result will only include valid chars
		 @param target An STL string to be populated with valid characters found
		 @param text The source text
		 @param howMany The number of bytes in the text (defaulg = null-terminated)
		 @param charCount The maximum number of (encoded) chars to read (default = no limit)
		 @param format The text data format
		 @return The number of source bytes used in the populated string
		 */
		static size_type make_string(base& target, const char* text, string_size howMany = {},
									 string_size charCount = {}, text_format format = {});
		/*!
		 Split the specified text into single and multi byte chars
		 @param source The source text
		 @return An array of single and multi-byte chars from the source string (empty on failure)
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
		 @param howMany The number of bytes to copy (default = null-terminated)
		 @param format The source text data format
		 */
		basic_string(const char* source, string_size howMany = {}, text_format format = {}) {
			make_string(m_string, source, howMany, {}, format);
		}
#ifndef __CLR_VER
		/*!
		 Constructor from an input UTF8 char array
		 @param source The character array to be copied
		 @param howMany The number of bytes to copy (default = null-terminated)
		 */
		basic_string(const char8_t* source, string_size howMany = {}) : basic_string{reinterpret_cast<const char*>(source), howMany, text_format{}} {}
#endif // !__CLR_VER
		/*!
		 Constructor from an input UTF16 char array
		 @param source The character array to be copied
		 @param howMany The number of 16-bit chars to copy (default = null-terminated)
		 */
		basic_string(const char16_t* source, string_size howMany = {}, bool is_big_endian = text_format::defaultEndian) : basic_string{reinterpret_cast<const char*>(source),
			howMany ? string_size{howMany * sizeof(char16_t)} : string_size{}, text_format{UTF16, is_big_endian}} {}
		/*!
		 Constructor from an input UTF32 char array
		 @param source The character array to be copied
		 @param howMany The number of 32-bit chars to copy (default = null-terminated)
		 */
		basic_string(const char32_t* source, string_size howMany = {}, bool is_big_endian = text_format::defaultEndian) : basic_string{reinterpret_cast<const char*>(source),
			howMany ? string_size{howMany * sizeof(char32_t)} : string_size{}, text_format{UTF32, is_big_endian}} {}
		/*!
		 Constructor from an input string
		 @param source The string to be copied
		 @param howMany The number of chars to copy (default = full length)
		 @param format The source text data format
		 */
		basic_string(const base& source, string_size howMany = {}, text_format format = text_format{}) :
		basic_string{source.data(), howMany, format} {}
#ifndef __CLR_VER
		/*!
		 Constructor from an input string
		 @param source The UTF-8 string to be copied
		 @param howMany The number of chars to copy (default = full length)
		 */
		basic_string(const std::u8string& source, string_size howMany = {}) : basic_string(source.data(), howMany) {}
#endif // !__CLR_VER
		/*!
		 Constructor from an input string
		 @param source The UTF-16 string to be copied
		 @param howMany The number of chars to copy (default = full length)
		 */
		basic_string(const std::u16string& source, string_size howMany = {});
		/*!
		 Constructor from an input string
		 @param source The UTF-32 string to be copied
		 @param howMany The number of chars to copy (default = full length)
		 */
		basic_string(const std::u32string& source, string_size howMany = {});
		/*!
		 Constructor with optional text fill expression and number of repeats
		 @param newSize The required number of expression repeats
		 @param fillText The expression to fill the string
		 */
		basic_string(size_type newSize, const basic_string& fillText) {
			m_string = std::string{};
			basic_string<Alloc>::resize(newSize, fillText);
		}
		/*!
		 Constructor from a char
		 @param val A char
		 */
		explicit basic_string(char val) : basic_string{&val, 1, text_encoding::ascii}	{}
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
		operator std::u8string() const { return std::u8string{reinterpret_cast<const char8_t*>(data())}; }
#endif
			///Conversion to std::u16string
		operator std::u16string() const;
			///Conversion to std::u32string
		operator std::u32string() const;
			///Conversion to 16-bit integer
		explicit operator int16_t() const { return to_int16_t().value_or(0); }
			///Conversion to 32-bit integer
		explicit operator int32_t() const { return to_int32_t().value_or(0); }
			///Conversion to 32-bit unsigned integer
		explicit operator uint32_t() const { return to_uint32_t().value_or(0); }
			///Conversion to 64-bit integer
		explicit operator int64_t() const { return to_int64_t().value_or(0); }
			///Conversion to 64-bit unsigned integer
		explicit operator uint64_t() const { return to_uint64_t().value_or(0); }
			///Conversion to float
		explicit operator float() const { return to_float().value_or(0.0); }
			///Conversion to double (0 if conversion impossible)
		explicit operator double() const { return to_double().value_or(0.0); }
		
			//MARK: - Operators
		
		/*!
		 Subscript operator
		 @param index The required character position
		 @return The character at the specified index
		 */
#ifdef ACTIVE_STRING_CHAR_REF
		const char_t& operator[](size_type index) const { return get_char(index); }
#else
		const char_t operator[](size_type index) const { return char_t{const_cast<basic_string*>(this), index}; }
#endif
		/*!
		 Subscript operator
		 @param index The required character position
		 @return The character at the specified index
		 */
#ifdef ACTIVE_STRING_CHAR_REF
		char_t& operator[](size_type index) { return get_char(index); }
#else
		char_t operator[](size_type index) { return char_t{this, index}; }
#endif
		/*!
		 Assignment operator
		 @param source The object to copy
		 @return A reference to this
		 */
		basic_string& operator= (const basic_string& source) {
			if (&source == this)
				return *this;
			return assign(source);
		}
		/*!
		 Move assignment operator
		 @param source The object to move
		 @return A reference to this
		 */
		basic_string& operator= (basic_string&& source) noexcept {
			m_string = std::move(source.m_string);
			return *this;
		}
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
		/*!
		 Addition with assignment operator
		 @param source The char to append
		 @return A reference to this
		 */
		basic_string& operator+=(char32_t source) { return append(source); }
		
			//MARK: - Functions (const)
		
		/*!
		 Return a const iterator pointing to the first character
		 @return A const iterator pointing to the first character
		 */
		const_iterator begin() const { return const_iterator{this, 0}; }
		/*!
		 Return an iterator pointing to the string end
		 @return An iterator pointing to the string end
		 */
		const_iterator end() const { return const_iterator{}; }
		/*!
		 Return a const iterator pointing to the first character
		 @return A const iterator pointing to the first character
		 */
		const_reverse_iterator rbegin() const { return const_reverse_iterator{this, size() - 1}; }
		/*!
		 Return an iterator pointing to the string end
		 @return An iterator pointing to the string end
		 */
		const_reverse_iterator rend() const { return const_reverse_iterator{}; }
		/*!
		 Get the first character in the string
		 @return A reference to the first character in the string
		 */
		const char_t& front() const { return operator[](0); }
		/*!
		 Get the last character in the string
		 @return A reference to the last character in the string
		 */
		const char_t& back() const { return operator[](size() - 1); }
		/*!
		 Return the number of bytes this string can contain
		 @return The number of bytes this string can contain
		 */
		size_type capacity() const { return m_string.capacity(); }
		/*!
		 Get the number of characters in the string
		 @return The number of characters in the string
		 */
		string_size size() const;
		/*!
		 Get the string length (alias for size)
		 @return The number of characters in the string (size)
		 */
		string_size length() const { return size(); }
		/*!
		 Return the number of bytes consumed by this string
		 @param howMany The number of chars to count (default = full length)
		 @return The number of bytes consumed by the string (NB: basic_string is null terminated, but this is not included in the data length)
		 */
		size_type data_size(string_size howMany = {}) const;
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
		char32_t at(string_size index) const;
		/*!
		 Get a specified substring of this string
		 @param startPos The position of the first character
		 @param howMany The number of characters to get (default = full length)
		 @return The requested string segment
		 */
		basic_string substr(string_size startPos = 0, string_size howMany = {}) const;
		/*!
		 Find the specified string within this
		 @param toFind The string to find
		 @param startPos The character to start searching from
		 @return The index where a match is found (npos = not found)
		 */
		string_size find(const basic_string& toFind, string_size startPos = 0) const;
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
		bool starts_with(const basic_string& toFind) const { return m_string.starts_with(toFind.m_string); }
		/*!
		 Determine if this string ends with a substring
		 @param toFind The substring to find
		 @return True if this string ends with the substring
		 */
		bool ends_with(const basic_string& toFind) const { return m_string.ends_with(toFind.m_string); }
		/*!
		 Find the first character which is in a specified string
		 @param toFind A string of characters to find
		 @param startPos The character to begin searching from
		 @return The index of the first matching character (npos = not found)
		 */
		string_size find_first_of(const basic_string& toFind, string_size startPos = 0) const;
		/*!
		 Find the first character not in a specified string
		 @param toFind A string of characters to not find
		 @param startPos The character to begin searching from
		 @return The index of the first non-matching character (npos = not found)
		 */
		string_size find_first_not_of(const basic_string& toFind, string_size startPos = 0) const;
		/*!
		 Find the last character in this string which is in the specified string
		 @param toFind A string of characters to find
		 @param lastPos The position of the last character to be compared (default = full length)
		 @return The index of the last matching character (npos = not found)
		 */
		string_size find_last_of(const basic_string& toFind, string_size lastPos = {}) const;
		/*!
		 Find the last character in this string which is not in the specified string
		 @param toFind A string of characters not to find
		 @param lastPos The position of the last character to be compared (default = full length)
		 @return The index of the last non-matching character (npos = not found)
		 */
		string_size find_last_not_of(const basic_string& toFind, string_size lastPos = {}) const;
		/*!
		 Find the specified string searching backwards
		 @param toFind The string to find
		 @param lastPos The position of the last character to be compared (default = full length)
		 @return The index of a matching string (npos = not found)
		 */
		string_size rfind(const basic_string& toFind, string_size lastPos = {}) const;
		/*!
		 Create an uppercase version of the string
		 @return An uppercase version of the string
		 */
		basic_string to_upper() const;
		/*!
		 Create a lowercase version of the string
		 @return A lowercase version of the string
		 */
		basic_string to_lower() const;
			///Conversion to optional int16_t (nullopt if conversion impossible)
		std::optional<int16_t> to_int16_t() const { try { return std::stoi(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional int32_t (nullopt if conversion impossible)
		std::optional<int32_t> to_int32_t() const { try { return std::stol(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional uint32_t (nullopt if conversion impossible)
		std::optional<uint32_t> to_uint32_t() const { try { return std::stoul(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional int64_t (nullopt if conversion impossible)
		std::optional<int64_t> to_int64_t() const { try { return std::stoll(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional uint64_t (nullopt if conversion impossible)
		std::optional<uint64_t> to_uint64_t() const { try { return std::stoull(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional float (nullopt if conversion impossible)
		std::optional<float> to_float() const { try { return std::stof(m_string); } catch(...) { return std::nullopt; } }
			///Conversion to optional double (nullopt if conversion impossible)
		std::optional<double> to_double() const { try { return std::stod(m_string); } catch(...) { return std::nullopt; } }
		/*!
		 Determine if the string is entirely alphanumeric
		 @param startPos The position to checking from
		 @param howMany The number of characters to check (default = to end)
		 @return True is the string is alphanumeric
		 */
		bool isAlphaNumeric(size_type startPos = 0, string_size howMany = {}) const;
		/*!
		 Determine if the string is entirely letters
		 @param startPos The position to checking from
		 @param howMany The number of characters to check (default = to end)
		 @return True is the string is letters
		 */
		bool is_alpha(size_type startPos = 0, string_size howMany = {}) const;
		/*!
		 Determine if the string is entirely numeric
		 @param startPos The position to checking from
		 @param howMany The number of characters to check (default = to end)
		 @return True is the string is numbers
		 */
		bool is_numeric(size_type startPos = 0, string_size howMany = {}) const;
		/*!
		 Three-way comparison to a reference string
		 @param ref The string to compare this to
		 @return The relationship between this and ref (less, equal, greater)
		 */
		std::strong_ordering compare(const basic_string& ref) const {
			std::u32string myString{*this}, refString{ref};
			return myString <=> refString;
		}
		/*!
		 Find the specified string within this using a filter
		 @param filter The string filter
		 @return The index where a match is found (npos = not found)
		 */
		string_size find_if(const Filter& filter) const;
		/*!
		 Find the specified string within this searching in reverse and using a filter
		 @param filter The string filter
		 @param lastPos The position to begin filtering from
		 @param howMany The number of chars to filter
		 @return The index where a match is found (npos = not found)
		 */
		string_size rfind_if(const Filter& filter, string_size lastPos = {}, string_size howMany = {}) const;
		/*!
		 Apply a function to specified characters in the string
		 @param func The character function (the returned value is ignored)
		 */
		void for_each(const Function& func) const;
		/*!
		 Apply a function to specified characters in the string in reverse order
		 @param func The character function (the returned value is ignored)
		 */
		void rfor_each(const Function& func) const;
		
			//MARK: - Functions (mutating)
		
		/*!
		 Return an iterator pointing to the first character
		 @return An iterator pointing to the first character
		 */
		iterator begin() { return iterator{this, 0}; }
		/*!
		 Return an iterator pointing to the string end
		 @return An iterator pointing to the string end
		 */
		iterator end() { return iterator{}; }
		/*!
		 Return a reverse iterator pointing to the last character
		 @return A reverse iterator pointing to the last character
		 */
		reverse_iterator rbegin() { return reverse_iterator{this, size() - 1}; }
		/*!
		 Return a reverse iterator pointing to the string end
		 @return A reverse iterator pointing to the string end
		 */
		reverse_iterator rend() { reverse_iterator iterator{}; }
		/*!
		 Get the first character in the string
		 @return A reference to the first character in the string
		 */
		char_t& front() { return operator[](0); }
		/*!
		 Get the last character in the string
		 @return A reference to the last character in the string
		 */
		char_t& back() { return operator[](size() - 1); }
		/*!
		 Reserve the specified number of bytes for the string to grow into
		 @param newSize The number of bytes to reserve
		 */
		void reserve(size_type newSize) { m_string.reserve(newSize); }
		/*!
		 Resize the string and (when grown) pad with the specified character
		 @param newSize The required number of characters
		 @param padding A padding character (when the sring grows - only the first character is used)
		 */
		void resize(string_size newSize, const basic_string& padding = "");
		/*!
		 Clear the string contents
		 */
		void clear() { m_string.clear(); }
		/*!
		 Assign a specified string to this
		 @param source The string to assign
		 @return A reference to this
		 */
		basic_string& assign(const basic_string& source) {
			m_string = source.m_string;
			return *this;
		}
		/*!
		 Assign a specified string to this
		 @param source The character array to be copied
		 @param byteCount The maximum number of bytes in the array (default = null-terminated)
		 @param charCount The maximum number of (encoded) characters to read (default = as byteCount limit)
		 @param format The source text data format
		 @return The number of bytes assigned from the source
		 */
		size_type assign(const char* source, string_size byteCount = {},
						 string_size charCount = {}, text_format format = {}) {
			m_string.clear();
			return make_string(m_string, source, byteCount, charCount, format);
		}
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
		basic_string& append(const basic_string& source) {
			m_string.append(source.m_string);
			return *this;
		}
		/*!
		 Append the specified string to this
		 @param source The string to append
		 @param howMany The number of bytes to append (default = null terminated)
		 @return A reference to this
		 */
		basic_string& append(const char* source, string_size howMany = {});
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
			append_unicode(source);
			return *this;
		}
		/*!
		 Insert a string into this
		 @param pos The insertion point
		 @param source The string to insert
		 @param start The start point in the source string
		 @param howMany The number of chars to insert (default inserts all)
		 @return A reference to this
		 */
		basic_string& insert(string_size pos, const basic_string& source, string_size start = 0, string_size howMany = {}) {
			return replace(pos, 0, source, start, howMany);
		}
		/*!
		 Replace a specified string segment with another string
		 @param pos The position to begin replacing (npos = append to end)
		 @param num The number of chars to replace (npos = to the string end)
		 @param source The replacement string
		 @param start The start point in the replacement string
		 @param howMany The number of chars to extract from the replacement string (default inserts all)
		 @return A reference to this
		 */
		basic_string& replace(string_size pos, string_size num, const basic_string& source,
							  string_size start = 0, string_size howMany = {});
		/*!
		 Erase a specified range of characters from a string
		 @param pos The position to erasing from
		 @param howMany The number of characters to erase (default = to erase to end)
		 @return A reference to this
		 */
		basic_string& erase(string_size pos = 0, string_size howMany = {}) { return replace(pos, howMany, basic_string{}); }
		/*!
		 Remove the last character from the string
		 */
		void pop_back() { if (!empty()) erase(length() - 1); }
		/*!
		 Pad the string with a repeated character to reach a specified length (so the existing content is flush with the right)
		 @param length The required string length
		 @param repeat The character to repeatedly insert util the length is met
		 @return A reference to this
		 */
		basic_string& pad_right(string_size length, const basic_string& repeat = " ");
		/*!
		 Pad the string with a repeated character to reach a specified length (so the existing content is flush with the left)
		 @param length The required string length
		 @param repeat The character to repeatedly append util the length is met
		 @return A reference to this
		 */
		basic_string& pad_left(string_size length, const basic_string& repeat = " ");
		/*!
		 Replace all instances of a specified expression
		 @param toFind The expression to be replaced
		 @param replacement The replacement expression
		 @return A reference to this
		 */
		basic_string& replace_all(const basic_string& toFind, const basic_string& replacement = basic_string());
		/*!
		 Replace all chars passing a specified filter
		 @param filter The filter for chars to be replaced
		 @param replacement The replacement expression
		 @return A reference to this
		 */
		basic_string& replace_if(const Filter& filter, const basic_string& replacement = basic_string());
		/*!
		 Replace any instances of specified characters
		 @param charsToFind The characters to be replaced (NB: each character is treated independently - the order does not matter)
		 @param replacement The replacement expression
		 @return A reference to this
		 */
		basic_string& replace_any_of(const basic_string& charsToFind, const basic_string& replacement = basic_string());
		/*!
		 Apply a function to specified characters in the string
		 @param func The character function (the returned value replaces the input character)
		 */
		void for_each(const Function& func);
		/*!
		 Apply a function to specified characters in the string in reverse order
		 @param func The character function (the returned value replaces the input character)
		 */
		void rfor_each(const Function& func);
		
	private:
		/*!
		 Append a unicode character (UTF32) to a utf8 string
		 @param code The unicode char to append
		 @return True if no errors occurred
		 */
		bool append_unicode(char32_t code);

#ifdef ACTIVE_STRING_CHAR_REF
		/*!
		 Get a reference to the character at a specified position
		 @param pos The character position
		 @return A reference to the requested character
		 */
		char_t& get_char(size_type pos) const {
			m_char = char_t{const_cast<basic_string*>(this), pos};
			return m_char;
		}
#endif // ACTIVE_STRING_CHAR_REF
		
			///The string content - NB: The STL representation works for many UTF-8 functions, and this class manages the remainder
		base m_string;
#ifdef ACTIVE_STRING_CHAR_REF
			///A placeholder for referencing characters within the string
		mutable char_t m_char;
#endif
	};

	using string = basic_string<>;

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
	std::string::size_type get_valid_byte_count(const char* text, string_size howMany = {},
												string_size charCount = {}, text_format format = {});
	/*!
	 Get the width of a specified UTF8 character in bytes
	 @param text The source text
	 @param howMany The maximum extent in bytes
	 @return The character width in bytes (npos for bad encoding)
	 */
	std::optional<unsigned char> get_UTF8_character_byte_count(const char* text, string_size howMany = {});
	/*!
	 Get the width of a specified character in bytes
	 @param text The source text
	 @param howMany The maximum extent in bytes
	 @param format The text data format
	 @return The character width in bytes (npos for bad encoding)
	 */
	std::optional<unsigned char> get_character_byte_count(const char* text, string_size howMany = {}, text_format format = {});
	/*!
	 Get the width of the previous character in bytes
	 @param text The source text (pointing to the current character)
	 @param howMany The maximum extent in bytes (e.g. to the string start)
	 @param format The text data format
	 @return The width of the previous character in bytes
	 */
	std::optional<unsigned char> get_prev_char_byte_count(const char* text, string_size howMany = {}, text_format format = {});
	/*!
	 Return the length of a string in bytes, limited by a character count
	 @param text The source text
	 @param howMany The number of characters to count (default = null-terminated)
	 @param isCountRequired True if the number of characters must exist in the text (unless howMany = npos)
	 @param format The text data format
	 @return The number of bytes in the char array (npos if isCountRequired and howMany not reached)
	 */
	string_size get_byte_count_char_limited(const char* text, string_size howMany = {},
												bool isCountRequired = false, text_format format = {});
	/*!
	 Get the number of valid characters found at a specified address
	 @param text The source text
	 @param format The text data format
	 @param howMany The number of bytes in the array (default = null-terminated)
	 @return The nummber of characters found (npos if bad encoding found)
	 */
	string_size get_character_count(const char* text, string_size howMany = {}, text_format format = {});
	/*!
	 Calculate a required number of characters base on a specified byte count, word size and (optional) character limit
	 @param wordSize The character word size, e.g. UTF16 = 2, UTF32 = 4
	 @param howMany The number of bytes (default = null-terminated)
	 @param charCount The maximum number of (encoded) chars to read (npos = no limit)
	 @return The required number of characters
	 */
	string_size get_char_count(unsigned char wordSize, string_size howMany = {}, string_size charCount = {});
	/*!
	 Determine the the number of characters in a char array
	 @param text The source text
	 @param howMany The number of bytes in the text (default = null-terminated)
	 @param format The text data format
	 @return The number of characters in the array
	 */
	std::string::size_type get_string_length(const char* text, string_size howMany = {},
											 text_format format = {});
	/*!
	 Determine if a 16-bit (UTF16) character code is within the BMP
	 @param code The character code
	 @return True if a 16-bit (UTF16) character code is within the BMP
	 */
	inline bool is_within_bmp(char16_t code) { return ((code < 0xD800) || (code >= 0xE000)); }
	/*!
	 Determine if a 32-bit (UTF32) character code is valid
	 @param uniChar The character code
	 @return True if a 32-bit (UTF32) character code is valid
	 */
	inline bool is_valid_unicode(char32_t uniChar) { return ((uniChar <= 0x10FFFF) && ((uniChar > 0xDFFF) || (uniChar < 0xD800))); }
	/*!
	 Get a UTF-32 char from a UTF-8 source
	 @param text The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	 @param howMany The number of available bytes in the source (default = null-terminated)
	 @return A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
	 */
	std::pair<char32_t, unsigned char> get_utf32_char_from_utf8(const char*& text, string_size howMany = {});
	/*!
	 Get a UTF-32 char from a UTF-16 source
	 @param text The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	 @param is_big_endian True if byte ordering is big-endian
	 @param howMany The number of available bytes in the source
	 @return A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
	 */
	std::pair<char32_t, unsigned char> get_utf32_char_from_utf16(const char16_t*& text, bool is_big_endian, string_size howMany);
	/*!
	 Get a unicode (UTF-32) character from a specified source
	 @param text The source text
	 @param howMany The number of bytes in the text
	 @param format The source data format
	 @return The unicode char paired with the number of bytes consumed from the source (0 = no valid char found)
	 */
	std::pair<char32_t, unsigned char> get_unicode_char(const char* text, string_size howMany = {}, text_format format = {});
	/*!
	 Get a UTF-32 string from a UTF-8 source
	 @param text The source UTF-8 text (advances to the byte beyond the last counted character)
	 @param howMany The number of bytes in the text (default = null-terminated)
	 @param isCountRequired True if the specified number of bytes must be valid (ignored if howMany = npos)
	 @return The unicode code point for the specified chars (npos on failure)
	 */
	std::optional<std::u32string> to_unicode(const char*& text, string_size howMany = {}, bool isCountRequired = false);
	/*!
	 Get a UTF-32 string from a UTF-16 (16-bit) source
	 @param text The source text (when valid, points to the next byte beyond the found character on return)
	 @param is_big_endian True if byte ordering is big-endian
	 @param howMany The number of words (16-bit values) in the text (default = null-terminated)
	 @param isCountRequired True if the specified number of words must be valid (ignored if howMany = npos)
	 @return The UTF-32 string read from the UTF-16 source (npos on error, including failure to meet isCountRequired condition)
	 */
	std::optional<std::u32string> from_utf16(const char16_t*& text, bool is_big_endian,
											string_size howMany = {}, bool isCountRequired = false);
	/*!
	 Get a UTF-16 string from a UTF-32 source
	 @param text The source text (when valid, points to the next byte beyond the found character on return)
	 @param howMany The number of code points in the text (default = null-terminated)
	 @param isCountRequired True if the specified number of code points must be valid (ignored if howMany = npos)
	 @return The UTF-16 string read from the UTF-32 source (npos on error, including failure to meet isCountRequired condition)
	 */
	std::optional<std::u16string> to_utf16(const char32_t*& text, string_size howMany = {}, bool isCountRequired = false);
	/*!
	 Collect the byte size of each (valid) character from a string into an array
	 @param text The source text
	 @param howMany The number of characters to collect, taken as a maximum rather than a requirement (default = null-terminated)
	 @param format The text data format (collection will stop if a character not matching the encoding is found)
	 @return An array containing the byte size of each character found (npos if no valid chars found)
	 */
	std::vector<unsigned char> collect_char_byte_count(const char* text, string_size howMany = {}, text_format format = {});
	/*!
	 Calculate the byte offsets for a start and number of chars within a string
	 @param text The source text
	 @param startPos The start character
	 @param howMany The number of characters to measure (default = null-terminated)
	 @param isHowManyChars True if the returned second value should be the char count rather than the byte count
	 @return Byte offset to the start character paired with byte/char offset from the start to the end of the last character (npos on failure)
	 */
	std::optional<std::pair<string::size_type, string::size_type>> get_byte_offsets(const char* text, string_size startPos = 0,
																					string_size howMany = {},
																					bool isHowManyChars = false);
	
}

namespace active {
		
	/*!
	 Equality operator
	 @param str1 The first string
	 @param str2 The second string
	 @return True if the strings are identical
	 */
	template <typename Alloc>
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

	template <typename Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::string& str2) {
		return str1 == basic_string{str2};
	}

	template <typename Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::u8string& str2) {
		return str1 == basic_string{str2};
	}

	template <typename Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::u16string& str2) {
		return str1 == basic_string{str2};
	}

	template <typename Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const std::u32string& str2) {
		return str1 == basic_string{str2};
	}

	template <typename Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const char* str2) {
		return str1 == basic_string{str2};
	}

	template <typename Alloc>
	inline bool operator== (const basic_string<Alloc>& str1, const char8_t* str2) {
		return str1 == basic_string{str2};
	}

	
	/*!
	 Inequality operator
	 @param str1 The first string
	 @param str2 The second string
	 @return True if the strings differ
	 */
	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const basic_string<Alloc>& str2) {
		return !(str1 == str2);
	}

	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::string& str2) {
		return !(str1 == basic_string{str2});
	}

	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::u8string& str2) {
		return !(str1 == basic_string{str2});
	}

	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::u16string& str2) {
		return !(str1 == basic_string{str2});
	}

	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const std::u32string& str2) {
		return !(str1 == basic_string{str2});
	}
	
	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const char* str2) {
		return !(str1 == basic_string{str2});
	}

	template <typename Alloc>
	inline bool operator!= (const basic_string<Alloc>& str1, const char8_t* str2) {
		return !(str1 == basic_string{str2});
	}


	/*!
	 Addition operator
	 @param str1 The first string
	 @param str2 The second string
	 @return The concatonated string
	 */
	template <typename Alloc>
	inline basic_string<Alloc> operator+(const basic_string<Alloc>& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <typename Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::u8string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::u16string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const basic_string<Alloc>& str1, const std::u32string& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const std::string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const std::u8string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const std::u16string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline bool operator+ (const std::u32string& str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(basic_string<Alloc>{str2});
	}

	template <typename Alloc>
	inline basic_string<Alloc> operator+(const basic_string<Alloc>& str1, const char* str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <typename Alloc>
	inline basic_string<Alloc> operator+(const basic_string<Alloc>& str1, const char8_t* str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <typename Alloc>
	inline basic_string<Alloc> operator+(const char* str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}

	template <typename Alloc>
	inline basic_string<Alloc> operator+(const char8_t* str1, const basic_string<Alloc>& str2) {
		return basic_string<Alloc>{str1}.append(str2);
	}
	
	template <typename Alloc>
	inline std::ostream& operator<<(std::ostream& left, const basic_string<Alloc>& right) {
		left << right.data();
		return left;
	}
	
	template <typename Alloc = std::allocator<char>>
	inline std::ostream& operator<<(std::ostream& left, const typename basic_string<Alloc>::char_t& right) {
		left << right.string().data();
		return left;
	}
	
	/*!
		Determine if a specified char is white-space
		@param uniChar The char to test
		@return True if a specified char is white-space
	*/
	inline bool is_white_space(char32_t uniChar) {
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
	inline bool is_line_ending(char32_t uniChar) {
		return ((uniChar == U'\r') || (uniChar == U'\n'));
	}
	
	
	/*!
		Determine if a specified char is numeric (int)
		@param uniChar The char to test
		@return True if a specified char is numeric
	*/
	inline bool is_digit(char32_t uniChar) {
		return ((uniChar <= U'9') && (uniChar >= U'0'));
	}
	
	
	/*!
		Determine if a specified char is numeric (int)
		@param uniChar The char to test
		@return True if a specified char is numeric
	*/
	inline bool is_numeric(char32_t uniChar) {
		return (is_digit(uniChar) || (uniChar == U'-'));
	}
	
	
	/*!
		Determine if a specified char is numeric (int)
		@param uniChar The char to test
		@return True if a specified char is numeric
	*/
	inline bool is_float(char32_t uniChar) {
		return (is_numeric(uniChar) || (uniChar == U'.'));
	}

	// MARK: - Constructors


	/*--------------------------------------------------------------------
		Constructor from an input string
	 
		source: The UTF-16 string to be copied
		howMany: The number of chars to copy (default = full length)
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>::basic_string(const std::u16string& source, string_size howMany) {
		const char16_t* text = source.data();
			//First convert to UTF-32
		if (auto string32 = string_function::from_utf16(text, text_format::defaultEndian, howMany); string32) {
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
		howMany: The number of chars to copy (default = full length)
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>::basic_string(const std::u32string& source, string_size howMany) {
		const char32_t* text = source.data();
			//NB: We are assuming that the incoming u32string byte ordering is the platform default
		if (auto uniString = from_unicode(text, text_format::defaultEndian, howMany); uniString)
			m_string = uniString->m_string;
	} //basic_string<Alloc>::basic_string


	/*--------------------------------------------------------------------
		Constructor from double
	 
		val: A double value
		prec: The required precision
		padZero: True to pad the number to the specified precision with zeros
	  --------------------------------------------------------------------*/
	template <typename Alloc>
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
		howMany: The number of 32-bit code points in the text (default = null-terminated)
		isCountRequired: True if the specified number of  code points must be valid (ignored if howMany = npos)
	 
		return: The UTF-8 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	std::optional<basic_string<Alloc>> basic_string<Alloc>::from_unicode(const char32_t*& text, bool is_big_endian,
																		 string_size howMany, bool isCountRequired) {
		if (howMany == 0) [[unlikely]]
			return std::optional(basic_string{});	//An empty string is not an error, so we don't return npos
		basic_string result;
		for ( ; *text != 0; ++text) {
			if (howMany) {
				if (howMany == 0)
					break;
				howMany -= 1;
			}
			if (!result.append_unicode(*text)) [[unlikely]]
				return std::nullopt;	//Bad encoding
		}
		return (isCountRequired && (howMany > 0)) ? std::nullopt : std::optional(result);
	} //basic_string<Alloc>::from_unicode

	
	/*--------------------------------------------------------------------
		Make an STL string from a char array. NB: always returns a result, but result will only include valid chars
		
		target: An STL string to be populated with valid characters found
		text: The source text
		howMany: The number of bytes in the text (default = null-terminated)
		charCount: The maximum number of (encoded) chars to read (npos = no limit)
		format: The text data format
		
		return: The number of source bytes used in the populated string
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::make_string(base& target, const char* text, string_size howMany,
													 string_size charCount, text_format format) {
		target.clear();
		if (text == nullptr) [[unlikely]]
			return 0;
		size_type data_size = 0;
		switch (format.encoding) {
			case UTF8: [[likely]] case ascii: {
				data_size = string_function::get_valid_byte_count(text, howMany, charCount, format.encoding);
				if (data_size < 1)
					break;
					//Any valid bytes can be consumed directly by std::string constructor
				target.assign(text, data_size);
				break;
			}
			case UTF16: {
				const auto* source = reinterpret_cast<const char16_t*>(text);
				if (auto string32 = string_function::from_utf16(source, format.is_big_endian,
															   string_function::get_char_count(sizeof(char16_t), howMany, charCount)); string32) {
					const char32_t* source32 = string32->data();
					if (auto uniString = from_unicode(source32, format.is_big_endian); uniString) {
						target = *uniString;
						data_size = target.size() * sizeof(char16_t);
					}
				}
				break;
			}
			case UTF32: [[unlikely]] {
				const auto* source = reinterpret_cast<const char32_t*>(text);
				if (auto uniString = from_unicode(source, format.is_big_endian,
												  string_function::get_char_count(sizeof(char32_t),howMany, charCount)); uniString) {
					target = *uniString;
					data_size = target.size() * sizeof(char32_t);
				}
				break;
			}
			case ISO8859_1: [[unlikely]]
				data_size = string_function::get_valid_byte_count(text, howMany, charCount, format);
				if (data_size < 1)
					break;
				const auto* source = reinterpret_cast<const unsigned char*>(text);
				for ( ; data_size--; ++source) {
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
		return data_size;
	} //basic_string<Alloc>::make_string
	
	
	/*--------------------------------------------------------------------
		Split the specified text into single and multi byte chars
	 
		source: The source text
	 
		return: An array of single and multi-byte chars from the source string (npos on failure)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	std::vector<basic_string<Alloc>> basic_string<Alloc>::split_single_chars(const basic_string& source) {
		std::vector<basic_string> result;
		auto charBytes = string_function::collect_char_byte_count(source.data());
		if (charBytes.empty()) [[unlikely]]
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

	/*--------------------------------------------------------------------
		Conversion to std::u16string
	 
		return: The equivalent std::u16string (UTF-16)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>::operator std::u16string() const {
		const auto* text = data();
		if (auto uniString = string_function::to_unicode(text); uniString) {
			const char32_t* text32 = uniString->data();
			if (auto uniString16 = string_function::to_utf16(text32); uniString16)
				return *uniString16;
		}
		return std::u16string{};
	} //basic_string<Alloc>::operator std::u16string


	/*--------------------------------------------------------------------
		Conversion to std::u32string
	 
		return: The equivalent std::u32string (UTF-32)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>::operator std::u32string() const {
		const auto* text = data();
		auto uniString = string_function::to_unicode(text);
		return uniString ? *uniString : std::u32string{};
	} //basic_string<Alloc>::operator std::u32string

	// MARK: - Operators

	/*--------------------------------------------------------------------
		Assignment operator

		source: A pointer to a char array

		return: A reference to this
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::operator= (const char* source) {
		m_string.clear();
		make_string(m_string, source);
		return *this;
	} //basic_string<Alloc>::operator=

	//MARK: - Functions (const)

	/*--------------------------------------------------------------------
		Get the number of characters in the string
	 
		return: The number of characters in the string
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::size() const {
		return string_function::get_string_length(m_string.data());
	} //basic_string<Alloc>::size


	/*--------------------------------------------------------------------
		Return the number of bytes consumed by this string

		howMany: The number of chars to count (default = for full length)
	 
		return: The number of bytes consumed by the string (NB: Strings are null terminated, but this is not included in the data length)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>::size_type basic_string<Alloc>::data_size(string_size howMany) const {
		if (!howMany) [[unlikely]]
			return static_cast<basic_string<Alloc>::size_type>(m_string.size());
		if (auto byteCount = string_function::get_byte_count_char_limited(m_string.data(), howMany); byteCount)
			return static_cast<size_type>(byteCount);
		return 0;
	} //basic_string<Alloc>::data_size


	/*--------------------------------------------------------------------
		Subscript operator
		
		index: The required character position (unchecked - out of bounds behaviour undefined)
		
		return: The character at the specified index
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	char32_t basic_string<Alloc>::at(string_size index) const {
		auto offsets = string_function::get_byte_offsets(data(), index, 1);
		if (!offsets) [[unlikely]]
			throw std::out_of_range("");
		auto source = data() + offsets->first;
		return string_function::get_utf32_char_from_utf8(source, offsets->second).first;
	} //basic_string<Alloc>::at
	

	/*--------------------------------------------------------------------
		Get a specified segment of this string

		startPos: The first character of the segment
		howMany: The number of characters in the segment (default = full length)

		return: The requested string segment
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc> basic_string<Alloc>::substr(string_size startPos, string_size howMany) const {
		auto offsets = string_function::get_byte_offsets(data(), startPos, howMany);
		if (!offsets) [[unlikely]]
			return {};
		return m_string.substr(offsets->first, offsets->second);
	} //basic_string<Alloc>::substr

	
	/*--------------------------------------------------------------------
		Find the specified string within this

		toFind: The string to find
		startPos: The character to start searching from

		return: The index where a match is found (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::find(const basic_string& toFind, string_size startPos) const {
		auto startByte = (startPos == 0) ? string_size{0} : string_function::get_byte_count_char_limited(m_string.data(), startPos, true);
		if (!startByte) [[unlikely]]
			return {};
		auto foundPos = m_string.find(toFind.m_string, startByte);
		if (foundPos == npos)
			return {};
		return string_function::get_character_count(m_string.data(), foundPos);
	} //basic_string<Alloc>::find
	

	/*--------------------------------------------------------------------
		Find the first character which is in a specified string

		toFind: A string of characters to find
		startPos: The character to begin searching from

		return: The index of the first matching character (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::find_first_of(const basic_string& toFind, string_size startPos) const {
		auto splitString = split_single_chars(toFind);
		if (splitString.empty()) [[unlikely]]
			return {};
		auto firstPos = npos;
		auto startByte = (startPos == 0) ? string_size{0} : string_function::get_byte_count_char_limited(m_string.data(), startPos, true);
		if (!startByte) [[unlikely]]
			return {};
		bool isFirst = true;
		for (auto& iter : splitString) {
			auto nextPos = isFirst ? m_string.find_first_of(iter.m_string, startByte) :
					m_string.find(iter.m_string, startByte);
			isFirst = false;
			if (nextPos < firstPos) {
				firstPos = nextPos;
				if (firstPos == 0)
					return 0;
			}
		}
		return (firstPos == npos) ? string_size{} : string_function::get_character_count(data(), firstPos);
	} //basic_string<Alloc>::find_first_of


	/*--------------------------------------------------------------------
		Find the first character not in a specified string

		toFind: A string of characters to not find
		startPos: The character to begin searching from

		return: The index of the first non-matching character (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::find_first_not_of(const basic_string& toFind, string_size startPos) const {
		auto splitString = split_single_chars(toFind);
		if (splitString.empty()) [[unlikely]]
			return {};
		auto charBytes = string_function::collect_char_byte_count(data());
		if (charBytes.empty()) [[unlikely]]
			return {};
		std::vector<size_type> minPos(splitString.size());
		size_type startByte = 0;
		for (int sizeIndex = 0; sizeIndex < charBytes.size(); ++sizeIndex) {
			auto size = charBytes[sizeIndex];
			if (startPos > sizeIndex) {
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
		return startPos < length() ? string_size{startPos} : string_size{};
	} //basic_string<Alloc>::find_first_not_of


	/*--------------------------------------------------------------------
		Find the last character in this string which is in the specified string

		toFind: A string of characters to find
		lastPos: The position of the last character to be compared

		return: The index of the last matching character (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::find_last_of(const basic_string& toFind, string_size lastPos) const {
		std::u32string searchPattern{toFind};
		return rfind_if([&searchPattern](char32_t testChar) -> bool {
			return searchPattern.find(testChar) != std::u32string::npos;
		});
	} //basic_string<Alloc>::find_last_of


	/*--------------------------------------------------------------------
		Find the last character in this string which is not in the specified string

		toFind: A string of characters not to find
		lastPos: The character to begin searching from (npos for string end)

		return: The index of the last non-matching character (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::find_last_not_of(const basic_string& toFind, string_size lastPos) const {
		std::u32string searchPattern{toFind};
		return rfind_if([&searchPattern](char32_t testChar) -> bool {
			return searchPattern.find(testChar) == std::u32string::npos;
		});
	} //basic_string<Alloc>::find_last_not_of


	/*--------------------------------------------------------------------
		Find the specified string searching backwards

		toFind: The string to find
		lastPos: The position of the last character to be compared

		return: The index of a matching string (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::rfind(const basic_string& toFind, string_size lastPos) const {
		auto endChar = string_function::get_byte_count_char_limited(data(), lastPos, true);
		if (!endChar) [[unlikely]]
			return {};
		auto foundPos = m_string.rfind(toFind.m_string, endChar);
		return (foundPos == npos) ? string_size{} : string_function::get_character_count(data(), foundPos);
	} //basic_string<Alloc>::rfind


	/*--------------------------------------------------------------------
		Create an uppercase version of the string
	 
		return: An uppercase version of the string
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc> basic_string<Alloc>::to_upper() const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty()) [[unlikely]]
			return *this;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!is_within_bmp(*i)) {
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
	template <typename Alloc>
	basic_string<Alloc> basic_string<Alloc>::to_lower() const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty()) [[unlikely]]
			return *this;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!is_within_bmp(*i)) {
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
		howMany: The number of characters to check (default = to end)

		return: True if the string is alphanumeric
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	bool basic_string<Alloc>::isAlphaNumeric(size_type startPos, string_size howMany) const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty()) [[unlikely]]
			return false;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!is_within_bmp(*i))
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
		howMany: The number of characters to check (default = to end)

		return: True if the string is letters
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	bool basic_string<Alloc>::is_alpha(size_type startPos, string_size howMany) const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty()) [[unlikely]]
			return false;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!is_within_bmp(*i))
				return false;
#endif
			if (!std::isalpha(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8")))	//NB: Need to consider if locale is appropriate
				return false;
		}
		return true;
	} //basic_string<Alloc>::is_alpha


	/*--------------------------------------------------------------------
		Determine if the string is entirely numbers
	 
		startPos: The position to checking from
		howMany: The number of characters to check (default = to end)
	 
		return: True if the string is numbers
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	bool basic_string<Alloc>::is_numeric(size_type startPos, string_size howMany) const {
#ifdef WINDOWS
		std::u16string uniString{*this};
#else
		std::u32string uniString{*this};
#endif
		if (uniString.empty()) [[unlikely]]
			return false;
			//NB: This algorithm is simplistic. Might use ICU lib in future. Behaviour on Windows might be wrong due to wchar_t size
		for (auto i = uniString.begin(); i != uniString.end(); ++i) {
#ifdef WINDOWS
			if (!is_within_bmp(*i))
				return false;
#endif
			if (!std::iswdigit(static_cast<wchar_t>(*i)))
				return false;
		}
		return true;
	} //basic_string<Alloc>::is_numeric


	/*--------------------------------------------------------------------
		Find the specified string within this using a filter
	 
		filter: The string filter
	 
		return: The index where a match is found (default = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::find_if(const Filter& filter) const {
		auto remaining = data_size();
		auto text = data();
		for (size_type index = 0; ; ++index) {
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, remaining); consumed != 0) {
				if (filter(incoming))
					return index;
				remaining -= consumed;
			} else
				break;
		}
		return {};
	} // basic_string<Alloc>::find_if

	
	/*--------------------------------------------------------------------
		Find the specified string within this searching in reverse and using a filter
	 
		filter: The string filter
		lastPos: The position to begin filtering from
		howMany: The number of chars to filter
	 
		return: The index where a match is found (npos = not found)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	string_size basic_string<Alloc>::rfind_if(const Filter& filter, string_size lastPos, string_size howMany) const {
		auto charBytes = string_function::collect_char_byte_count(data(), lastPos);
		if (charBytes.empty()) [[unlikely]]
			return {};
			//Add up all the bytes in the string (total number of bytes used by the string)
		auto lastByte = std::reduce(charBytes.begin(), charBytes.end());
		string_size index = charBytes.size();
		string_size minIndex = (howMany && (howMany >= index)) ? index - howMany : string_size{0};
		const char* text = data() + lastByte;
		for (auto iter = charBytes.rbegin(); (iter != charBytes.rend()) && (index-- != minIndex); text -= *iter, ++iter) {
			text -= *iter;
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, *iter); consumed != 0) {
				if (filter(incoming))
					return index;
			} else
				break;
		}
		return {};
	} //basic_string<Alloc>::rfind_if


	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string
	 
		func: The character function (the returned value is ignored)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	void basic_string<Alloc>::for_each(const Function& func) const {
		auto remaining = data_size();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = get_utf32_char_from_utf8(text, remaining); consumed != 0) {
				remaining -= consumed;
				func(incoming);
			} else
				break;
		}
	} //basic_string<Alloc>::for_each

	
	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string in reverse order
	 
		func: The character function (the returned value is ignored)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	void basic_string<Alloc>::rfor_each(const Function& func) const {
		auto charBytes = string_function::collect_char_byte_count(data());
		if (charBytes.empty()) [[unlikely]]
			return;
			//Add up all the bytes in the string (total number of bytes used by the string)
		auto lastByte = std::reduce(charBytes.begin(), charBytes.end());
		const char* text = data() + lastByte;
		for (auto iter = charBytes.rbegin(); iter != charBytes.rend(); text -= *iter, ++iter) {
			text -= *iter;
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, *iter); consumed != 0)
				func(incoming);
			else
				break;
		}
	} //basic_string<Alloc>::rfor_each


	//MARK: - Functions (mutating)

	/*--------------------------------------------------------------------
		Resize the string and (when grown) pad with the specified character
	 
		newSize: The required number of characters
		padding: A padding character (when the sring grows - only the first character is used)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	void basic_string<Alloc>::resize(string_size newSize, const basic_string& padding) {
		if (newSize == 0) { [[unlikely]]
			clear();
			return;
		}
		auto currentSize = size();
		if (currentSize > newSize) {
			m_string = substr(0, newSize).m_string;
			return;
		}
		auto padChar = (padding.empty()) ? " " : padding.substr(0, 1);
		for (auto index = newSize - currentSize; --index; )
			append(padChar);
	} //basic_string<Alloc>::resize


	/*--------------------------------------------------------------------
		Assign a double value to the string (used in special cases where fast conversion is a priority
	 
		value: The string to assign
		decPlaces: The number of decimal places
	 
		return: True if the assignment was successful
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	bool basic_string<Alloc>::assign(double value, uint8_t decPlaces) {
#if (defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 13)) || (defined(__clang__) && (__clang_major__ < 15))
		m_string = std::move(basic_string{value, pow(10.0, decPlaces)}.m_string);
#else
		constexpr size_type bufferLen = 40;
		m_string.resize(bufferLen);
		auto result = std::to_chars(m_string.data(), m_string.data() + bufferLen, value, std::chars_format::fixed, decPlaces);
		if (result.ec != std::errc()) [[unlikely]]
			return false;
		*result.ptr = '\0';
		m_string.resize(result.ptr - m_string.data());
#endif
		return true;
	} //basic_string<Alloc>::assign

	
	/*--------------------------------------------------------------------
		Append the specified string to this
	 
		source: The string to append
		howMany: The number of bytes to append (default = null terminated)
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(const char* source, string_size howMany) {
		auto charBytes = string_function::get_valid_byte_count(source, howMany);
		if (charBytes) [[likely]]
			m_string.append(source, charBytes);
		return *this;
	}
	
	
	/*--------------------------------------------------------------------
		Append the specified char to this (NB: don't use this casually - encoding must be assumed and converted accordingly)
	 
		source: The char to append
	 
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::append(char source) {
		if (auto uniChar = string_function::get_unicode_char(&source, 1, ISO8859_1); uniChar.second > 0)
			append(uniChar.first);
		return *this;
	} //basic_string<Alloc>::append


	/*--------------------------------------------------------------------
		Replace a specified string segment with another string
		
		pos: The position to begin replacing
		num: The number of chars to replace
		source: The replacement string
		start: The start point in the replacement string
		howMany: The number of chars to extract from the replacement string (npos inserts all)
		
		return: A reference to this
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replace(string_size pos, string_size num, const basic_string& source,
													  string_size start, string_size howMany) {
		basic_string toAppend(source.substr(start, howMany));
		string_size myLength = length();
		if (!pos || (pos >= myLength))
			return append(toAppend);
		if (num && ((pos + num) < myLength))
			toAppend += substr(pos + num);
		resize(pos);
		return append(toAppend);
	} //basic_string<Alloc>::replace


	/*--------------------------------------------------------------------
		Pad the string with a repeated character to reach a specified length (so the existing content is flush with the right)
	 
		length: The required string length
		repeat: The character to repeatedly insert until the length is met
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::pad_right(string_size length, const basic_string& repeat) {
		if ((length <= size()) || repeat.empty()) [[unlikely]]
			return *this;
		auto required = length - size(),
				fillSize = repeat.size();
		auto toInsert = static_cast<size_type>(required / fillSize);
		if (toInsert > 0) [[likely]]
			insert(0, basic_string(toInsert, repeat.m_string));
		return *this;
	} //basic_string<Alloc>::pad_right


	/*--------------------------------------------------------------------
		Pad the string with a repeated character to reach a specified length (so the existing content is flush with the left)
	 
		length: The required string length
		repeat: The character to repeatedly append until the length is met
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::pad_left(string_size length, const basic_string& repeat) {
		if ((length <= size()) || repeat.empty()) [[unlikely]]
			return *this;
		auto required = size() - length,
				fillSize = repeat.size();
		auto toAppend = required / fillSize;
		if (toAppend > 0) [[likely]]
			append(basic_string(toAppend, repeat.m_string));
		return *this;
	} //basic_string<Alloc>::pad_left


	/*--------------------------------------------------------------------
		Replace all instances of a specified expression
	 
		toFind: The expression to be replaced
		replacement: The replacement expression
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replace_all(const basic_string& toFind, const basic_string& replacement) {
		std::u32string searchPattern{toFind}, substitute{replacement}, result;
		bool found = false;
		std::u32string::size_type matched = 0;
		const auto searchSize = searchPattern.size();
		auto remaining = data_size();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, remaining); consumed != 0) {
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
	} //basic_string<Alloc>::replace_all


	/*--------------------------------------------------------------------
		Replace all chars passing a specified filter
	 
		filter: The filter for chars to be replaced
		replacement: The replacement expression
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replace_if(const Filter& filter, const basic_string& replacement) {
		std::u32string substitute{replacement}, result;
		bool found = false;
		auto remaining = data_size();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, remaining); consumed != 0) {
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
	} //basic_string<Alloc>::replace_all


	/*--------------------------------------------------------------------
		Replace any instances of specified characters
	 
		charsToFind: The characters to be replaced (NB: each character is treated independently - the order does not matter)
		replacement: The replacement expression
	 
		return: A reference to this
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	basic_string<Alloc>& basic_string<Alloc>::replace_any_of(const basic_string& charsToFind, const basic_string& replacement) {
		std::u32string searchPattern{charsToFind}, substitute{replacement}, result;
		auto filter = [&searchPattern](char32_t testChar) -> bool {
			return searchPattern.find(testChar) != npos;
		};
		bool found = false;
		auto remaining = data_size();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, remaining); consumed != 0) {
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
	} //basic_string<Alloc>::replace_any_of


	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string
	 
		func: The character function (the returned value replaces the input character)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	void basic_string<Alloc>::for_each(const Function& func) {
		std::u32string result;
		auto remaining = data_size();
		auto text = data();
		for (;;) {
			if (auto [incoming, consumed] = get_utf32_char_from_utf8(text, remaining); consumed != 0) {
				remaining -= consumed;
				if (auto nextChar = func(incoming); nextChar)
					result += *nextChar;
			} else
				break;
		}
		*this = result;
	} //basic_string<Alloc>::for_each

	
	/*--------------------------------------------------------------------
		Apply a function to specified characters in the string in reverse order
	 
		func: The character function (the returned value replaces the input character)
	  --------------------------------------------------------------------*/
	template <typename Alloc>
	void basic_string<Alloc>::rfor_each(const Function& func) {
		auto charBytes = string_function::collect_char_byte_count(data());
		if (charBytes.empty()) [[unlikely]]
			return;
			//Add up all the bytes in the string (total number of bytes used by the string)
		std::u32string result;
		result.reserve(charBytes.size());
		auto lastByte = std::reduce(charBytes.begin(), charBytes.end());
		const char* text = data() + lastByte;
		for (auto iter = charBytes.rbegin(); iter != charBytes.rend(); text -= *iter, ++iter) {
			text -= *iter;
			if (auto [incoming, consumed] = string_function::get_utf32_char_from_utf8(text, *iter); consumed != 0) {
				if (auto nextChar = func(incoming); nextChar)
					result += *nextChar;
			} else
				break;
		}
		std::reverse(result.begin(), result.end());
		*this = result;
	} //basic_string<Alloc>::rfor_each
	
	
	/*--------------------------------------------------------------------
	 Append a unicode character (UTF32) to a utf8 string
	 
	 code: The unicode char to append
	 
	 return: True if no errors occurred
	 --------------------------------------------------------------------*/
	template <typename Alloc>
	bool basic_string<Alloc>::append_unicode(char32_t code) {
			//Deal with 7-bit codes first
		if (code < 0x80) { [[likely]]
			m_string += static_cast<char>(code);
			return true;
		}
			//Weed out invalid codes
		if ((code > 0x10FFFF) || ((code >= 0x110000) && (code <= 0x1FFFFF)) || ((code >= 0xD800) && (code <= 0xDFFF))) [[unlikely]]
			return false;	//Bad encoding
		char buffer[4] = {0};
		unsigned char mask = 0x80;
		size_type offset = 3;
		do {
			mask >>= 1;
			mask |= 0x80;
			buffer[offset] = (static_cast<char>(code) & 0x3F) | 0x80;
			--offset;
			code >>= 6;
		} while (code > static_cast<char32_t>((mask ^ 0xFF) >> 1));
		buffer[offset] = static_cast<char>(code) | mask;
		m_string.append(buffer + offset, 4 - offset);
		return true;
	} //basic_string<Alloc>::append_unicode
	
}  // namespace active

	///Hashing for `basic_string`, e.g. to use as a key in `unordered_map`
template <typename Alloc>
struct std::hash<active::basic_string<Alloc>> {
	size_t operator()(const active::basic_string<Alloc>& k) const {
		return hash<std::string>()(k);	//Just use the hashing provided by std::string
	}
};

#endif	//ACTIVE_STRING
