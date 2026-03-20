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
	static void byteSwap(T* val, std::string::size_type howMany, bool toBigEndian) {
		if ((howMany < 1) || (toBigEndian == text_format::defaultEndian))
			return;
		for (; howMany--; ++val) {
			auto data = reinterpret_cast<unsigned char*>(val);
			auto bytes = sizeof(T);
			for (auto i = bytes-- / 2; i--; )
				std::swap(data[i], data[bytes - i]);
		}
	} //byteSwap

}  // namespace


/*--------------------------------------------------------------------
	Get the number of bytes in the specified text (counting only valid UTF-8 characters)
 
	text: The source text
	howMany: The number of bytes in the text (nullopt = null-terminated)
	charCount: The maximum number of (encoded) chars to seek
	format: The text data format
 
	return: The number of bytes in the text containing valid characters
  --------------------------------------------------------------------*/
std::string::size_type string_function::getValidByteCount(const char* text, std::optional<std::string::size_type> howMany, std::optional<std::string::size_type> charCount, text_format format) {
	bool isOpen = !charCount;
		//Detect empty strings or null requests
	if ((text == nullptr) || (howMany == 0) || (!isOpen && (*charCount < 1)))
		return 0;
	const auto* endPos = text;
	do {
			//Get the size of the next char
		if (auto nextLen = string_function::getCharacterByteCount(endPos, howMany, format); nextLen && (*nextLen > 0)) {
			endPos += *nextLen;
				//If the source is byte-limited, ensure the remaining count is updated
			if (howMany) {
				*howMany -= *nextLen;
				if (*howMany == 0)
					break;
			}
		} else
			break;
	} while ((*endPos != 0) && (isOpen || --(*charCount)));
	return static_cast<std::string::size_type>(endPos - text);
} //string_function::getValidByteCount


/*--------------------------------------------------------------------
	Get the width of a specified character in bytes
 
	text: The source text
	howMany: The number of bytes in the array
	format: The text data format
 
	return: The character width in bytes (nullopt on failure, i.e. bad encoding)
  --------------------------------------------------------------------*/
std::optional<unsigned char> string_function::getCharacterByteCount(const char* text, std::optional<std::string::size_type> howMany, text_format format) {
	if ((howMany == 0) || (text == nullptr))
		return 0;
	switch (format.encoding) {
		case UTF8: {
			if (*text == 0)
				return 0;
			unsigned char result = 1;
				//If the upper bit is clear, it's a 1-byte char
			if ((*text & 0x80) != 0) {
					//Check for other UTF-8 byte sizes
				if ((*text & 0xE0) == 0xC0)
					result = 2;
				else if ((*text & 0xF0) == 0xE0)
					result = 3;
				else if ((*text & 0xF8) == 0xF0)
					result = 4;
				else
					return std::nullopt;
				if (howMany && (result > howMany))
					return std::nullopt;
				else {
						//Check that the additional characters in a multi-byte sequence are valid
					for (auto i = result; --i; ) {
						if ((*(text + i) & 0xC0) != 0x80) {
							return std::nullopt;
						}
					}
				}
			}
			return result;
		}
		case ascii: case ISO8859_1:
			return (*text == 0) ? 0 : 1;
		case UTF16: {
			if (howMany && (howMany < 2))
				return std::nullopt;
			auto uniChar = *(reinterpret_cast<const char16_t*>(text));
			if (uniChar == 0)
				return 0;
			unsigned char size = isWithinBMP(uniChar) ? 2 : 4;
			return (howMany && (howMany < size)) ? std::nullopt : std::optional<unsigned char>(size);
		}
		case UTF32:
			if (howMany && (howMany < 4))
				return std::nullopt;
			auto uniChar = *(reinterpret_cast<const char32_t*>(text));
			if (uniChar == 0)
				return 0;
			return isValidUnicode(uniChar) ? std::optional<unsigned char>(4) : std::nullopt;
	}
	return 0;
} //string_function::getCharacterByteCount


/*--------------------------------------------------------------------
	Return the length of a string in bytes, limited by a character count
	
	text: The source text
	howMany: The number of characters to count (nullopt = null-terminated)
	isCountRequired: True if the number of characters must exist in the text (unless howMany = nullopt)
	format: The text data format
	
	return: The number of bytes in the char array (nullopt if isCountRequired and howMany not reached)
  --------------------------------------------------------------------*/
std::optional<std::string::size_type> string_function::getByteCountCharLimited(const char* text, std::optional<std::string::size_type> howMany, bool isCountRequired, text_format format) {
		//A null pointer can be a valid input if there is no specific requirement for content, in which case we can say it has zero bytes
	if ((howMany == 0) || (text == nullptr) || (*text == 0))
		return (!howMany || !isCountRequired) ? std::optional(0) : std::nullopt;
		//Start by pointing to the first char, then bump up for each successive char
	const auto* endPos = text;
	auto isCountChecked = howMany ? isCountRequired : false;
		//Ensure the loop is limited where a specific number of chars is specified
	while (!howMany || (*howMany)--) {
			//Check if the number of bytes in the next char can be established
		if (auto nextLen = string_function::getCharacterByteCount(endPos, std::nullopt, format); nextLen && (nextLen > 0))
			endPos += *nextLen;	//If so, bump the leading pointer to the next char position
		else {
			if (isCountChecked)
				return std::nullopt;	//If a specific number of chars is required, we haven't reached it - return a fail condition
			break;
		}
	}
		//The final number of bytes is simply the difference between the original text start and the end point reached
	return static_cast<std::string::size_type>(endPos - text);
} //string_function::getByteCountCharLimited


/*--------------------------------------------------------------------
	Get the number of valid characters found at a specified address
 
	text: The source text
	howMany: The number of bytes in the array (nullopt = null-terminated)
	format: The text data format
 
	return: The nummber of characters found
  --------------------------------------------------------------------*/
std::optional<std::string::size_type> string_function::getCharacterCount(const char* text, std::optional<std::string::size_type> howMany, text_format format) {
	if ((howMany == 0) || (text == nullptr))
		return 0;
	std::string::size_type totalChars = 0;
		//Keep checking the next char until we hit the string end, run out of bytes or find bad encoding
	for (;;) {
		if (auto nextLen = getCharacterByteCount(text, howMany, format); nextLen && (nextLen > 0)) {
			text += *nextLen;
			++totalChars;
			if (howMany) {
				*howMany -= *nextLen;
				if (howMany == 0)
					break;
			}
		} else
			break;
	}
	return totalChars;
} //string_function::getCharacterCount


/*--------------------------------------------------------------------
	Calculate a required number of characters base on a specified byte count, word size and (optional) character limit
 
	wordSize: The character word size, e.g. UTF16 = 2, UTF32 = 4
	howMany: The number of bytes (npos = null-terminated)
	charCount: The maximum number of (encoded) chars to read (npos = no limit)
 
	return: The required number of characters
  --------------------------------------------------------------------*/
std::optional<std::string::size_type> string_function::getCharCount(unsigned char wordSize, std::optional<std::string::size_type> howMany, std::optional<std::string::size_type> charCount) {
	if (!howMany && !charCount)
		return std::nullopt;
	if (!howMany)
		return *charCount;
	auto result = *howMany / wordSize;
	if (charCount)
		result = std::min(result, *charCount);
	return result;
} //getCharCount

	
/*--------------------------------------------------------------------
	Determine the the number of characters in a char array
	
	@param text The source text
	@param howMany The number of bytes in the text (nullopt = null-terminated)
	@param format The text data format
	
	@return The number of characters in the array
  --------------------------------------------------------------------*/
std::string::size_type string_function::getStringLength(const char* text, std::optional<std::string::size_type> howMany, text_format format) {
	if ((howMany == 0) || (text == nullptr))
		return 0;
	std::string::size_type charCount = 0;
		//Loop until we hit a null char (or run out of bytes - checked within loop)
	while (*text != 0) {
			//This will return nullopt when the string is exhausted of badly encoded, in which case the loop exits
		if (auto nextLen = string_function::getCharacterByteCount(text, howMany, format); nextLen && (nextLen > 0)) {
			++charCount;
				//If there are a finite number of bytes, we need to reduce the number remaining
			if (howMany)
				*howMany -= *nextLen;
				//Bump the source text to the next character
			text += *nextLen;
		} else
			break;
	}
	return charCount;
} //string_function::getStringLength


/*--------------------------------------------------------------------
	Get a UTF-32 char from a UTF-8 source
 
	text: The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	howMany: The number of available bytes in the source
 
	return: A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, unsigned char> string_function::getUTF32CharFromUTF8(const char*& text, std::string::size_type howMany) {
	std::pair<char32_t, unsigned char> result{0, 0};
		//Determine text points to a valid character and get the size
	auto size = string_function::getCharacterByteCount(text, howMany, UTF8);
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
} //string_function::getUTF32CharFromUTF8


/*--------------------------------------------------------------------
	Get a UTF-32 char from a UTF-16 source
 
	text: The UTF-8 source text (when valid, points to the next byte beyond the found character on return)
	is_big_endian: True if byte ordering is big-endian
	howMany: The number of available bytes in the source
 
	return: A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, unsigned char> string_function::getUTF32CharFromUTF16(const char16_t*& text, bool is_big_endian, std::string::size_type howMany) {
	std::pair<char32_t, unsigned char> result{0, 0};
	if (howMany < sizeof(char16_t))
		return result;	//No chars to read from source
	result.first = static_cast<char32_t>(*text);
		//Byte-swap incoming data if neccessary (no action if incoming data matches platform byte order)
	byteSwap(reinterpret_cast<char16_t*>(&result.first), 2, is_big_endian);
	++text;
		//Deal with single 16-bit encoding first
	if (string_function::isWithinBMP(static_cast<char16_t>(result.first))) {
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
} //string_function::getUTF32CharFromUTF16


/*--------------------------------------------------------------------
	Get a unicode (UTF-32) character from a specified source
 
	text: The source text
	howMany: The number of bytes in the text
	format: The source data format
 
	return: The unicode char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, unsigned char> string_function::getUnicodeChar(const char* text, std::optional<std::string::size_type> howMany, text_format format) {
	if (text == nullptr)
		return {};
	switch (format.encoding) {
		case UTF8:
			return getUTF32CharFromUTF8(text, howMany.value_or(possibleCharWidth));
		case UTF16: {
			auto source = reinterpret_cast<const char16_t*>(text);
			auto result = getUTF32CharFromUTF16(source, format.is_big_endian, howMany.value_or(possibleCharWidth));
			return result;
		}
		case UTF32: {
			std::pair<char32_t, unsigned char> result{0, 0};
			if (howMany && (*howMany < sizeof(char32_t)))
				return result;
			auto source = reinterpret_cast<const char32_t*>(text);
			result.first = source[0];
			if (isValidUnicode(result.first)) {
				byteSwap(&result.first, 1, format.is_big_endian);
				result.second = sizeof(char32_t);
			} else
				result.second = 0;	//Invalid UTF32 code point
			return result;
		}
		case ascii: case ISO8859_1:
			std::pair<char32_t, unsigned char> result{0, 0};
			if (howMany && (*howMany < 1))
				return result;
			result.first = static_cast<char32_t>(text[0]);
			result.second = 1;
			return result;
	}
	return {};
} //string_function::getUnicodeChar


/*--------------------------------------------------------------------
	Get a UTF-32 string from a UTF-8 source
 
	text: The source text (advances to the byte beyond the last counted character)
	howMany: The number of bytes in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of bytes must be valid (ignored if howMany = nullopt)
 
	return: The unicode code point for the specified chars (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::u32string> string_function::toUnicode(const char*& text, std::optional<std::string::size_type> howMany, bool isCountRequired) {
	if (text == nullptr)
		return std::nullopt;
	std::u32string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	auto bytesRemaining = howMany.value_or(possibleCharWidth);
	while (*text != 0) {
		auto nextChar = getUTF32CharFromUTF8(text, bytesRemaining);
		if (nextChar.second == 0)
			break;	//Source is consumed or bad encoding
		uniString += nextChar.first;
		if (howMany)
			bytesRemaining -= nextChar.second;
	}
	return (isCountRequired && (bytesRemaining > 0)) ? std::nullopt : std::optional(uniString);
} //string_function::toUnicode


/*--------------------------------------------------------------------
	Get a UTF-32 string from a UTF-16 (16-bit) source
 
	text: The source text
	is_big_endian: True if byte ordering is big-endian
	howMany: The number of words (16-bit values) in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of words must be valid (ignored if howMany = nullopt)
 
	return: The UTF-32 string read from the UTF-16 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<std::u32string> string_function::fromUTF16(const char16_t*& text, bool is_big_endian, std::optional<std::string::size_type> howMany, bool isCountRequired) {
	std::u32string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	auto charsRemaining = sizeof(char16_t) * howMany.value_or(possibleCharWidth / sizeof(char16_t));
	while (*text != 0) {
		auto nextChar = getUTF32CharFromUTF16(text, is_big_endian, charsRemaining);
		if (nextChar.second == 0)
			break;	//Source is consumed or bad encoding
		uniString += nextChar.first;
		if (howMany)
			charsRemaining -= nextChar.second;
	}
	return (isCountRequired && (howMany > 0)) ? std::nullopt : std::optional(uniString);
} //string_function::fromUTF16


/*--------------------------------------------------------------------
	Get a UTF-16 string from a UTF-32 source
 
	text: The source text
	howMany: The number of code points in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of code points must be valid (ignored if howMany = nullopt)
 
	return: The UTF-16 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<std::u16string> string_function::toUTF16(const char32_t*& text, std::optional<std::string::size_type> howMany, bool isCountRequired) {
	std::u16string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	for ( ; *text != 0; ++text) {
		if (howMany) {
			if (howMany == 0)
				break;
			*howMany -= 1;
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
} //string_function::toUTF16


/*--------------------------------------------------------------------
	Collect the byte size of each (valid) character from a string into an array
	
	@param text The source text
	@param howMany The number of characters to collect, taken as a maximum rather than a requirement (nullopt = null-terminated)
	@param format The text data format (collection will stop if a character not matching the encoding is found)
	
	@return An array containing the byte size of each character found (nullopt if no valid chars found)
  --------------------------------------------------------------------*/
std::vector<unsigned char> string_function::collectCharByteCount(const char* text, std::optional<std::string::size_type> howMany,
												text_format format) {
		//Array to collect character sizes
	std::vector<unsigned char> charLength;
	if (text == nullptr)
		return charLength;
		//Note: loop still works as expected even when howMany = nullopt
	while (!howMany || (*howMany)--) {
		if (auto nextLen = string_function::getCharacterByteCount(text, std::nullopt, format); nextLen && (nextLen > 0)) {
			charLength.push_back(*nextLen);
			text += *nextLen;
		} else
			break;
	}
	return charLength;
} //string_function::collectCharByteCount


/*--------------------------------------------------------------------
	Calculate the byte offsets for a start and number of chars within a string
 
	text: The source text
	startPos: The start character
	howMany: The number of characters to measure (nullopt = null-terminated)
	isHowManyChars: True if the returned second value should be the char count rather than the byte count
 
	return: Byte offset to the start character paired with byte/char offset from the start to the end of the last character (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::pair<std::string::size_type, std::string::size_type>> string_function::getByteOffsets(const char* text,
		std::string::size_type startPos, std::optional<std::string::size_type> howMany, bool isHowManyChars) {
	if (text == nullptr)
		return std::nullopt;
		//Find the byte offset to the start char
	auto startByte = (startPos == 0) ? 0 : string_function::getByteCountCharLimited(text, startPos, true);
	if (!startByte)
		return std::nullopt;
		//Caller shouldn't use legacy string::npos, but this check enforces the 'optional' approach
	if (howMany == std::string::npos)
		howMany = std::nullopt;
		//Now get the number of bytes or chars to the last char - if successful, return the required offsets
	if (auto textCount = isHowManyChars ?
			string_function::getCharacterCount(text + *startByte, howMany) :
			string_function::getByteCountCharLimited(text + *startByte, howMany); textCount)
		return std::optional(std::make_pair(*startByte, *textCount));
	return std::nullopt;	//Fail condition
} //string_function::getByteOffsets
