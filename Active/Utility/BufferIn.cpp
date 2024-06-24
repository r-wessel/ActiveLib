/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/BufferIn.h"

#include "Active/File/File.h"
#include "Active/Utility/MathFunctions.h"
#include "Active/Utility/BufferOut.h"
#include "Active/Utility/StackBufferOut.h"
#include "Active/Utility/Defer.h"
#include "Active/Utility/Memory.h"

#include <array>

using namespace active::file;
using namespace active::math;
using namespace active::utility;

namespace {

	constexpr  Memory::size_type defaultBufferSize = 0xF000;

	constexpr  Memory::size_type stackBufferSize = 0x0400;

	constexpr  Memory::size_type possibleCharWidth = 4;
	
	using StackBuffer = StackBufferOut<stackBufferSize>;
	using enum TextEncoding;
	
		///Text formats that can be detected (in order of priority)
	std::array supportedFormats = {
		DataFormat{UTF8},
		DataFormat{UTF32, false, false},
		DataFormat{UTF32, false, true},
		DataFormat{UTF16, false, Memory::defaultEndian},
		DataFormat{UTF16, false, !Memory::defaultEndian},
	};
	
}  // namespace

void swap(BufferIn& v1, BufferIn& v2) {
	v1.swap(v2);
}


/*--------------------------------------------------------------------
	Constructor

	sourceFile: The source data file
	format: The source data format (nullopt = attempt to discover format from source)
  --------------------------------------------------------------------*/
BufferIn::BufferIn(const File& sourceFile, DataFormat::Option format) {
	setSource(sourceFile, format);
} //BufferIn::BufferIn


/*--------------------------------------------------------------------
	Constructor

	sourceMem: A source block of memory
	format: The source data format (nullopt = attempt to discover format from source)
  --------------------------------------------------------------------*/
BufferIn::BufferIn(const Memory& sourceMem, DataFormat::Option format) {
	setSource(sourceMem, format);
} //BufferIn::BufferIn


/*--------------------------------------------------------------------
	Constructor

	sourceString: The source data string
	format: The source data format (nullopt = attempt to discover format from source)
  --------------------------------------------------------------------*/
BufferIn::BufferIn(const String& sourceString, DataFormat::Option format) {
	setSource(sourceString, format);
} //BufferIn::BufferIn


/*--------------------------------------------------------------------
	Move constructor
 
	source: The object to move
  --------------------------------------------------------------------*/
BufferIn::BufferIn(BufferIn&& source) noexcept :
		m_buffer{std::exchange(source.m_buffer, nullptr)},
		m_file(std::exchange(source.m_file, nullptr)),
		m_fileCache(std::exchange(source.m_fileCache, nullptr)),
		m_readPos(std::exchange(source.m_readPos, 0)),
		m_bufferLen(std::exchange(source.m_bufferLen, 0)),
		m_remaining(std::exchange(source.m_remaining, 0)),
		m_lastRow(std::exchange(source.m_lastRow, 0)),
		m_lastColumn(std::exchange(source.m_lastColumn, 0)),
		m_foundCR(std::exchange(source.m_foundCR, false)) {
	m_format = source.m_format;
} //BufferIn::BufferIn


/*--------------------------------------------------------------------
	Read operator
 
	val: A double to read
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::operator>>(double& val) const {
	if (auto temp = readWord().toDouble(); temp)
		val = *temp;
	else {
		setState(std::ios_base::failbit);
		val = 0.0;
	}
	return *this;
} //BufferIn::operator>>

// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment with move operator
 
	source: The object to assign
 
	return: A reference to this object
  --------------------------------------------------------------------*/
BufferIn& BufferIn::operator= (BufferIn&& source) noexcept {
	if (this == &source)
		return *this;
	m_buffer = std::exchange(source.m_buffer, nullptr);
	m_file = std::exchange(source.m_file, nullptr);
	m_fileCache = std::exchange(source.m_fileCache, nullptr);
	m_readPos = std::exchange(source.m_readPos, 0);
	m_bufferLen = std::exchange(source.m_bufferLen, 0);
	m_remaining = std::exchange(source.m_remaining, 0);
	m_lastRow = std::exchange(source.m_lastRow, 0);
	m_lastColumn = std::exchange(source.m_lastColumn, 0);
	m_foundCR = std::exchange(source.m_foundCR, false);
	m_format = source.m_format;
	return *this;
} //BufferIn::operator=

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Get the total number of bytes in the source, e.g. the source file/string/memory
 
	return: The total number of bytes in the source
  --------------------------------------------------------------------*/
Memory::size_type BufferIn::sourceSize() const {
	if (m_file == nullptr)
		return m_bufferLen;
	try {
		return static_cast<Memory::size_type>(m_file->size());
	} catch(...) {
		setState(std::ios_base::failbit);
	}
	return 0;
} //BufferIn::sourceSize


/*--------------------------------------------------------------------
	Get the current read position in the source data (not the read position in the buffer)
 
	return: The read position (e.g. the read position in a source file)
  --------------------------------------------------------------------*/
Memory::size_type BufferIn::getPosition() const {
	if (m_file != nullptr) {
		try {
			return static_cast<Memory::size_type>(m_file->getPosition()) - dataSize();	//Otherwise calculate the current position in the file
		} catch(...) {
			setState(std::ios_base::failbit);
		}
	}
	return m_readPos;
} //BufferIn::getPosition


/*--------------------------------------------------------------------
	Apply a function to the buffered characters
 
	func: The character function
  --------------------------------------------------------------------*/
void BufferIn::forEach(const String::Function& func, String* pool) const {
		//Allocate a buffer for pooling data as required
	std::optional<std::u32string> dataBuffer;
	if (pool != nullptr)
		dataBuffer = std::u32string{};
	while (good()) {
		auto uniChar = getEncodedChar(true);
		if (uniChar.second == 0)
			break;	//End of stream
		if (auto processedChar = func(uniChar.first); dataBuffer && processedChar)
			dataBuffer->push_back(*processedChar);
	}
		//Write the pooled data as required
	if (dataBuffer)
		pool->append(*dataBuffer);
} //BufferIn::forEach


/*--------------------------------------------------------------------
	Find the specified string within this using a filter
 
	filter: The string filter
	pool: Optional string to collect the skipped characters (nullptr = discard)
	isFoundSkipped: True if the buffer read position should skip over the found byte
 
	return: True if a match is found
  --------------------------------------------------------------------*/
bool BufferIn::findIf(const String::Filter& filter, String* pool, bool isFoundSkipped) const {
		//Allocate a buffer for pooling data as required
	std::optional<std::u32string> dataBuffer;
	if (pool != nullptr)
		dataBuffer = std::u32string{};
	bool isFound = false;
	while (good()) {
		auto uniChar = getEncodedChar(true);
		if (uniChar.second == 0)
			break;	//End of stream
		if (filter(uniChar.first)) {
			if (!isFoundSkipped)
				rewind(uniChar.second);	//Restore the found char as required
			isFound = true;
			break;
		}
		if (dataBuffer)
			dataBuffer->push_back(uniChar.first);
	}
		//Write the pooled data as required
	if (dataBuffer)
		pool->append(*dataBuffer);
	return isFound;
} //BufferIn::findIf


/*--------------------------------------------------------------------
	Find a specified character in the buffered content (skipping over all all non-matching data)
 
	toFind: The character to find (UTF-32)
	pool: Optional string to collect the skipped characters (nullptr = discard)
	isFoundSkipped: True if the buffer read position should skip over the found byte
 
	return: True if a match is found
  --------------------------------------------------------------------*/
bool BufferIn::find(char32_t toFind, String* pool, bool isFoundSkipped) const {
	String stopString{&toFind, 1};
	return stopString.empty() ? false : seek(stopString, pool, false, false, false, false, isFoundSkipped, false);
} //BufferIn::find


/*--------------------------------------------------------------------
	Get a single char
	
	dest: The incoming char
	
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::get(unsigned char& dest) const {
	if ((bufferMin(1) > 0)) {
		dest = *(m_buffer + m_readPos);
		bumpReadPos(1);
	}
	return *this;
} //BufferIn::get


/*--------------------------------------------------------------------
	Get the next character from the buffer, encoding as necessary
 
	encodedChar: The next encoded character (empty on failure)
	isConsumed: True to consume the character bytes from the buffer
 
	return: The number of bytes consumed by the next character
  --------------------------------------------------------------------*/
Memory::size_type BufferIn::getEncodedChar(String& encodedChar, bool isConsumed) const {
	encodedChar.clear();
	if (fail())
		return 0;
	if (eof()) {
		setState(std::ios_base::failbit);	//Attempting to read from eof is an error
		return 0;
	}
	auto maxBytes = bufferMin(possibleCharWidth);
	if (maxBytes == 0)
		return 0;
	auto byteLen = encodedChar.assign(m_buffer + m_readPos, maxBytes, 1, m_format);
	if (isConsumed && (byteLen > 0)) {
		updatePosition(m_buffer[m_readPos], static_cast<uint8_t>(byteLen));
		bumpReadPos(byteLen);
	}
	return byteLen;
} //BufferIn::getEncodedChar


/*--------------------------------------------------------------------
	Get the next character from the buffer as a UTF32 char
 
	isConsumed: True to consume the character bytes from the buffer
 
	return: The character coupled with the number of bytes consumed by the next character (0 bytes = no char read)
  --------------------------------------------------------------------*/
std::pair<char32_t, Memory::size_type> BufferIn::getEncodedChar(bool isConsumed) const {
	std::pair<char32_t, Memory::size_type> result{0, 0};
	if (fail())
		return result;
	if (eof()) {
		setState(std::ios_base::failbit);	//Attempting to read from eof is an error
		return result;
	}
	auto uniChar = String::getUnicodeChar(m_buffer + m_readPos, bufferMin(possibleCharWidth), m_format);
	if (isConsumed) {
		updatePosition(m_buffer[m_readPos], static_cast<uint8_t>(uniChar.second));
		bumpReadPos(uniChar.second);
	}
	return uniChar;
} //BufferIn::getEncodedChar


/*--------------------------------------------------------------------
	Get a single char in a string (supporting multi-byte chars)
	
	dest: The incoming char (supporting unicode)
	
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::get(String& dest) const {
	String incoming;
	if (getEncodedChar(incoming) > 0)
		dest.append(incoming);
	return *this;
} //BufferIn::get


/*--------------------------------------------------------------------
	Read a stream of bytes

	dest: The data destination
	howMany: The number of bytes to read (contains the number of bytes actually read on return)

	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::read(char* dest, Memory::size_type& howMany) const {
	if ((dest == nullptr) || (howMany == 0))
		return *this;
	Memory::size_type toRead = howMany;
	howMany = 0;
	Memory::size_type batchSize = minVal(getCapacity(), toRead);
	while ((toRead > 0) && good()) {
		Memory::size_type thisBatch = std::min(bufferMin(batchSize), toRead);
		if (thisBatch == 0)
			break;
		std::copy(m_buffer + m_readPos, m_buffer + m_readPos + thisBatch, dest + howMany);
		bumpReadPos(thisBatch);
		howMany += thisBatch;
		toRead -= thisBatch;
	}
	return *this;
} //BufferIn::read


/*--------------------------------------------------------------------
	Read a word (starting from the first non-whitespace char and up to next whitespace character)
 
	division: The dividing character(s)
 
	return: The word read (empty if not found)
  --------------------------------------------------------------------*/
String BufferIn::readWord(const String& division) const {
	String incoming;
	if (findFirstNotOf(division))
		findFirstOf(division, &incoming);
	return incoming;
} //BufferIn::readWord


/*--------------------------------------------------------------------
	Read words from the buffer
 
	howMany: Optional limit on the number of words to read (nullopt = to end of buffer)
	division: The dividing character(s)
 
	return: The found words
  --------------------------------------------------------------------*/
std::vector<String> BufferIn::readWords(Memory::sizeOption howMany, const String& division) const {
	std::vector<String> result;
	bool isOpen = !howMany;
	while (good() && (isOpen || (result.size() < *howMany))) {
		if (auto nextWord = readWord(division); !nextWord.empty())
			result.emplace_back(nextWord);
		else
			break;
	}
	return result;
} //BufferIn::readWords


/*--------------------------------------------------------------------
	Get a string of a specified number of characters
 
	dest: The incoming string
	howMany: The number of whole characters to get (nullopt to read all)
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::getString(String& dest, String::sizeOption howMany) const {
	String encodedChar;
	auto toRead = howMany.value_or(0);
	bool isOpen = !howMany;
		//Minimise reallocations of string
	dest.reserve(dest.dataSize() + (howMany ? *howMany : getSupplyCount()));
	while ((isOpen || toRead--) && getEncodedChar(encodedChar))
		dest.append(encodedChar);
	return *this;
} //BufferIn::getString


/*--------------------------------------------------------------------
	Get a single line (terminating at any known line ending)
	
	line: The incoming line
	keepStop: True to keep the line terminator(s)
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::getLine(String& line, bool keepStop) const {
	seek(String::allLineEnding, &line, true, false, false, false, true, keepStop);
	return *this;
} //BufferIn::getLine


/*--------------------------------------------------------------------
	Skip a specified number of bytes forward in the input source
 
	howMany: The number of bytes to skip forward
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::skip(Memory::size_type howMany) const {
		//In case 'len' is bigger than the buffer size, we may need to bufferMin the buffer multiple times and skip the buffer size each time
	while (good() && (howMany > 0)) {
		auto batchSize = bufferMin(minVal(howMany, getCapacity()));
		bumpReadPos(batchSize);
		howMany -= batchSize;
	}
	return *this;
} //BufferIn::skip


/*--------------------------------------------------------------------
	Rewind a specified number of bytes backward in the input source
 
	howMany: The number of bytes to rewind back
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferIn& BufferIn::rewind(Memory::size_type howMany) const {
	if (howMany < 1)
		return *this;
	unsetState(std::ios_base::eofbit);
	if (m_readPos >= howMany)
		m_readPos -= howMany;
	else {
		if (m_file == nullptr) {
			setState(std::ios_base::failbit);
		} else {
				//Attempt to move the file position to start reading from the start of the rewound position
			try {
				auto currentPosition = getPosition();
				if (howMany > currentPosition)
					setState(std::ios_base::failbit);	//Attempting to move before the file start
				else if (good())
					setPosition(currentPosition - howMany);
			} catch(...) {
				setState(std::ios_base::failbit);
			}
			if (!good())
				m_remaining = 0;
				//Reset the buffer position and length to force the data to be reloaded on the next buffer request
			m_readPos = 0;
			m_bufferLen = 0;
		}
	}
	return *this;
} //BufferIn::rewind


/*--------------------------------------------------------------------
	Set the current read position in the source data (not the read position in the buffer)
 
	pos: The read position (e.g. the read position in a source file)
  --------------------------------------------------------------------*/
void BufferIn::setPosition(Memory::size_type pos) const {
	if (pos >= sourceSize()) {
		setState(std::ios_base::failbit);
		return;
	}
	unsetState(std::ios_base::eofbit);
	if (m_file == nullptr)
		m_readPos = pos;
	else {
		try {
			m_file->setPosition(pos);
			m_remaining = m_file->size() - pos;
		} catch(...) {
			setState(std::ios_base::failbit);
			m_remaining = 0;
		}
			//Reset the buffer position and length to force the data to be reloaded on the next buffer request
		m_readPos = 0;
		m_bufferLen = 0;
	}
} //BufferIn::setPosition

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Swap this buffer with another
 
	other: The buffer to swap with this
  --------------------------------------------------------------------*/
void BufferIn::swap(BufferIn& other) {
	BufferIn temp(std::move(other));
	other = std::move(*this);
	*this = std::move(temp);
} //BufferIn::swap

/*--------------------------------------------------------------------
	Set the data source
 
	sourceFile: The source data file
	format: The source data format (nullopt = discover format from source)
  --------------------------------------------------------------------*/
void BufferIn::setSource(const File& sourceFile, DataFormat::Option format) {
	initialise(nullptr, &sourceFile, 0);
	m_format = format ? *format : discoverFormat();
} //BufferIn::setSource


/*--------------------------------------------------------------------
	Set the data source
 
	sourceMem: A source block of memory
	format: The source data format (nullopt = discover format from source)
  --------------------------------------------------------------------*/
void BufferIn::setSource(const Memory& sourceMem, DataFormat::Option format) {
	if (sourceMem)
		initialise(sourceMem.data(), nullptr, sourceMem.size());
	else
		initialise(nullptr, nullptr, 0);
	m_format = format ? *format : discoverFormat();
} //BufferIn::setSource


/*--------------------------------------------------------------------
	Set the data source
 
	sourceString: The source data string
	format: The source data format (nullopt = discover format from source)
  --------------------------------------------------------------------*/
void BufferIn::setSource(const String& sourceString, DataFormat::Option format) {
	if (!sourceString.empty())
			//NB: Source is never mutated by BufferIn so const discard is safe
		initialise(const_cast<char*>(sourceString.data()), nullptr, sourceString.dataSize());
	else
		initialise(nullptr, nullptr, 0);
	m_format = format ? *format : discoverFormat();
} //BufferIn::setSource

// MARK: - Functions (private)

/*--------------------------------------------------------------------
	Bump the read position by the specified number of bytes (and checking eof)
 
	howMany: The number of bytes to bump the read position by
  --------------------------------------------------------------------*/
void BufferIn::bumpReadPos(Memory::size_type howMany) const {
	if (eof())
		return;
	m_readPos += howMany;
	checkEndOfFile();
} //BufferIn::bumpReadPos


/*--------------------------------------------------------------------
	Check if the end of file has been reached (and set the eof flag accordingly)
 
	return: True if eof has been reached
  --------------------------------------------------------------------*/
bool BufferIn::checkEndOfFile() const {
	if (eof())
		return true;
	if ((m_remaining > 0) || (m_readPos < m_bufferLen))
		return false;
	setState(std::ios_base::eofbit);
	return true;
} //BufferIn::checkEndOfFile


/*--------------------------------------------------------------------
	Attempt to ensure a minimum number of bytes is buffered (refill if lower)
 
 	minLength: The minimum buffer length required
 
	return: The number of bytes available in the buffer
  --------------------------------------------------------------------*/
Memory::size_type BufferIn::bufferMin(Memory::size_type minLength) const {
	if (!good())
		return 0;
	if ((m_readPos + minLength) >= m_bufferLen)
		refillBuffer();
	return dataSize();
} //BufferIn::bufferMin


/*--------------------------------------------------------------------
	Refill the buffer from the current data source
	
	return: True if no errors were encountered
  --------------------------------------------------------------------*/
bool BufferIn::refillBuffer() const {
	if (!good())
		return false;
	if (eof()) {
		setState(std::ios_base::failbit);	//If we're already at eof and more data is requested, it's a fail condition
		return false;
	}
	if (m_remaining == 0) {
		if (m_readPos >= m_bufferLen)
			setState(std::ios_base::eofbit);	//If no bytes remain and we've emptied the buffer, the eof flag should be set
		return true;	//Having no remaining data or first reaching eof is not an error
	}
	bool success = true;
	Memory::size_type unused = (m_readPos >= m_bufferLen) ? 0 : m_bufferLen - m_readPos;
	if (unused > 0)
		std::copy(m_buffer + m_readPos, m_buffer + m_bufferLen, m_buffer);
	Memory::size_type toRead = minVal(m_fileCache->size() - unused, m_remaining);
	if ((toRead > 0) && (m_file != nullptr)) {
		File::size_type incoming = toRead;
		Memory readBuffer(m_buffer + unused, incoming);
		try {
			toRead = m_file->read(readBuffer, incoming);
			success = (toRead > 0);
		} catch(...) {
			success = false;
		}
	}
	m_readPos = 0;
	if (success) {
		m_remaining -= toRead;
		m_bufferLen = unused + toRead;
	} else {
		setState(std::ios_base::failbit);
		m_remaining = 0;
		m_bufferLen = unused;
	}
	return success;
} //BufferIn::refillBuffer


/*--------------------------------------------------------------------
	Seek specified char(s) in the buffered data
 
	toFind: A string containing the character(s) to find (for multiple chars, they must be contiguous but not ordered, e.g. CR/LF)
	pool: Optional string to collect the skipped characters (nullptr = discard)
	isContiguousMatch: True if contiguous matching characters should be found, e.g. any grouping of CR and LF chars
	isAllMatched: True if all chars should be found (only applicable when isContiguousMatch is true)
	isOrderedMatch: True if the matching characters must be in the specified order (only applicable when isContiguousMatch is true)
	isRepeatMatch: Used when isContiguousMatch is true, ignored for isOrderedMatch; true to match found chars multiple times, e.g. all white-chars
	isFoundSkipped: True if the buffer read position should skip over the found chars
	isFoundPooled: True if the found chars are added to the pool string
	escapeChar: An optional escape char signifying the next character is escaped
 
	return: True if a match was found
  --------------------------------------------------------------------*/
bool BufferIn::seek(const String& toFind, String* pool, bool isContiguousMatch, bool isAllMatched, bool isOrderedMatch,
					bool isRepeatMatch, bool isFoundSkipped, bool isFoundPooled, std::optional<char32_t> escapeChar) const {
	if (toFind.empty())
		return false;
		//Ensure passed parameters are mutually coherent
	if (!isContiguousMatch)
		isAllMatched = isOrderedMatch = isRepeatMatch = false;
	else if (isOrderedMatch)
		isRepeatMatch = false;
		//Build a list of unique characters to match
	std::u32string matches{toFind};
		//If we're matching chars in order, we have to compare only one at a time
	auto base = matches.begin(),
				top = isOrderedMatch ? ++matches.begin() : matches.end();
		//Allocate a buffer for data pooling as required
	std::optional<std::u32string> dataBuffer, foundBuffer;
	if (pool != nullptr)
		dataBuffer = std::u32string{};
	if (!isFoundSkipped || isFoundPooled || isOrderedMatch)
		foundBuffer = std::u32string{};
	bool isCharMatched = false,	//True if multiple characters are to be matched and at least one has been found
		 isEscaped = false;	//True if the next char is escaped
	Memory::sizeOption foundStart;
	auto seekScope = defer([&]{
		if (isCharMatched)
			std::swap(foundBuffer, dataBuffer);
			//If a found expression is to remain in the buffer, rewind to where it started
		if (!isFoundSkipped && foundStart)
			setPosition(*foundStart);
			//Ensure pooled data is collected when requested
		if (dataBuffer) {
				//If the found expression is pooled, write it
			if (isFoundPooled)
				dataBuffer->append(*foundBuffer);
			pool->append(*dataBuffer);
		}
	});
	for (;;) {
			//Attempt to read the next char (may be multi-byte)
		auto startPos = getPosition();
		auto uniChar = getEncodedChar();
		if (uniChar.second == 0)
			return isContiguousMatch && isCharMatched && !isAllMatched;
			//Once a char is read, the following code must be executed for any exit from this scope
		auto loopScope = defer([&]{
			if (dataBuffer && (uniChar.second > 0))
				dataBuffer->push_back(uniChar.first);
		});
		if (isEscaped) {
			isEscaped = false;
			continue;
		}
			//Iterate through chars to be matched
		for (auto i = base; i != top; ++i) {
			if (*i == uniChar.first) {
				if (!isCharMatched) {
					std::swap(foundBuffer, dataBuffer);
					foundStart = startPos;
				}
				isCharMatched = true;
				if (!isContiguousMatch)
					return true;	//If we're only looking for the first matching char, we're done
				if (isOrderedMatch) {
					if (++base; base == matches.end())
						return true;
					++top;
					break;
				}
				if (!isRepeatMatch) {
					i = matches.erase(i);
					if (matches.empty())
						return true;
				}
			} else {
				if (escapeChar == uniChar.first)
					isEscaped = true;
				if (isCharMatched) {
					if (!isAllMatched) {
						rewind(uniChar.second);	//Restore the non-matching char to the buffer
						uniChar.second = 0;	//And ensure it isn't written to the data pool
						return true;
					}
					if (!isOrderedMatch)
						continue;
						//See if the expression to be matched can resume from any of the chars found so far
					dataBuffer->push_back(uniChar.first);
					uniChar.second = 0;
					for (auto index = 1; index < dataBuffer->size(); ++index) {
						if (toFind.startsWith(dataBuffer->substr(index))) {
								//If we can resume, write any non-matching chars to the data pool (remember - buffers are still swapped at this point)
							if (foundBuffer)
								foundBuffer->append(dataBuffer->substr(0, index));
							dataBuffer->erase(0, index);
							continue;
						}
					}
						//If none of the chars found so far can be used for a continuous sequence, we need to resume the search from scratch
					std::swap(dataBuffer, foundBuffer);
						//If we're pooling data, the accumulated found chars should be written to the non-matching data pool
					if (dataBuffer && foundBuffer) {
						dataBuffer->append(*foundBuffer);
						foundBuffer->clear();	//And reset the 'found' chars
						foundStart.reset();
					}
					isCharMatched = false;
					base = matches.begin();
					top = ++matches.begin();
					break;
				}
			}
		}
	}
} //BufferIn::seek


/*--------------------------------------------------------------------
	Seek the first character not matching a specified set
 
	toFind: A string containing the character(s) to skip
	pool: Optional string to collect the skipped characters (nullptr = discard)
	escapeChar: An optional escape char signifying the next character is escaped
 
	return: True if a match was found
  --------------------------------------------------------------------*/
bool BufferIn::seekNot(const String& toFind, String* pool, std::optional<char32_t> escapeChar) const {
	if (toFind.empty())
		return false;
	std::u32string matches{toFind};
	auto dataPool{pool == nullptr ? std::nullopt : std::make_optional<std::u32string>()};
	bool found = false;
	do {
		auto uniChar = getEncodedChar(false);
		if (uniChar.second == 0)
			break;
		if (matches.find(uniChar.first) == std::u32string::npos) {
			found = true;
			break;
		}
		if (dataPool)
			dataPool->push_back(uniChar.first);
		updatePosition(static_cast<char>(uniChar.first & 0xFF), static_cast<uint8_t>(uniChar.second));
		bumpReadPos(uniChar.second);
	} while (good());
	if (dataPool)
		pool->assign(*dataPool);
	return found;
} //BufferIn::seekNot


/*--------------------------------------------------------------------
	Update row/col position
 
	incoming: The incoming character (test for line endings)
	size: Size of the incoming char (allowing for multi-byte chars)
  --------------------------------------------------------------------*/
void BufferIn::updatePosition(unsigned char incoming, uint8_t size) const {
	if (size == 1) {
		if (incoming == '\r') {
			++m_lastRow;
			m_lastColumn = 0;
			m_foundCR = true;
		} else {
			if (incoming == '\n') {
				if (!m_foundCR) {	//Check for Windows CRLF
					++m_lastRow;
					m_lastColumn = 0;
				}
			} else
				m_lastColumn += size;
			m_foundCR = false;
		}
	} else
		m_lastColumn += size;
} //BufferIn::updatePosition


/*--------------------------------------------------------------------
	Get the buffer capacity, i.e. the largest number of bytes it can hold
 
	return: The buffer capacity
  --------------------------------------------------------------------*/
Memory::size_type BufferIn::getCapacity() const {
		///For file buffer, the buffer capacity is the default allocation. In all other cases it's the existing buffer length
	return maxVal(m_bufferLen, defaultBufferSize);
} //BufferIn::getCapacity


/*--------------------------------------------------------------------
	Discover the source format by analysing the content
 
	return: The source format
  --------------------------------------------------------------------*/
DataFormat BufferIn::discoverFormat() {
	auto position = getPosition();
	auto row = m_lastRow, column = m_lastColumn;
	char bom[4];
	Memory::size_type len = 4;
	if (!read(bom, len))
		return DataFormat{};	//No valid content, so result inconsequential
	if (auto format = DataFormat::fromBOM(bom, len); format) {
		setPosition(position + format->second);	//Move to the first byte beyond the BOM
		m_lastColumn = column + format->second;
		return format->first;
	}
	constexpr Memory::size_type sampleSize = 0x400;
	len = std::min(sampleSize, getSupplyCount());
		//Try reading a batch of data as a specific coding type
	DataFormat result{ISO8859_1, false, false};	//Fallback if other encodings fail
	for (auto format : supportedFormats) {
		setPosition(position);
					setFormat(format);
		Memory::size_type offset = 0;
		while (offset < len) {
			auto nextChar = getEncodedChar();
			if (nextChar.second == 0)
				break;
			offset += nextChar.second;
		}
		if (offset >= len) {
			result = format;
			break;
		}
	}
	setPosition(position);
	m_lastRow = row;
	m_lastColumn = column;
	return result;
} //BufferIn::discoverFormat


/*--------------------------------------------------------------------
	Initialise the buffer source data
	  
	pointerSrc: A pointer to the source data
	fileSrc: A pointer to the source file
	dataLen: The data length (only for pointer sources)
  --------------------------------------------------------------------*/
void BufferIn::initialise(char* pointerSrc, const File* fileSrc, Memory::size_type dataLen) {
	IOBase::clear();
	m_buffer = nullptr;
	m_fileCache.reset();
	m_bufferLen = 0;
	m_readPos = 0;
	m_foundCR = false;
	m_lastRow = 1;
	m_lastColumn = 0;
	m_fileCache.reset();
	m_buffer = nullptr;
	if (fileSrc == nullptr) {
		m_file = nullptr;
		m_buffer = pointerSrc;	//Buffer the source directly
		m_bufferLen = dataLen;
		m_remaining = 0;	//All the data is in the buffer
		if (m_bufferLen == 0)
			setState(std::ios_base::eofbit);
	} else {
		m_file = fileSrc;
		try {
			dataLen = m_file->size() - m_file->getPosition();
			m_remaining = dataLen;
			m_fileCache = std::make_unique<Memory>();
			m_fileCache->resize(std::min(dataLen, defaultBufferSize));
			m_buffer = m_fileCache->data();
		} catch(...) {
			setState(std::ios_base::failbit);
			m_remaining = 0;
		}
		m_bufferLen = 0;	//The buffer starts empty
	}
} //BufferIn::initialise
