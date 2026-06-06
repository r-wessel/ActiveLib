/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/string/string_utf8.h"

#include <cwctype>
#include <locale>
#include <optional>
#include <set>
#include <stdexcept>

using namespace active;

//MARK: - Internal implementation

namespace {
	
	using enum text_encoding;

	constexpr std::string::size_type possibleCharWidth = 4;
	
	/*!
		Byte-swap an array of integer values (in the native byte-order)
		@param val A pointer to the array start
		@param howMany How many values are in the array
		@param toBigEndian True if the end result should be big-endian
	*/
	template<typename T> requires (std::is_arithmetic_v<T>)
	static void byte_swap(T* val, std::string::size_type howMany, bool toBigEndian) {
		if ((howMany < 1) || (toBigEndian == text_format::defaultEndian))
			return;
		for (; howMany--; ++val) {
			auto data = reinterpret_cast<unsigned char*>(val);
			auto bytes = sizeof(T);
			for (auto i = bytes-- / 2; i--; )
				std::swap(data[i], data[bytes - i]);
		}
	} //byte_swap

}  // namespace


/*--------------------------------------------------------------------
	Get the number of bytes in the specified text (counting only valid UTF-8 characters)
 
	text: The source text
	howMany: The number of bytes in the text (nullopt = null-terminated)
	charCount: The maximum number of (encoded) chars to seek
	format: The text data format
 
	return: The number of bytes in the text containing valid characters
  --------------------------------------------------------------------*/
std::string::size_type string_function::get_valid_byte_count(const char* text, string_size howMany,
															 string_size charCount, text_format format) {
	bool isOpen = !charCount;
		//Detect empty strings or null requests
	if ((text == nullptr) || (howMany == 0) || (charCount < 1)) [[unlikely]]
		return 0;
	const auto* endPos = text;
	bool isLimited{howMany};
	do {
			//Get the size of the next char
		if (auto nextLen = string_function::get_character_byte_count(endPos, howMany, format); nextLen > 0) { [[likely]]
			endPos += *nextLen;
				//If the source is byte-limited, ensure the remaining count is updated
			if (isLimited) {
				howMany -= *nextLen;
				if (howMany == 0)
					break;
			}
		} else
			break;
	} while ((*endPos != 0) && (isOpen || (--charCount > 0)));
	return endPos - text;
} //string_function::get_valid_byte_count


/*--------------------------------------------------------------------
	Get the width of a specified UTF8 character in bytes
 
	text: The source text
	howMany: The maximum extent in bytes
 
	return: The character width in bytes (nullopt for bad encoding)
  --------------------------------------------------------------------*/
std::optional<unsigned char> string_function::get_UTF8_character_byte_count(const char* text, string_size howMany) {
	if ((howMany < 1) || (text == nullptr)) [[unlikely]]
		return 0;
	char target{*text};
	if (target == 0)
		return 0;
	unsigned char result = 1;
		//If the upper bit is clear, it's a 1-byte char
	if ((target & 0x80) != 0) {
			//Check for other UTF-8 byte sizes
		if ((target & 0xE0) == 0xC0)
			result = 2;
		else if ((target & 0xF0) == 0xE0)
			result = 3;
		else if ((target & 0xF8) == 0xF0)
			result = 4;
		else [[unlikely]]
			return std::nullopt;
		if ((howMany != string_size::npos) && (static_cast<string_size>(result) > howMany)) [[unlikely]]
			return std::nullopt;
		else {
				//Check that the additional characters in a multi-byte sequence are valid
			for (auto i = result; --i; ) {
				if ((*(text + i) & 0xC0) != 0x80) [[unlikely]]
					return std::nullopt;
			}
		}
	}
	return result;
} //string_function::get_UTF8_character_byte_count


/*--------------------------------------------------------------------
	Get the width of a specified character in bytes
 
	text: The source text
	howMany: The number of bytes in the array
	format: The text data format
 
	return: The character width in bytes (nullopt on failure, i.e. bad encoding)
  --------------------------------------------------------------------*/
std::optional<unsigned char> string_function::get_character_byte_count(const char* text, string_size howMany, text_format format) {
	switch (format.encoding) {
		case UTF8:
			return get_UTF8_character_byte_count(text, howMany);
		case ascii: case ISO8859_1:
			return (*text == 0) ? 0 : 1;
		case UTF16: {
			if (text == nullptr) [[unlikely]]
				return 0;
			if ((howMany != string_size::npos) && (howMany < 2)) [[unlikely]]
				return std::nullopt;
			auto uniChar = *(reinterpret_cast<const char16_t*>(text));
			if (uniChar == 0)
				return 0;
			unsigned char size = is_within_bmp(uniChar) ? 2 : 4;
			return ((howMany != string_size::npos) && (howMany < size)) ? std::nullopt : std::optional<unsigned char>(size);
		}
		case UTF32:
			if (text == nullptr) [[unlikely]]
				return 0;
			if ((howMany != string_size::npos) && (howMany < 4)) [[unlikely]]
				return std::nullopt;
			auto uniChar = *(reinterpret_cast<const char32_t*>(text));
			if (uniChar == 0)
				return 0;
			return is_valid_unicode(uniChar) ? std::optional<unsigned char>(4) : std::nullopt;
	}
	return std::nullopt;
} //string_function::get_character_byte_count


/*--------------------------------------------------------------------
	Get the width of the previous character in bytes
 
	text: The source text (assumed to be pointing to the current character)
	howMany: The maximum extent in bytes (e.g. to the string start)
	format: The text data format
 
	return: The width of the previous character in bytes
  --------------------------------------------------------------------*/
std::optional<unsigned char> string_function::get_prev_char_byte_count(const char* text, string_size howMany, text_format format) {
	return std::nullopt;	//TODO: Implement
} //string_function::get_prev_char_byte_count


/*--------------------------------------------------------------------
	Return the length of a string in bytes, limited by a character count
	
	text: The source text
	howMany: The number of characters to count (nullopt = null-terminated)
	isCountRequired: True if the number of characters must exist in the text (unless howMany = nullopt)
	format: The text data format
	
	return: The number of bytes in the char array (nullopt if isCountRequired and howMany not reached)
  --------------------------------------------------------------------*/
string_size string_function::get_byte_count_char_limited(const char* text, string_size howMany, bool isCountRequired, text_format format) {
		//A null pointer can be a valid input if there is no specific requirement for content, in which case we can say it has zero bytes
	if ((howMany == 0) || (text == nullptr) || (*text == 0))
		return (!howMany || !isCountRequired) ? string_size{0} : string_size{};
		//Start by pointing to the first char, then bump up for each successive char
	const auto* endPos = text;
	auto isCountChecked = howMany ? isCountRequired : false;
		//Ensure the loop is limited where a specific number of chars is specified
	while (!howMany || --howMany) {
			//Check if the number of bytes in the next char can be established
		if (auto nextLen = string_function::get_character_byte_count(endPos, string_size::npos, format); nextLen && (nextLen > 0))
			endPos += *nextLen;	//If so, bump the leading pointer to the next char position
		else {
			if (isCountChecked)
				return std::nullopt;	//If a specific number of chars is required, we haven't reached it - return a fail condition
			break;
		}
	}
		//The final number of bytes is simply the difference between the original text start and the end point reached
	return endPos - text;
} //string_function::get_byte_count_char_limited


/*--------------------------------------------------------------------
	Get the number of valid characters found at a specified address
 
	text: The source text
	howMany: The number of bytes in the array (nullopt = null-terminated)
	format: The text data format
 
	return: The nummber of characters found
  --------------------------------------------------------------------*/
string_size string_function::get_character_count(const char* text, string_size howMany, text_format format) {
	if ((howMany == 0) || (text == nullptr))
		return 0;
	std::string::size_type totalChars = 0;
		//Keep checking the next char until we hit the string end, run out of bytes or find bad encoding
	for (;;) {
		if (auto nextLen = get_character_byte_count(text, howMany, format); nextLen && (nextLen > 0)) {
			text += *nextLen;
			++totalChars;
			if (howMany) {
				howMany -= *nextLen;
				if (howMany == 0)
					break;
			}
		} else
			break;
	}
	return totalChars;
} //string_function::get_character_count


/*--------------------------------------------------------------------
	Calculate a required number of characters based on a specified byte count, word size and (optional) character limit
 
	wordSize: The character word size, e.g. UTF16 = 2, UTF32 = 4
	howMany: The number of bytes (npos = null-terminated)
	charCount: The maximum number of (encoded) chars to read (npos = no limit)
 
	return: The required number of characters
  --------------------------------------------------------------------*/
string_size string_function::get_char_count(unsigned char wordSize, string_size howMany, string_size charCount) {
	if (!howMany && !charCount)
		return {};
	if (!howMany)
		return charCount;
	auto result = howMany / wordSize;
	if (charCount)
		result = std::min(result, charCount);
	return result;
} //get_char_count

	
/*--------------------------------------------------------------------
	Determine the the number of characters in a char array
	
	@param text The source text
	@param howMany The number of bytes in the text (nullopt = null-terminated)
	@param format The text data format
	
	@return The number of characters in the array
  --------------------------------------------------------------------*/
std::string::size_type string_function::get_string_length(const char* text, string_size howMany, text_format format) {
	if ((howMany == 0) || (text == nullptr))
		return 0;
	std::string::size_type charCount = 0;
		//Loop until we hit a null char (or run out of bytes - checked within loop)
	while (*text != 0) {
			//This will return nullopt when the string is exhausted of badly encoded, in which case the loop exits
		if (auto nextLen = string_function::get_character_byte_count(text, howMany, format); nextLen && (nextLen > 0)) {
			++charCount;
				//If there are a finite number of bytes, we need to reduce the number remaining
			if (howMany)
				howMany -= *nextLen;
				//Bump the source text to the next character
			text += *nextLen;
		} else
			break;
	}
	return charCount;
} //string_function::get_string_length


/*--------------------------------------------------------------------
	Get a UTF-32 char from a UTF-8 source
 
	text: The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	howMany: The number of available bytes in the source (default = null-terminated)
 
	return: A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, unsigned char> string_function::get_utf32_char_from_utf8(const char*& text, string_size howMany) {
	std::pair<char32_t, unsigned char> result{0, 0};
		//Determine text points to a valid character and get the size
	auto size = string_function::get_character_byte_count(text, howMany, UTF8);
	if (!size || (size == 0))
		return result;
		//Get the first byte to initialise the code point
	result.first = static_cast<unsigned char>(*text);
	result.second = *size;
	++text;
		//If this is a single-byte character, we're done
	if (result.second > 1) {
		result.first &= 0x07;
			//Otherwise left-shift the result by 6 bits and merge in the lower 6 bits of the next char
		for (auto i = result.second; --i; ++text)
			result.first = (result.first << 6) | (static_cast<unsigned char>(*text) & 0x3F);
	}
	return result;
} //string_function::get_utf32_char_from_utf8


/*--------------------------------------------------------------------
	Get a UTF-32 char from a UTF-16 source
 
	text: The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	is_big_endian: True if byte ordering is big-endian
	howMany: The number of available bytes in the source
 
	return: A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, unsigned char> string_function::get_utf32_char_from_utf16(const char16_t*& text, bool is_big_endian, string_size howMany) {
	std::pair<char32_t, unsigned char> result{0, 0};
	if (howMany < sizeof(char16_t))
		return result;	//No chars to read from source
	result.first = static_cast<char32_t>(*text);
		//Byte-swap incoming data if neccessary (no action if incoming data matches platform byte order)
	byte_swap(reinterpret_cast<char16_t*>(&result.first), 2, is_big_endian);
	++text;
		//Deal with single 16-bit encoding first
	if (string_function::is_within_bmp(static_cast<char16_t>(result.first))) {
		result.second = sizeof(char16_t);
		return result;
	}
	if (howMany < (2 * sizeof(char16_t)))
		return result;	//Incomplete char - bad result
	if ((*text < 0xDC00) || (*text > 0xDFFF))
		return result;	//Bad encoding
	result.first = (((result.first - 0xD800) << 10) | (static_cast<char32_t>(*text) - 0xDC00)) + 0x10000;
	++text;
	result.second = 2 * sizeof(char16_t);
	return result;
} //string_function::get_utf32_char_from_utf16


/*--------------------------------------------------------------------
	Get a unicode (UTF-32) character from a specified source
 
	text: The source text
	howMany: The number of bytes in the text
	format: The source data format
 
	return: The unicode char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, unsigned char> string_function::get_unicode_char(const char* text, string_size howMany, text_format format) {
	if (text == nullptr)
		return {0, 0};
	switch (format.encoding) {
		case UTF8:
			return get_utf32_char_from_utf8(text, howMany.value_or(possibleCharWidth));
		case UTF16: {
			auto source = reinterpret_cast<const char16_t*>(text);
			auto result = get_utf32_char_from_utf16(source, format.is_big_endian, howMany.value_or(possibleCharWidth));
			return result;
		}
		case UTF32: {
			std::pair<char32_t, unsigned char> result{0, 0};
			if (howMany && (howMany < sizeof(char32_t)))
				return result;
			auto source = reinterpret_cast<const char32_t*>(text);
			result.first = source[0];
			if (is_valid_unicode(result.first)) {
				byte_swap(&result.first, 1, format.is_big_endian);
				result.second = sizeof(char32_t);
			} else
				result.second = 0;	//Invalid UTF32 code point
			return result;
		}
		case ascii: case ISO8859_1:
			std::pair<char32_t, unsigned char> result{0, 0};
			if (howMany < 1)
				return result;
			result.first = static_cast<char32_t>(text[0]);
			result.second = 1;
			return result;
	}
	return {};
} //string_function::get_unicode_char


/*--------------------------------------------------------------------
	Get a UTF-32 string from a UTF-8 source
 
	text: The source text (advances to the byte beyond the last counted character)
	howMany: The number of bytes in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of bytes must be valid (ignored if howMany = nullopt)
 
	return: The unicode code point for the specified chars (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::u32string> string_function::to_unicode(const char*& text, string_size howMany, bool isCountRequired) {
	if (text == nullptr)
		return std::nullopt;
	std::u32string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	auto bytesRemaining = howMany.value_or(possibleCharWidth);
	while (*text != 0) {
		auto nextChar = get_utf32_char_from_utf8(text, bytesRemaining);
		if (nextChar.second == 0)
			break;	//Source is consumed or bad encoding
		uniString += nextChar.first;
		if (howMany)
			bytesRemaining -= nextChar.second;
	}
	return (isCountRequired && (bytesRemaining > 0)) ? std::nullopt : std::optional(uniString);
} //string_function::to_unicode


/*--------------------------------------------------------------------
	Get a UTF-32 string from a UTF-16 (16-bit) source
 
	text: The source text
	is_big_endian: True if byte ordering is big-endian
	howMany: The number of words (16-bit values) in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of words must be valid (ignored if howMany = nullopt)
 
	return: The UTF-32 string read from the UTF-16 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<std::u32string> string_function::from_utf16(const char16_t*& text, bool is_big_endian, string_size howMany, bool isCountRequired) {
	std::u32string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	auto charsRemaining = sizeof(char16_t) * howMany.value_or(possibleCharWidth / sizeof(char16_t));
	while (*text != 0) {
		auto nextChar = get_utf32_char_from_utf16(text, is_big_endian, charsRemaining);
		if (nextChar.second == 0)
			break;	//Source is consumed or bad encoding
		uniString += nextChar.first;
		if (howMany)
			charsRemaining -= nextChar.second;
	}
	return (isCountRequired && (howMany > 0)) ? std::nullopt : std::optional(uniString);
} //string_function::from_utf16


/*--------------------------------------------------------------------
	Get a UTF-16 string from a UTF-32 source
 
	text: The source text
	howMany: The number of code points in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of code points must be valid (ignored if howMany = nullopt)
 
	return: The UTF-16 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<std::u16string> string_function::to_utf16(const char32_t*& text, string_size howMany, bool isCountRequired) {
	std::u16string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	for ( ; *text != 0; ++text) {
		if (howMany) {
			if (howMany == 0)
				break;
			howMany -= 1;
		}
		char32_t code = static_cast<char32_t>(*text);
			//Deal with single 16-bit encoding first
		if ((code < 0xD800) || ((code >= 0xE000) && (code < 0xFFFF))) {
			uniString += static_cast<char16_t>(code);
			continue;
		}
		code = code - 0x10000;
		uniString += static_cast<char16_t>(code >> 10) + 0xD800;
		uniString += static_cast<char16_t>(code & 0x3FF) + 0xDC00;
	}
	return (isCountRequired && (howMany > 0)) ? std::nullopt : std::optional(uniString);
} //string_function::to_utf16


/*--------------------------------------------------------------------
	Collect the byte size of each (valid) character from a string into an array
	
	@param text The source text
	@param howMany The number of characters to collect, taken as a maximum rather than a requirement (nullopt = null-terminated)
	@param format The text data format (collection will stop if a character not matching the encoding is found)
	
	@return An array containing the byte size of each character found (nullopt if no valid chars found)
  --------------------------------------------------------------------*/
std::vector<unsigned char> string_function::collect_char_byte_count(const char* text, string_size howMany,
												text_format format) {
		//Array to collect character sizes
	std::vector<unsigned char> charLength;
	if (text == nullptr)
		return charLength;
		//Note: loop still works as expected even when howMany = nullopt
	while (!howMany || howMany--) {
		if (auto nextLen = string_function::get_character_byte_count(text, string_size::npos, format); nextLen && (nextLen > 0)) {
			charLength.push_back(*nextLen);
			text += *nextLen;
		} else
			break;
	}
	return charLength;
} //string_function::collect_char_byte_count


/*--------------------------------------------------------------------
	Calculate the byte offsets for a start and number of chars within a string
 
	text: The source text
	startPos: The start character
	howMany: The number of characters to measure (nullopt = null-terminated)
	isHowManyChars: True if the returned second value should be the char count rather than the byte count
 
	return: Byte offset to the start character paired with byte/char offset from the start to the end of the last character (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::pair<std::string::size_type, std::string::size_type>> string_function::get_byte_offsets(const char* text,
		string_size startPos, string_size howMany, bool isHowManyChars) {
	if (text == nullptr)
		return std::nullopt;
		//Find the byte offset to the start char
	auto startByte = (startPos == 0) ? string_size{0} : string_function::get_byte_count_char_limited(text, startPos, true);
	if (!startByte)
		return std::nullopt;
		//Now get the number of bytes or chars to the last char - if successful, return the required offsets
	if (auto textCount = isHowManyChars ?
			string_function::get_character_count(text + startByte, howMany) :
			string_function::get_byte_count_char_limited(text + startByte, howMany); textCount)
		return std::optional(std::make_pair(startByte, textCount));
	return std::nullopt;	//Fail condition
} //string_function::get_byte_offsets
