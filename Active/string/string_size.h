/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_STRING_SIZE
#define ACTIVE_STRING_SIZE

#include <numeric>
#include <optional>
#include <string>

namespace active {
		
	/*!
	 String size type
	 
	 Provides numeric safety to prevent common overflow problems converting to/from a string size/position. An exception is thrown if converting
	 to another numeric type will lose data, e.g. assigning a string position of 300 to a char. Also throws if any attempt is made to use or modify
	 a variable set to npos that would offset from npos (or to move a value below zero or above npos)
	 
	 A string size also adopts some behaviours of an optional, treating `string::npos` as equivalent to `std::nullopt`. An explicit bool conversion
	 operator returns false for a value of npos and true for any other value. An comparison to a value of npos returns false. Rather then writing
	 the:
	 
	 	if (auto pos = text.find("."); (pos != npos) && (pos > 4))
	 
	 ...you could write:
	 
	 	if (auto pos = text.find("."); pos > 4)
	 
	 A string size can be assigned std::nullopt (meaning npos). It also provides functions like `value_or`. Note that `string_size` is directly
	 interoperable with `std::string::siez_type`.
	 */
	struct string_size {
	public:
			//MARK: - Types
			///Class size type
		using size_type = std::string::size_type;

			//MARK: - Constants
		
			///Constant to indicate an undefined/missing position in a string
		static constexpr size_type npos = std::string::npos;

			//MARK: - Constructors
		
		/*!
		 Default constructor
		 */
		constexpr string_size() {}
		/*!
		 Constructor
		 @param nullPos An undefined string position
		 */
		constexpr string_size(const std::nullopt_t nullPos) {}
		/*!
		 Constructor
		 @param pos A literal string position
		 */
		template<typename T> requires std::is_arithmetic_v<T>
		constexpr string_size(const T pos) : m_position{static_cast<size_type>(pos)} {
			if constexpr (std::numeric_limits<T>::is_signed) {
				if (pos < 0)
					throw std::out_of_range("");
			}
		}

			//MARK: - Operators
		
		/*!
		 Assignment operator
		 @param source The object to copy
		 @return A reference to this
		 */
		constexpr string_size& operator= (const size_type& source) {
			m_position = source;
			return *this;
		}
		
		/*!
		 Equality operator
		 @param ref The object to compare
		 @return True if this and ref are equal
		 */
		bool operator== (const string_size& ref) const {
			auto state = has_value();
			if (state != ref.has_value())
				return false;
			return !state || (m_position == ref.m_position);
		}
		/*!
		 Equality operator
		 @param ref The object to compare
		 @return True if this and ref are equal
		 */
		bool operator!= (const string_size& ref) const {
			auto state = has_value();
			if (state != ref.has_value())
				return false;
			return !state || (m_position != ref.m_position);
		}
		/*!
		 Less-than operator
		 @param ref The object to compare
		 @return True if this is less than ref
		 */
		bool operator< (const string_size& ref) const { return has_value() && ref.has_value() && (m_position < ref.m_position); }
		bool operator<= (const string_size& ref) const { return has_value() && ref.has_value() && (m_position <= ref.m_position); }
		/*!
		 Greater-than operator
		 @param ref The object to compare
		 @return True if this is greater than ref
		 */
		bool operator> (const string_size& ref) const { return has_value() && ref.has_value() && (m_position > ref.m_position); }
		bool operator>= (const string_size& ref) const { return has_value() && ref.has_value() && (m_position >= ref.m_position); }
		/*!
		 Increment operator
		 @return A reference to this
		 */
		constexpr string_size& operator++() {
			if (!has_value())
				throw std::out_of_range("");
			++m_position;
			return *this;
		}
		constexpr string_size operator++(int) {
			if (!has_value())
				throw std::out_of_range("");
			auto temp = *this;
			++m_position;
			return temp;
		}
		/*!
		 Decrement operator
		 @return A reference to this
		 */
		constexpr string_size& operator--() {
			if (!has_value())
				throw std::out_of_range("");
			--m_position;
			return *this;
		}
		constexpr string_size operator--(int) {
			if (!has_value())
				throw std::out_of_range("");
			auto temp = *this;
			--m_position;
			return temp;
		}
		/*!
		 Addition with assign operator
		 @param addend The amount to add
		 @return A reference to this
		 */
		string_size& operator+=(string_size addend) {
			if (!has_value() || !addend.has_value())
				throw std::out_of_range("");
			string_size diff = npos - m_position;
			if (diff < addend)
				throw std::out_of_range("");
			else if (diff.m_position == 0)
				m_position = npos;
			else
				m_position += addend.m_position;
			return *this;
		}
		/*!
		 Subtract with assign operator
		 @param subtrahend The amount to subtract
		 @return A reference to this
		 */
		string_size& operator-=(string_size subtrahend) {
			if (!has_value() || !subtrahend.has_value())
				throw std::out_of_range("");
			if (m_position < subtrahend.m_position) {
				if (subtrahend.m_position - m_position > 1)
					throw std::out_of_range("");
				else
					m_position = npos;
			} else
				m_position -= subtrahend.m_position;
			return *this;
		}
		/*!
		 Multiplication with assign operator
		 @param multiplier The amount to multiply by
		 @return This multiplied by multiplier
		 */
		string_size& operator*=(string_size multiplier) {
			if (!has_value() || !multiplier.has_value() || ((npos / multiplier.m_position) < m_position))
				throw std::out_of_range("");
			m_position *= multiplier.m_position;
			return *this;
		}
		/*!
		 Division with assign operator
		 @param divisor The amount to divide by
		 @return This divided by divisor
		 */
		string_size& operator/=(string_size divisor) {
			if (!has_value() || !divisor.has_value() || (divisor.m_position == 0))
				throw std::out_of_range("");
			m_position /= divisor.m_position;
			return *this;
		}

			//MARK: - Conversion operators
		
			///Conversion to `bool`
		constexpr explicit operator bool() const { return has_value(); }
			///Conversion to `size_type`
		template<typename T> requires(std::is_same_v<size_type, T>)
		constexpr operator T() const { return m_position; }
			///Conversion to other arithmetic types
		template<typename T> requires(std::is_arithmetic_v<T> && !std::is_same_v<bool, T> && !std::is_same_v<size_type, T>)
		constexpr operator T() const {
			if (m_position > std::numeric_limits<T>::max())
				throw std::out_of_range("");
			return m_position;
		}

			//MARK: - Functions (const)
		
		/*!
		 Determine if the position is defined
		 @return True if the position value is defined
		 */
		constexpr bool has_value() const { return m_position != npos; }
		/*!
		 Return this value when defined, otherwise an alternative value
		 @param alt The alternative value
		 @return Either this (defined) value or the alternative
		 */
		constexpr size_type value_or(size_type alt) const { return has_value() ? m_position : alt; }
		
	private:
		size_type m_position = npos;
	};
	
		// MARK: - string_size operators
	
		//Operator==
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator== (const string_size& left, const T right) { return left == string_size{right}; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator== (const T left, const string_size& right) { return string_size{left} == right; }
		//Operator!=
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator!= (const string_size& left, const T right) { return left != string_size{right}; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator!= (const T left, const string_size& right) { return string_size{left} != right; }
		//Operator+
	constexpr string_size operator+(const string_size& left, const string_size& right) { return string_size{left} += right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator+(const string_size& left, const T& right)  { return string_size{left} += right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator+(const T& left, const string_size& right)  { return string_size{left} += right; }
		//Operator-
	constexpr string_size operator-(const string_size& left, const string_size& right)  { return string_size{left} -= right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator-(const string_size& left, const T& right)  { return string_size{left} -= right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator-(const T& left, const string_size& right)  { return string_size{left} -= right; }
		//Operator*
	constexpr string_size operator*(const string_size& left, const string_size& right) { return string_size{left} *= right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator*(const string_size& left, const T& right) { return string_size{left} *= right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator*(const T& left, const string_size& right) { return string_size{left} /= right; }
		//Operator/
	constexpr string_size operator/(const string_size& left, const string_size& right) { return string_size{left} *= right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator/(const string_size& left, const T& right) { return string_size{left} /= right; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr string_size operator/(const T& left, const string_size& right) { return string_size{left} /= right; }
		//Operator<
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator< (const string_size& left, const T right) { return left < string_size{right}; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator< (const T left, const string_size& right) { return string_size{left} < right; }
	template<typename T> requires std::is_arithmetic_v<T>
		//Operator<=
	constexpr bool operator<= (const string_size& left, const T right) { return left <= string_size{right}; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator<= (const T left, const string_size& right) { return string_size{left} <= right; }
		//Operator>
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator> (const string_size& left, const T right) { return left > string_size{right}; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator> (const T left, const string_size& right) { return string_size{left} > right; }
	template<typename T> requires std::is_arithmetic_v<T>
		//Operator>=
	constexpr bool operator>= (const string_size& left, const T right) { return left >= string_size{right}; }
	template<typename T> requires std::is_arithmetic_v<T>
	constexpr bool operator>= (const T left, const string_size& right) { return string_size{left} >= right; }
	
}  // namespace active

#endif	//ACTIVE_STRING_SIZE
