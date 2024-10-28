/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/String.h"

#include "Active/Utility/BufferIn.h"
#include "Active/Utility/BufferOut.h"

#include <cwctype>
#include <format>
#include <numeric>
#include <locale>
#include <optional>
#include <set>
#include <stdexcept>

using namespace active::utility;

namespace active::utility {
	
		///The line terminating char(s) for the current platform
#ifdef WINDOWS
	const String String::lineTerminator{"\r\n"};
#else
	const String String::lineTerminator{"\n"};
#endif
		///All white space characters
	const String String::allWhiteSpace{" \t\r\n"};
		///All line ending characters
	const String String::allLineEnding{"\r\n"};
			///All digit characters (0-9)
	const String String::allDigit{"0123456789"};
			///All numeric characters (floating and integer)
	const String String::allNumeric{"0123456789-"};
		///All numeric characters (floating and integer)
	const String String::allFloat{"0123456789.-"};
}  // namespace active::utility

//MARK: - Internal implementation

namespace {
	
	using enum TextEncoding;

	constexpr String::size_type possibleCharWidth = 4;

	/*!
		Return the length of a string in bytes, limited by a character count
		
		@param text The source text
		@param howMany The number of characters to count (nullopt = null-terminated)
		@param isCountRequired True if the number of characters must exist in the text (unless howMany = nullopt)
		@param format The text data format
		
		@return The number of bytes in the char array (nullopt if isCountRequired and howMany not reached)
	*/
	String::sizeOption getByteCountCharLimited(const char* text, String::sizeOption howMany = std::nullopt, bool isCountRequired = false,
												DataFormat format = DataFormat{}) {
			//A null pointer can be a valid input if there is no specific requirement for content, in which case we can say it has zero bytes
		if ((howMany == 0) || (*text == 0))
			return (!howMany || !isCountRequired) ? std::optional(0) : std::nullopt;
			//Start by pointing to the first char, then bump up for each successive char
		const auto* endPos = text;
		auto isCountChecked = howMany ? isCountRequired : false;
			//Ensure the loop is limited where a specific number of chars is specified
		while (!howMany || (*howMany)--) {
				//Check if the number of bytes in the next char can be established
			if (auto nextLen = String::getCharacterByteCount(endPos, std::nullopt, format); nextLen && (nextLen > 0))
				endPos += *nextLen;	//If so, bump the leading pointer to the next char position
			else {
				if (isCountChecked)
					return std::nullopt;	//If a specific number of chars is required, we haven't reached it - return a fail condition
				break;
			}
		}
			//The final number of bytes is simply the difference between the original text start and the end point reached
		return static_cast<String::size_type>(endPos - text);
	} //getByteCountCharLimited
	
	
	/*!
		Collect the byte size of each (valid) character from a string into an array
		
		@param text The source text
		@param howMany The number of characters to collect, taken as a maximum rather than a requirement (nullopt = null-terminated)
		@param format The text data format (collection will stop if a character not matching the encoding is found)
		
		@return An array containing the byte size of each character found (nullopt if no valid chars found)
	*/
	std::optional<std::vector<String::size_type>> collectCharByteCount(const char* text, String::sizeOption howMany = std::nullopt,
																		DataFormat format = DataFormat{}) {
			//Array to collect character sizes
		std::vector<String::size_type> charLength;
			//Note: loop still works as expected even when howMany = nullopt
		while (!howMany || (*howMany)--) {
			if (auto nextLen = String::getCharacterByteCount(text, std::nullopt, format); nextLen && (nextLen > 0)) {
				charLength.push_back(*nextLen);
				text += *nextLen;
			} else
				break;
		}
		return charLength.empty() ? std::nullopt : std::optional(charLength);
	} //collectCharByteCount
	
	
	/*!
		Determine the the number of characters in a char array
		
		@param text The source text
		@param howMany The number of bytes in the text (nullopt = null-terminated)
		@param format The text data format
		
		@return The number of characters in the array
	*/
	String::size_type getStringLength(const char* text, String::sizeOption howMany = std::nullopt, DataFormat format = DataFormat{}) {
		if (howMany == 0)
			return 0;
		String::size_type charCount = 0;
			//Loop until we hit a null char (or run out of bytes - checked within loop)
		while (*text != 0) {
				//This will return nullopt when the string is exhausted of badly encoded, in which case the loop exits
			if (auto nextLen = String::getCharacterByteCount(text, howMany, format); nextLen && (nextLen > 0)) {
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
	} //getStringLength
	
	
	/*!
		Calculate a required number of characters base on a specified byte count, word size and (optional) character limit
		@param wordSize The character word size, e.g. UTF16 = 2, UTF32 = 4
		@param howMany The number of bytes (npos = null-terminated)
		@param charCount The maximum number of (encoded) chars to read (npos = no limit)
		@return The required number of characters
	*/
	String::sizeOption getCharCount(String::size_type wordSize, String::sizeOption howMany = std::nullopt, String::sizeOption charCount = std::nullopt) {
		if (!howMany && !charCount)
			return std::nullopt;
		if (!howMany)
			return *charCount;
		auto result = *howMany / wordSize;
		if (charCount)
			result = std::min(result, *charCount);
		return result;
	} //getCharCount
	
	
	/*!
		Make an STL string from a char array. NB: always returns a result, but result will only include valid chars
		
		@param target An STL string to be populated with valid characters found
		@param text The source text
		@param howMany The number of bytes in the text (nullopt = null-terminated)
		@param charCount The maximum number of (encoded) chars to read (npos = no limit)
		@param format The text data format
		
		@return The number of source bytes used in the populated string
	*/
	String::size_type makeString(std::string& target, const char* text, String::sizeOption howMany = std::nullopt,
			String::sizeOption charCount = std::nullopt, DataFormat format = DataFormat{}) {
		String::size_type dataSize = 0;
		switch (format.encoding) {
			case UTF8: case ascii: {
				dataSize = String::getValidByteCount(text, howMany, charCount, format.encoding);
				if (dataSize < 1)
					break;
					//Any valid bytes can be consumed directly by std::string constructor
				target.assign(text, dataSize);
				break;
			}
			case UTF16: {
				const auto* source = reinterpret_cast<const char16_t*>(text);
				if (auto string32 = String::fromUTF16(source, format.isBigEndian, getCharCount(sizeof(char16_t), howMany, charCount)); string32) {
					const char32_t* source32 = string32->data();
					if (auto uniString = String::fromUnicode(source32, format.isBigEndian); uniString) {
						target = *uniString;
						dataSize = target.size() * sizeof(char16_t);
					}
				}
				break;
			}
			case UTF32: {
				const auto* source = reinterpret_cast<const char32_t*>(text);
				if (auto uniString = String::fromUnicode(source, format.isBigEndian, getCharCount(sizeof(char32_t), howMany, charCount)); uniString) {
					target = *uniString;
					dataSize = target.size() * sizeof(char32_t);
				}
				break;
			}
			case ISO8859_1:
				dataSize = String::getValidByteCount(text, howMany, charCount, format);
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
	} //makeString

	
	/*!
		Split the specified text into single and multi byte chars
	 
		@param source The source text
	 
		@return An array of single and multi-byte charsfrom the source string (nullopt on failure)
	*/
	std::optional<std::vector<String>> splitSingleChars(const String& source) {
		auto charBytes = collectCharByteCount(source.data());
		if (!charBytes)
			return std::nullopt;
			//The first string in the result is reserved for single-byte chars
		std::string singleChars;
		std::set<String> multiChar;
		const auto* text = source.data();
			//Iterate through all chars, either appending to the single-bytes chars item or collecting in a set of unique multi-byte chars
		for (auto& charSize : *charBytes) {
			if (charSize == 1) {
					//Make sure we don't have the char already
				if (singleChars.find(text[0]) == std::string::npos)
					singleChars += text[0];
			} else
				multiChar.insert(String{text, charSize});
				//Move to the next char
			text += charSize;
		}
		std::vector<String> splitString{String{singleChars}};
			//Append the unique multi-byte chars to the result
		for (const auto& i : multiChar)
			splitString.push_back(i);
		return std::optional(splitString);
	} //splitSingleChars
	
	
	/*!
		Calculate the byte offsets for a start and number of chars within a string
	 
		@param text The source text
		@param startPos The start character
		@param howMany The number of characters to measure (nullopt = null-terminated)
		@param isHowManyChars True if the returned second value should be the char count rather than the byte count
	 
		@return Byte offset to the start character paired with byte/char offset from the start to the end of the last character (nullopt on failure)
	*/
	std::optional<std::pair<String::size_type, String::size_type>> getByteOffsets(const char* text,
																				  String::size_type startPos = 0,
																				  String::sizeOption howMany = std::nullopt,
																				  bool isHowManyChars = false) {
			//Find the byte offset to the start char
		auto startByte = (startPos == 0) ? 0 : getByteCountCharLimited(text, startPos, true);
		if (!startByte)
			return std::nullopt;
			//Caller shouldn't use legacy String::npos, but this check enforces the 'optional' approach
		if (howMany == String::npos)
			howMany = std::nullopt;
			//Now get the number of bytes or chars to the last char - if successful, return the required offsets
		if (auto textCount = isHowManyChars ?
				String::getCharacterCount(text + *startByte, howMany) :
				getByteCountCharLimited(text + *startByte, howMany); textCount)
			return std::optional(std::make_pair(*startByte, *textCount));
		return std::nullopt;	//Fail condition
	} //getByteOffsets
	
	
	/*--------------------------------------------------------------------
		Determine if a 16-bit (UTF16) character code is within the BMP
	 
		code: The character code
	 
		return: True if a 16-bit (UTF16) character code is within the BMP
	  --------------------------------------------------------------------*/
	bool isWithinBMP(char16_t code) {
		return ((code < 0xD800) || (code >= 0xE000));
	} //isWithinBMP
	
	
	/*--------------------------------------------------------------------
		Determine if a 32-bit (UTF32) character code is valid
	 
		code: The character code
	 
		return: True if a 32-bit (UTF32) character code is valid
	  --------------------------------------------------------------------*/
	bool isValidUnicode(char32_t uniChar) {
		return ((uniChar <= 0x10FFFF) && ((uniChar > 0xDFFF) || (uniChar < 0xD800)));
	} //isValidUnicode
	
	
	/*--------------------------------------------------------------------
		Get a UTF-32 char from a UTF-8 source
	 
		text: The UTF-8 source text
		howMany: The number of available bytes in the source
	 
		return: A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
	  --------------------------------------------------------------------*/
	std::pair<char32_t, short> getUTF32CharFromUTF8(const char*& text, String::size_type howMany) {
		std::pair<char32_t, short> result{0, 0};
			//Determine text points to a valid character and get the size
		auto size = String::getCharacterByteCount(text, howMany, UTF8);
		if (!size || (size == 0))
			return result;
			//Get the first byte to initialise the code point
		result.first = static_cast<unsigned char>(*text);
		result.second = static_cast<short>(*size);
		++text;
			//If this is a single-byte character, we're done
		if (result.second > 1) {
			result.first &= 0x07;
				//Otherwise left-shift the result by 6 bits and merge in the lower 6 bits of the next char
			for (auto i = result.second; --i; ++text)
				result.first = (result.first << 6) | (static_cast<unsigned char>(*text) & 0x3F);
		}
		return result;
	} //getUTF32CharFromUTF8
	
	
	/*--------------------------------------------------------------------
		Get a UTF-32 char from a UTF-16 source
	 
		text: The UTF-16 source text
		isBigEndian: True if byte ordering is big-endian
		howMany: The number of available bytes in the source
	 
		return: A UTF-32 char paired with the number of bytes consumed from the source (0 = no valid char found)
	  --------------------------------------------------------------------*/
	std::pair<char32_t, short> getUTF32CharFromUTF16(const char16_t*& text, bool isBigEndian, String::size_type howMany) {
		std::pair<char32_t, short> result{0, 0};
		if (howMany < sizeof(char16_t))
			return result;	//No chars to read from source
		result.first = static_cast<char32_t>(*text);
			//Byte-swap incoming data if neccessary (no action if incoming data matches platform byte order)
		Memory::byteSwap(reinterpret_cast<char16_t*>(&result.first), 2, isBigEndian);
		++text;
			//Deal with single 16-bit encoding first
		if (isWithinBMP(static_cast<char16_t>(result.first))) {
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
	} //getUTF32CharFromUTF16

}  // namespace

// MARK: - Constructors

/*--------------------------------------------------------------------
	Constructor from an input buffer
 
	source: The source input buffer
  --------------------------------------------------------------------*/
String::String(const BufferIn&& source) {
	source.getString(*this);
} //String::String


/*--------------------------------------------------------------------
	Constructor

	source: The character array to be copied
	howMany: The number of bytes to copy (nullopt for full length)
	encoding: The character encoding type
  --------------------------------------------------------------------*/
String::String(const char* source, sizeOption howMany, DataFormat format) {
	 makeString(m_string, source, howMany, std::nullopt, format);
} //String::String


/*--------------------------------------------------------------------
	Constructor from an input string
 
	source: The UTF-16 string to be copied
	howMany: The number of chars to copy (nullopt for full length)
 --------------------------------------------------------------------*/
String::String(const std::u16string& source, sizeOption howMany) {
	const char16_t* text = source.data();
		//First convert tp UTF-32
	if (auto string32 = fromUTF16(text, Memory::defaultEndian, howMany); string32) {
			//Then convert UTF-32 to UTF-8
		const char32_t* text32 = string32->data();
			//NB: We are assuming that the incoming u16string byte ordering is the platform default
		if (auto uniString = fromUnicode(text32, Memory::defaultEndian); uniString)
			m_string = uniString->m_string;
	}
} //String::String


/*--------------------------------------------------------------------
	Constructor from an input string
 
	source: The UTF-32 string to be copied
	howMany: The number of chars to copy (nullopt for full length)
 --------------------------------------------------------------------*/
String::String(const std::u32string& source, sizeOption howMany) {
	const char32_t* text = source.data();
		//NB: We are assuming that the incoming u32string byte ordering is the platform default
	if (auto uniString = fromUnicode(text, Memory::defaultEndian, howMany); uniString)
		m_string = uniString->m_string;
} //String::String


/*--------------------------------------------------------------------
	Constructor with optional text fill expression and number of repeats
 
	newSize: The required number of expression repeats
	fillText: The expression to fill the resized string
  --------------------------------------------------------------------*/
String::String(size_type newSize, const String& fillText) {
	m_string = std::string{};
	String::resize(newSize, fillText);
} //String::String


/*--------------------------------------------------------------------
	Constructor from double
 
	val: A double value
	prec: The required precision
	padZero: True to pad the number to the specified precision with zeros
  --------------------------------------------------------------------*/
String::String(double val, double prec, bool padZero) {
	int32_t dec = std::max(0, static_cast<int32_t>(-log10(prec)));
	m_string = std::format("{:.{}f}", val, dec);
		//Padding is added by default, so strip it when unwanted
	if (!padZero) {
		if (auto pointPos = m_string.find('.'); pointPos != npos) {
			auto lastZero = m_string.find_last_not_of("0");
			m_string = substr(0, (pointPos == lastZero) ? pointPos : lastZero + 1);
		}
	}
} //String::String


/*--------------------------------------------------------------------
	Copy constructor

	source: The object to be copied
	startPos: The position to begin copying from
	howMany: The number of characters to copy
  --------------------------------------------------------------------*/
String::String(const String& source) {
	m_string = source.m_string;
} //String::String


/*--------------------------------------------------------------------
	Move constructor
 
	source: The object to move
  --------------------------------------------------------------------*/
String::String(String&& source) noexcept :
		m_string{std::move(source.m_string)} {
}

//MARK: - Static functions

/*--------------------------------------------------------------------
	Get the number of bytes in the specified text (counting only valid UTF-8 characters)
 
	text: The source text
	howMany: The number of bytes in the text (nullopt = null-terminated)
	charCount: The maximum number of (encoded) chars to seek
	format: The text data format
 
	return: The number of bytes in the text containing valid characters
  --------------------------------------------------------------------*/
String::size_type String::getValidByteCount(const char* text, sizeOption howMany, sizeOption charCount, DataFormat format) {
	bool isOpen = !charCount;
		//Detect empty strings or null requests
	if ((howMany == 0) || (!isOpen && (*charCount < 1)))
		return 0;
	const auto* endPos = text;
	do {
			//Get the size of the next char
		if (auto nextLen = String::getCharacterByteCount(endPos, howMany, format); nextLen && (nextLen > 0)) {
			endPos += *nextLen;
				//If the source is byte-limited, ensure the remaining count is updated
			if (howMany)
				*howMany -= *nextLen;
		} else
			break;
	} while ((*endPos != 0) && (isOpen || --(*charCount)));
	return static_cast<String::size_type>(endPos - text);
} //String::getValidByteCount


/*--------------------------------------------------------------------
	Get the width of a specified character in bytes
 
	text: The source text
	howMany: The number of bytes in the array
	format: The text data format
 
	return: The character width in bytes (nullopt on failure, either null char or bad encoding)
  --------------------------------------------------------------------*/
String::sizeOption String::getCharacterByteCount(const char* text, sizeOption howMany, DataFormat format) {
	if (howMany == 0)
		return 0;
	switch (format.encoding) {
		case UTF8: {
			if (*text == 0)
				return 0;
			size_type result = 1;
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
			size_type size = isWithinBMP(uniChar) ? 2 : 4;
			return (howMany && (howMany < size)) ? std::nullopt : sizeOption(size);
		}
		case UTF32:
			if (howMany && (howMany < 4))
				return std::nullopt;
			auto uniChar = *(reinterpret_cast<const char32_t*>(text));
			if (uniChar == 0)
				return 0;
			return isValidUnicode(uniChar) ? sizeOption(4) : std::nullopt;
	}
	return 0;
} //String::getCharacterByteCount


/*--------------------------------------------------------------------
	Get the number of valid characters found at a specified address
 
	text: The source text
	howMany: The number of bytes in the array (nullopt = null-terminated)
	format: The text data format
 
	return: The nummber of characters found
  --------------------------------------------------------------------*/
String::sizeOption String::getCharacterCount(const char* text, sizeOption howMany, DataFormat format) {
	if (howMany == 0)
		return 0;
	size_type totalChars = 0;
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
} //String::getCharacterCount


/*--------------------------------------------------------------------
	Get a unicode (UTF-32) character from a specified source
 
	text: The source text
	howMany: The number of bytes in the text
			@param format The source data format
 
	return: The unicode char paired with the number of bytes consumed from the source (0 = no valid char found)
  --------------------------------------------------------------------*/
std::pair<char32_t, String::size_type> String::getUnicodeChar(const char* text, sizeOption howMany, DataFormat format) {
	switch (format.encoding) {
		case UTF8:
			return getUTF32CharFromUTF8(text, howMany.value_or(possibleCharWidth));
		case UTF16: {
			auto source = reinterpret_cast<const char16_t*>(text);
			auto result = getUTF32CharFromUTF16(source, format.isBigEndian, howMany.value_or(possibleCharWidth));
			return result;
		}
		case UTF32: {
			std::pair<char32_t, String::size_type> result{0, 0};
			if (howMany && (*howMany < sizeof(char32_t)))
				return result;
			auto source = reinterpret_cast<const char32_t*>(text);
			result.first = source[0];
			if (isValidUnicode(result.first)) {
				Memory::byteSwap(&result.first, 1, format.isBigEndian);
				result.second = sizeof(char32_t);
			} else
				result.second = 0;	//Invalid UTF32 code point
			return result;
		}
		case ascii: case ISO8859_1:
			std::pair<char32_t, String::size_type> result{0, 0};
			if (howMany && (*howMany < 1))
				return result;
			result.first = static_cast<char32_t>(text[0]);
			result.second = 1;
			return result;
	}
	return {};
} //String::getUnicodeChar

	
/*--------------------------------------------------------------------
	Get a UTF-8 string from a UTF-32 source
 
	text: The source text
	isBigEndian: True if byte ordering is big-endian
	howMany: The number of 32-bit code points in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of  code points must be valid (ignored if howMany = nullopt)
 
	return: The UTF-8 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<String> String::fromUnicode(const char32_t*& text, bool isBigEndian, String::sizeOption howMany, bool isCountRequired) {
	if (howMany == 0)
		return std::optional(String{});	//An empty string is not an error, so we don't return std::nullopt
	String result;
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
		String::size_type offset = 3;
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
} //String::fromUnicode


/*--------------------------------------------------------------------
	Get a UTF-32 string from a UTF-8 source
 
	text: The source text (advances to the byte beyond the last counted character)
	howMany: The number of bytes in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of bytes must be valid (ignored if howMany = nullopt)
 
	return: The unicode code point for the specified chars (nullopt on failure)
  --------------------------------------------------------------------*/
std::optional<std::u32string> String::toUnicode(const char*& text, String::sizeOption howMany, bool isCountRequired) {
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
} //String::toUnicode


/*--------------------------------------------------------------------
	Get a UTF-32 string from a UTF-16 (16-bit) source
 
	text: The source text
	isBigEndian: True if byte ordering is big-endian
	howMany: The number of words (16-bit values) in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of words must be valid (ignored if howMany = nullopt)
 
	return: The UTF-32 string read from the UTF-16 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<std::u32string> String::fromUTF16(const char16_t*& text, bool isBigEndian, String::sizeOption howMany, bool isCountRequired) {
	std::u32string uniString;
	if (howMany == 0)
		return std::optional(uniString);	//An empty string is not an error, so we don't return std::nullopt
	auto charsRemaining = sizeof(char16_t) * howMany.value_or(possibleCharWidth / sizeof(char16_t));
	while (*text != 0) {
		auto nextChar = getUTF32CharFromUTF16(text, isBigEndian, charsRemaining);
		if (nextChar.second == 0)
			break;	//Source is consumed or bad encoding
		uniString += nextChar.first;
		if (howMany)
			charsRemaining -= nextChar.second;
	}
	return (isCountRequired && (howMany > 0)) ? std::nullopt : std::optional(uniString);
} //String::fromUTF16


/*--------------------------------------------------------------------
	Get a UTF-16 string from a UTF-32 source
 
	text: The source text
	howMany: The number of code points in the text (nullopt = null-terminated)
	isCountRequired: True if the specified number of code points must be valid (ignored if howMany = nullopt)
 
	return: The UTF-16 string read from the UTF-32 source (nullopt on error, including failure to meet isCountRequired condition)
  --------------------------------------------------------------------*/
std::optional<std::u16string> String::toUTF16(const char32_t*& text, String::sizeOption howMany, bool isCountRequired) {
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
} //String::toUTF16

//MARK: - Conversion operators

#ifndef __CLR_VER
/*--------------------------------------------------------------------
	Conversion to std::u16string
 
	return: The equivalent std::u16string (UTF-16)
  --------------------------------------------------------------------*/
String::operator std::u8string() const {
	return std::u8string{reinterpret_cast<const char8_t*>(data())};
} //String::operator std::u8string
#endif


/*--------------------------------------------------------------------
	Conversion to std::u16string
 
	return: The equivalent std::u16string (UTF-16)
  --------------------------------------------------------------------*/
String::operator std::u16string() const {
	const auto* text = data();
	if (auto uniString = String::toUnicode(text); uniString) {
		const char32_t* text32 = uniString->data();
		if (auto uniString16 = String::toUTF16(text32); uniString16)
			return *uniString16;
	}
	return std::u16string{};
} //String::operator std::u16string


/*--------------------------------------------------------------------
	Conversion to std::u32string
 
	return: The equivalent std::u32string (UTF-32)
  --------------------------------------------------------------------*/
String::operator std::u32string() const {
	const auto* text = data();
	auto uniString = String::toUnicode(text);
	return uniString ? *uniString : std::u32string{};
} //String::operator std::u32string

// MARK: - Operators

/*--------------------------------------------------------------------
	Subscript operator
	
	index: The required character position (unchecked - out of bounds behaviour undefined)
	
	return: The character at the specified index
  --------------------------------------------------------------------*/
char32_t String::operator[](size_type index) const {
	auto offsets = getByteOffsets(data(), index, 1);
	auto source = data() + offsets->first;
	return getUTF32CharFromUTF8(source, offsets->second).first;
} //String::operator[]


/*--------------------------------------------------------------------
	Assignment operator

	source: The object to copy

	return: A reference to this
  --------------------------------------------------------------------*/
String& String::operator= (const String& source) {
	if (&source == this)
		return *this;
	return assign(source);
} //String::operator=


/*--------------------------------------------------------------------
	Move assignment operator
 
	source: The object to move
 
	return: A reference to this
  --------------------------------------------------------------------*/
String& String::operator= (String&& source) noexcept {
	m_string = std::move(source.m_string);
	return *this;
} //String::operator=


/*--------------------------------------------------------------------
	Assignment operator

	source: A pointer to a char array

	return: A reference to this
  --------------------------------------------------------------------*/
String& String::operator= (const char* source) {
	m_string.clear();
	makeString(m_string, source);
	return *this;
} //String::operator=

//MARK: - Functions (const)

/*--------------------------------------------------------------------
	Return the number of bytes this string can contain

	return: The number of bytes this string can contain
  --------------------------------------------------------------------*/
String::size_type String::capacity() const {
	return static_cast<String::size_type>(m_string.capacity());
} //String::capacity


/*--------------------------------------------------------------------
	Get the number of characters in the string
 
	return: The number of characters in the string
  --------------------------------------------------------------------*/
String::size_type String::size() const {
	return getStringLength(m_string.data());
} //String::size


/*--------------------------------------------------------------------
	Return the number of bytes consumed by this string

	howMany: The number of chars to count (nullopt = for full length)
 
	return: The number of bytes consumed by the string (NB: String are null terminated, but this is not included in the data length)
  --------------------------------------------------------------------*/
String::size_type String::dataSize(sizeOption howMany) const {
	if (!howMany)
		return static_cast<String::size_type>(m_string.size());
	if (auto byteCount = getByteCountCharLimited(m_string.data(), howMany); byteCount)
		return *byteCount;
	return 0;
} //String::dataSize


/*--------------------------------------------------------------------
	Subscript operator
	
	index: The required character position (unchecked - out of bounds behaviour undefined)
	
	return: The character at the specified index
  --------------------------------------------------------------------*/
char32_t String::at(size_type index) const {
	auto offsets = getByteOffsets(data(), index, 1);
	if (!offsets)
		throw std::out_of_range("");
	auto source = data() + offsets->first;
	return getUTF32CharFromUTF8(source, offsets->second).first;
} //String::at


/*--------------------------------------------------------------------
	Apply a function to specified characters in the string
 
	func: The character function (the returned value is ignored)
  --------------------------------------------------------------------*/
void String::forEach(const Function& func) const {
	BufferIn{*this}.forEach([&](char32_t incoming) -> std::optional<char32_t> { return func(incoming); });
} //String::forEach


/*--------------------------------------------------------------------
	Get a specified segment of this string

	startPos: The first character of the segment
	howMany: The number of characters in the segment (nullopt for full length)

	return: The requested string segment
  --------------------------------------------------------------------*/
String String::substr(size_type startPos, sizeOption howMany) const {
	auto offsets = getByteOffsets(data(), startPos, howMany);
	if (!offsets)
		return String{};
	return String{m_string.substr(offsets->first, offsets->second)};
} //String::substr


/*--------------------------------------------------------------------
	Create an uppercase version of the string
 
	return: An uppercase version of the string
  --------------------------------------------------------------------*/
String String::uppercase() const {
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
#endif
		*i = std::toupper(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8"));	//NB: Need to consider if locale is appropriate
	}
	return String(uniString);
} //String::lowercase


/*--------------------------------------------------------------------
	Create a lowercase version of the string
 
	return: A lowercase version of the string
  --------------------------------------------------------------------*/
String String::lowercase() const {
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
#endif
		*i = std::tolower(static_cast<wchar_t>(*i), std::locale("en_US.UTF-8"));	//NB: Need to consider if locale is appropriate
	}
	return String(uniString);
} //String::lowercase


/*--------------------------------------------------------------------
	Write this string to a buffer
 
	buffer: The destination buffer
	format: The required output format
	isNullAdded: True to add a terminating null
	howMany: The number of characters to write (nullopt for all)
	maxBytes: The maximum number of bytes to write
 
	return: A reference to the destination
  --------------------------------------------------------------------*/
const BufferOut& String::writeTo(const BufferOut& buffer, DataFormat format, bool isNullAdded, sizeOption howMany, sizeOption maxBytes) const {
	switch (format.encoding) {
		case UTF8: case ascii: case ISO8859_1:
			return writeUTF8(buffer, isNullAdded, howMany, maxBytes);
		case UTF16:
			return writeUTF16(buffer, isNullAdded, format.isBigEndian, howMany, maxBytes);
		case UTF32:
			return writeUTF32(buffer, isNullAdded, format.isBigEndian, howMany, maxBytes);
	}
	return buffer;
} //String::writeTo


/*--------------------------------------------------------------------
	Write this string to a buffer (as internally encoded)

	buffer: The destination buffer
	isNullAdded: True to add a terminating null
	howMany: The number of characters to write (nullopt for all)
	maxBytes: The maximum number of bytes the destination can hold (including terminating null - nullopt for unlimited)

	return: A reference to the destination
  --------------------------------------------------------------------*/
const BufferOut& String::writeUTF8(const BufferOut& buffer, bool isNullAdded, sizeOption howMany, sizeOption maxBytes) const {
	if ((howMany == 0) || (maxBytes == 0))
		return buffer;
	if (!m_string.empty()) {
		if (howMany) {
			if (auto charBytes = getByteCountCharLimited(data(), howMany); charBytes && (!maxBytes || (maxBytes > *charBytes)))
				maxBytes = *charBytes;
		}
			//If the buffer has a byte limit, use it if a maximum has not been specified or is too large
		if (auto bufferMax = buffer.maxSize(); bufferMax && (!maxBytes || (*bufferMax < *maxBytes)))
			maxBytes = *bufferMax;
		String::size_type byteCount = maxBytes ?
				getValidByteCount(m_string.data(), *maxBytes - (isNullAdded ? 1 : 0)) :
				dataSize();
		buffer.write(data(), byteCount);
	}
	if (isNullAdded)
		buffer.write(0);
	return buffer;
} //String::writeUTF8


/*--------------------------------------------------------------------
	Write this string as UTF-16 to a buffer
 
	buffer: The destination buffer
	isNullAdded: True to add a terminating null
	isBigEndian: True if byte ordering is big-endian
	howMany: The number of characters to write (nullopt for all)
	maxBytes: The maximum number of chars the destination can hold (including terminating null - nullopt for full length)
 
	return: A reference to the destination
  --------------------------------------------------------------------*/
const BufferOut& String::writeUTF16(const BufferOut& buffer, bool isNullAdded, bool isBigEndian, sizeOption howMany, sizeOption maxBytes) const {
	if ((howMany == 0) || (maxBytes == 0))
		return buffer;
	const auto* text = data();
	if (auto uniString = String::toUnicode(text, howMany); uniString) {
		const char32_t* text32 = uniString->data();
		if (auto uniString16 = String::toUTF16(text32); uniString16) {
				//If the buffer has a byte limit, use it if a maximum has not been specified or is too large
			if (auto bufferMax = buffer.maxSize(); bufferMax && (!maxBytes || (*bufferMax < *maxBytes)))
				maxBytes = *bufferMax;
				//NB: When a terminating null is required, deduct this when a max size for the destination is specified
			String::size_type byteCount = maxBytes ?
					getValidByteCount(reinterpret_cast<char*>(uniString16->data()), *maxBytes - (isNullAdded ? sizeof(char16_t) : 0), std::nullopt, UTF16) :
							(uniString->size() * sizeof(char16_t));
				//Byte-swap the data as required (no action if platform endianess matches requirement)
			Memory::byteSwap(uniString16->data(), byteCount / sizeof(char16_t), isBigEndian);
			buffer.write(reinterpret_cast<const char*>(uniString16->data()), byteCount);
		}
	}
	if (isNullAdded)
		buffer.writeBinary(char16_t());
	return buffer;
} //String::writeUTF16


/*--------------------------------------------------------------------
	Write this string as UTF-32 to a buffer
 
	buffer: The destination buffer
	isNullAdded: True to add a terminating null
	isBigEndian: True if byte ordering is big-endian
	howMany: The number of characters to write (nullopt for all)
	maxBytes: The maximum number of chars the destination can hold (including terminating null - nullopt for full length)
 
	return: A reference to the destination
  --------------------------------------------------------------------*/
const BufferOut& String::writeUTF32(const BufferOut& buffer, bool isNullAdded, bool isBigEndian, sizeOption howMany, sizeOption maxBytes) const {
	if ((howMany == 0) || (maxBytes == 0))
		return buffer;
	const auto* text = data();
	auto uniString = String::toUnicode(text);
	if (uniString) {
			//If the buffer has a byte limit, use it if a maximum has not been specified or is too large
		if (auto bufferMax = buffer.maxSize(); bufferMax && (!maxBytes || (*bufferMax < *maxBytes)))
			maxBytes = *bufferMax;
			//NB: When a terminating null is required, deduct this when a max size for the destination is specified
		String::size_type byteCount = maxBytes ?
				getValidByteCount(m_string.data(), *maxBytes - (isNullAdded ? sizeof(char32_t) : 0), std::nullopt, UTF32) :
				(uniString->size() * sizeof(char32_t));
			//Byte-swap the data as required (no action if platform endianess matches requirement)
		Memory::byteSwap(uniString->data(), byteCount / sizeof(char32_t), isBigEndian);
		buffer.write(reinterpret_cast<const char*>(uniString->data()), byteCount);
	}
	if (isNullAdded)
		buffer.writeBinary(char32_t());
	return buffer;
} //String::writeUTF32


/*--------------------------------------------------------------------
	Determine if the string is entirely alphanumeric
 
	startPos: The position to checking from
	howMany: The number of characters to check (nullopt = to end)

	return: True if the string is alphanumeric
  --------------------------------------------------------------------*/
bool String::isAlphaNumeric(size_type startPos, sizeOption howMany) const {
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
} //String::isAlphaNumeric


/*--------------------------------------------------------------------
	Determine if the string is entirely letters
  
	startPos: The position to checking from
	howMany: The number of characters to check (nullopt = to end)

	return: True if the string is letters
  --------------------------------------------------------------------*/
bool String::isAlpha(size_type startPos, sizeOption howMany) const {
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
} //String::isAlpha


/*--------------------------------------------------------------------
	Determine if the string is entirely numbers
 
	startPos: The position to checking from
	howMany: The number of characters to check (nullopt = to end)
 
	return: True if the string is numbers
  --------------------------------------------------------------------*/
bool String::isNumeric(size_type startPos, sizeOption howMany) const {
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
} //String::isNumeric


/*--------------------------------------------------------------------
	Three-way comparison to a reference string
 
	ref: The string to compare this to
 
	return: The relationship between this and ref (less, equal, greater)
  --------------------------------------------------------------------*/
std::strong_ordering String::compare(const String& ref) const {
	std::u32string myString{*this}, refString{ref};
	return myString <=> refString;
} //String::compare


/*--------------------------------------------------------------------
	Find the specified string within this

	toFind: The string to find
	startPos: The character to start searching from

	return: The index where a match is found (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::find(const String& toFind, size_type startPos) const {
	auto startByte = (startPos == 0) ? 0 : getByteCountCharLimited(m_string.data(), startPos, true);
	if (!startByte)
		return std::nullopt;
	auto foundPos = m_string.find(toFind.m_string, *startByte);
	if (foundPos == npos)
		return std::nullopt;
	return getCharacterCount(m_string.data(), foundPos);
} //String::find


/*--------------------------------------------------------------------
	Find the specified string within this using a filter
 
	filter: The string filter
 
	return: The index where a match is found (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::findIf(const Filter& filter) const {
	size_type index = 0;
	if (!BufferIn{*this}.findIf([&](char32_t incoming) {
		if (filter(incoming))
			return true;
		++index;
		return false;
	}))
		return std::nullopt;
	return index;
} // String::findIf


/*--------------------------------------------------------------------
	Find the first character which is in a specified string

	toFind: A string of characters to find
	startPos: The character to begin searching from

	return: The index of the first matching character (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::findFirstOf(const String& toFind, size_type startPos) const {
	auto splitString = splitSingleChars(toFind);
	if (!splitString)
		return std::nullopt;
	auto firstPos = npos;
	auto startByte = (startPos == 0) ? 0 : getByteCountCharLimited(m_string.data(), startPos, true);
	if (!startByte)
		return std::nullopt;
	bool isFirst = true;
	for (auto& iter : *splitString) {
		auto nextPos = isFirst ? m_string.find_first_of(iter.m_string, *startByte) : m_string.find(iter.m_string, *startByte);
		isFirst = false;
		if (nextPos < firstPos) {
			firstPos = nextPos;
			if (firstPos == 0)
				return 0;
		}
	}
	return (firstPos == npos) ? std::nullopt : getCharacterCount(data(), firstPos);
} //String::findFirstOf


/*--------------------------------------------------------------------
	Find the first character not in a specified string

	toFind: A string of characters to not find
	startPos: The character to begin searching from

	return: The index of the first non-matching character (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::findFirstNotOf(const String& toFind, size_type startPos) const {
	auto splitString = splitSingleChars(toFind);
	if (!splitString)
		return std::nullopt;
	auto charBytes = collectCharByteCount(data());
	if (!charBytes)
		return std::nullopt;
	std::vector<size_type> minPos(splitString->size());
	size_type startByte = 0;
	for (int sizeIndex = 0; sizeIndex < charBytes->size(); ++sizeIndex) {
		auto size = (*charBytes)[sizeIndex];
		if (sizeIndex < startPos) {
			startByte += size;
			continue;
		}
		size_type lowest = npos;
		for (int charIndex = 0; charIndex < splitString->size(); ++charIndex) {
			if (minPos[charIndex] > startByte)
				continue;
			minPos[charIndex] = (charIndex == 0) ?
					m_string.find_first_of((*splitString)[charIndex].m_string, startByte) :
					m_string.find((*splitString)[charIndex].m_string, startByte);
			if (minPos[charIndex] < lowest)
				lowest = minPos[charIndex];
		}
		if (lowest > startByte)
			break;
		startByte += size;
		++startPos;
	}
	return startPos < length() ? sizeOption(startPos) : std::nullopt;
} //String::findFirstNotOf


/*--------------------------------------------------------------------
	Find the last character in this string which is in the specified string

	toFind: A string of characters to find
	lastPos: The position of the last character to be compared

	return: The index of the last matching character (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::findLastOf(const String& toFind, sizeOption lastPos) const {
	auto splitString = splitSingleChars(toFind);
	if (!splitString)
		return std::nullopt;
	auto firstPos = npos;
	auto startByte = (lastPos == npos) ? npos : getByteCountCharLimited(m_string.data(), lastPos, false);
	bool isFirst = true;
	for (auto& iter : *splitString) {
		auto nextPos = isFirst ? m_string.find_last_of(iter.m_string, *startByte) : m_string.rfind(iter.m_string, *startByte);
		isFirst = false;
		if (nextPos < firstPos) {
			firstPos = nextPos;
			if (firstPos == 0)
				return 0;
		}
	}
	return (firstPos == npos) ? std::nullopt : getCharacterCount(data(), firstPos);
} //String::findLastOf


/*--------------------------------------------------------------------
	Find the last character in this string which is not in the specified string

	toFind: A string of characters not to find
	lastPos: The character to begin searching from (nullopt for string end)

	return: The index of the last non-matching character (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::findLastNotOf(const String& toFind, sizeOption lastPos) const {
		//Divide the chars to find into single-byte and multi-byte chars
	auto splitString = splitSingleChars(toFind);
	if (!splitString)
		return std::nullopt;
		//Calculate the number of bytes for each character in the string to be searched (allows us to iterate backward)
	auto charBytes = collectCharByteCount(data(), lastPos);
	if (!charBytes)
		return std::nullopt;
		//If the character to start from is not specified, use the last character in the string
	if ((lastPos == std::nullopt) || (lastPos == npos))
		lastPos = charBytes->size() - 1;
		//Add up all the bytes in the string (total number of bytes used by the string)
	auto lastByte = std::reduce(charBytes->begin(), charBytes->end());
	std::vector<size_type> maxPos(splitString->size(), lastByte);
	for (auto iter = charBytes->rbegin(); iter != charBytes->rend(); ++iter, --(*lastPos)) {
		auto charStart = lastByte - *iter;
		--lastByte;
		if (*iter == 1) {
			if ((*splitString)[0].m_string.find(m_string[charStart]) == npos)
				return lastPos;
		} else {
			String::sizeOption highest;
			for (auto index = 1; index < splitString->size(); ++index) {
				if ((maxPos[index] < charStart) || (maxPos[index] == npos))
					continue;
				maxPos[index] = m_string.rfind((*splitString)[index].m_string, lastByte);
				if ((maxPos[index] != npos) && (!highest || (maxPos[index] > *highest)))
					highest = maxPos[index];
			}
			if (highest && (*highest < charStart))
				return lastPos;
		}
		lastByte = charStart;
		if (lastPos == 0)
			break;
	}
	return std::nullopt;
} //String::findLastNotOf


/*--------------------------------------------------------------------
	Find the specified string searching backwards

	toFind: The string to find
	lastPos: The position of the last character to be compared

	return: The index of a matching string (nullopt = not found)
  --------------------------------------------------------------------*/
String::sizeOption String::rfind(const String& toFind, sizeOption lastPos) const {
	auto endChar = getByteCountCharLimited(data(), lastPos, true);
	if (!endChar)
		return std::nullopt;
	auto foundPos = m_string.rfind(toFind.m_string, *endChar);
	return (foundPos == npos) ? std::nullopt : getCharacterCount(data(), foundPos);
} //String::rfind

//MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Reserve the specified number of bytes for the string to grow into

	newSize: The number of bytes to reserve
  --------------------------------------------------------------------*/
void String::reserve(size_type newSize) {
	m_string.reserve(newSize);
} //String::reserve


/*--------------------------------------------------------------------
	Resize the string and (when grown) pad with the specified character
 
	newSize: The required number of characters
	padding: A padding character (when the sring grows - only the first character is used)
  --------------------------------------------------------------------*/
void String::resize(size_type newSize, const String& padding) {
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
} //String::resize


/*--------------------------------------------------------------------
	Apply a function to specified characters in the string
 
	func: The character function (the returned value replaces the input character)
  --------------------------------------------------------------------*/
void String::forEach(const Function& func) {
	String result;
	BufferIn{*this}.forEach([&](char32_t incoming) -> std::optional<char32_t> { return func(incoming); }, &result);
	m_string = std::move(result.m_string);
} //String::forEach


/*--------------------------------------------------------------------
	Assign a specified string to this
 
	source: The string to assign
 
	return: A reference to this
  --------------------------------------------------------------------*/
String& String::assign(const String& source) {
	m_string = source.m_string;
	return *this;
} //String::assign


/*--------------------------------------------------------------------
	Assign a specified string to this
 
	source: The character array to be copied
	byteCount: The maximum number of bytes in the array (nullopt = null-terminated)
	charCount: The maximum number of (encoded) characters to read (nullopt = as byteCount limit)
	format: The source text data format

	return: The number of bytes assigned from the source
  --------------------------------------------------------------------*/
String::size_type String::assign(const char* source, sizeOption byteCount, sizeOption charCount, DataFormat format) {
	m_string.clear();
	return makeString(m_string, source, byteCount, charCount, format);
} //String::assign


/*--------------------------------------------------------------------
	Append the specified string to this

	source: The string to append

	return: A reference to this
  --------------------------------------------------------------------*/
String& String::append(const String& source) {
	m_string.append(source.m_string);
	return *this;
} //String::append


/*--------------------------------------------------------------------
	Insert a string into this
	
	pos: The insertion point
	source: The string to insert
	start: The start point in the source string
	howMany: The number of chars to insert (nullopt inserts all)
	
	return: A reference to this
  --------------------------------------------------------------------*/
String& String::insert(size_type pos, const String& source, size_type start, sizeOption howMany) {
	return replace(pos, 0, source, start, howMany);
} //String::insert


/*--------------------------------------------------------------------
	Replace a specified string segment with another string
	
	pos: The position to begin replacing
	num: The number of chars to replace
	source: The replacement string
	start: The start point in the replacement string
	howMany: The number of chars to extract from the replacement string (nullopt inserts all)
	
	return: A reference to this
  --------------------------------------------------------------------*/
String& String::replace(sizeOption pos, sizeOption num, const String& source, size_type start, sizeOption howMany) {
	String toAppend(source.substr(start, howMany));
	size_type myLength = length();
	if (!pos || (pos >= myLength))
		return append(toAppend);
	if (num && ((*pos + *num) < myLength))
		toAppend += substr(*pos + *num);
	resize(*pos);
	return append(toAppend);
} //String::replace


/*--------------------------------------------------------------------
	Replace all instances of a specified expression
 
	toFind: The expression to be replaced
	replacement: The replacement expression
 
	return: A reference to this
 --------------------------------------------------------------------*/
String& String::replaceAll(const String& toFind, const String& replacement) {
	String result;
	BufferIn processor(*this);
	while (processor.find(toFind, &result, true))
		result.append(replacement);
	*this = std::move(result);
	return *this;
} //String::replaceAll


/*--------------------------------------------------------------------
	Replace all chars passing a specified filter
 
	filter: The filter for chars to be replaced
	replacement: The replacement expression
 
	return: A reference to this
 --------------------------------------------------------------------*/
String& String::replaceIf(const Filter& filter, const String& replacement) {
	String result;
	BufferIn processor(*this);
	while (processor.findIf(filter, &result, true))
		result.append(replacement);
	*this = std::move(result);
	return *this;
} //String::replaceAll


/*--------------------------------------------------------------------
	Replace any instances of specified characters
 
	charsToFind: The characters to be replaced (NB: each character is treated independently - the order does not matter)
	replacement: The replacement expression
 
	return: A reference to this
 --------------------------------------------------------------------*/
String& String::replaceAnyOf(const String& charsToFind, const String& replacement) {
	String result;
	BufferIn processor(*this);
	while (processor.findFirstOf(charsToFind, &result, false, false, true))
		result.append(replacement);
	*this = std::move(result);
	return *this;
} //String::replaceAnyOf


/*--------------------------------------------------------------------
	Erase a specified range of characters from a string
	
	pos: The position to erasing from
	howMany: The number of characters to erase (nullopt to erase all)
 
	return: A reference to this
  --------------------------------------------------------------------*/
String& String::erase(size_type pos, sizeOption howMany) {
	return replace(pos, howMany, String{});
} //String::erase


/*--------------------------------------------------------------------
	Remove the last character from the string
 --------------------------------------------------------------------*/
void String::popBack() {
	if (!empty())
		erase(length() - 1);
} //String::pop_back


/*--------------------------------------------------------------------
	Pad the string with a repeated character to reach a specified length (so the existing content is flush with the right)
 
	length: The required string length
	repeat: The character to repeatedly insert until the length is met
 
	return: A reference to this
 --------------------------------------------------------------------*/
String& String::padRight(size_type length, const String& repeat) {
	if ((length <= size()) || repeat.empty())
		return *this;
	auto required = length - size(),
			fillSize = repeat.size();
	auto toInsert = required / fillSize;
	if (toInsert > 0)
		insert(0, String(toInsert, repeat.m_string));
	return *this;
} //String::padRight


/*--------------------------------------------------------------------
	Pad the string with a repeated character to reach a specified length (so the existing content is flush with the left)
 
	length: The required string length
	repeat: The character to repeatedly append until the length is met
 
	return: A reference to this
 --------------------------------------------------------------------*/
String& String::padLeft(size_type length, const String& repeat) {
	if ((length <= size()) || repeat.empty())
		return *this;
	auto required = size() - length,
			fillSize = repeat.size();
	auto toAppend = required / fillSize;
	if (toAppend > 0)
		append(String(toAppend, repeat.m_string));
	return *this;
} //String::padLeft
