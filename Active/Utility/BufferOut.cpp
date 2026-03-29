/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/BufferOut.h"

#include "Active/File/File.h"
#include "Active/Utility/Memory.h"

#include <utility>

using namespace active;
using namespace active::file;
using namespace active;

using enum text_encoding;

namespace {
	
		///Default bounds of small cache to accumulate small buffer writes, e.g. single chars or small strings
	const size_t smallCacheBounds = 0x200;
		///Max size of data to be written to the small cache
	const size_t smallDataBounds = 0x80;
	
	/*!
	 Write this string to a buffer (as internally encoded)
	 @param string The string to write
	 @param buffer The destination buffer
	 @param isNullAdded True to add a terminating null
	 @param howMany The number of characters to write (nullopt for all)
	 @param maxBytes The maximum number of bytes the destination can hold (including terminating null - nullopt for full length)
	 @return A reference to the destination
	 */
	const BufferOut& writeUTF8(const String& string, const BufferOut& buffer, bool isNullAdded = true,
							   string_position howMany = std::nullopt,
							   string_position maxBytes = std::nullopt) {
		if ((howMany == 0) || (maxBytes == 0))
			return buffer;
		if (!string.empty()) {
			if (howMany) {
				if (auto charBytes = string_function::get_byte_count_char_limited(string.data(), howMany); charBytes && (!maxBytes || (maxBytes > *charBytes)))
					maxBytes = *charBytes;
			}
				//If the buffer has a byte limit, use it if a maximum has not been specified or is too large
			if (auto bufferMax = buffer.maxSize(); bufferMax && (!maxBytes || (*bufferMax < *maxBytes)))
				maxBytes = *bufferMax;
			String::size_type byteCount = maxBytes ? string_function::get_valid_byte_count(string.data(), *maxBytes - (isNullAdded ? 1 : 0)) : string.data_size();
			buffer.write(string.data(), byteCount);
		}
		if (isNullAdded)
			buffer.write(0);
		return buffer;
	} //writeUTF8

	/*!
	 Write this string as UTF-16 to a buffer
	 @param string The string to write
	 @param buffer The destination buffer
	 @param isNullAdded True to add a terminating null
	 @param is_big_endian True if byte ordering is big-endian
	 @param howMany The number of characters to write (nullopt for all)
	 @param maxBytes The maximum number of chars the destination can hold (including terminating null - nullopt for full length)
	 @return A reference to the destination
	 */
	const BufferOut& writeUTF16(const String& string, const BufferOut& buffer, bool isNullAdded = true,
								bool is_big_endian = text_format::defaultEndian,
								string_position howMany = std::nullopt,
								string_position maxBytes = std::nullopt) {
		if ((howMany == 0) || (maxBytes == 0))
			return buffer;
		const auto* text = string.data();
		if (auto uniString = string_function::to_unicode(text, howMany); uniString) {
			const char32_t* text32 = uniString->data();
			if (auto uniString16 = string_function::to_utf16(text32); uniString16) {
					//If the buffer has a byte limit, use it if a maximum has not been specified or is too large
				if (auto bufferMax = buffer.maxSize(); bufferMax && (!maxBytes || (*bufferMax < *maxBytes)))
					maxBytes = *bufferMax;
					//NB: When a terminating null is required, deduct this when a max size for the destination is specified
				String::size_type byteCount = maxBytes ?
				string_function::get_valid_byte_count(reinterpret_cast<char*>(uniString16->data()), *maxBytes - (isNullAdded ? sizeof(char16_t) : 0),
										  std::nullopt, UTF16) :
				(uniString->size() * sizeof(char16_t));
					//Byte-swap the data as required (no action if platform endianess matches requirement)
				Memory::byteSwap(uniString16->data(), byteCount / sizeof(char16_t), is_big_endian);
				buffer.write(reinterpret_cast<const char*>(uniString16->data()), byteCount);
			}
		}
		if (isNullAdded)
			buffer.writeBinary(char16_t());
		return buffer;
	} //writeUTF16
	
	
	/*!
	 Write this string as UTF-32 to a buffer
	 @param string The string to write
	 @param buffer The destination buffer
	 @param isNullAdded True to add a terminating null
	 @param is_big_endian True if byte ordering is big-endian
	 @param howMany The number of characters to write (nullopt for all)
	 @param maxBytes The maximum number of chars the destination can hold (including terminating null - nullopt for full length)
	 @return A reference to the destination
	 */
	const BufferOut& writeUTF32(const String& string, const BufferOut& buffer, bool isNullAdded = true,
								bool is_big_endian = text_format::defaultEndian,
								string_position howMany = std::nullopt,
								string_position maxBytes = std::nullopt) {
		if ((howMany == 0) || (maxBytes == 0))
			return buffer;
		const auto* text = string.data();
		auto uniString = string_function::to_unicode(text);
		if (uniString) {
				//If the buffer has a byte limit, use it if a maximum has not been specified or is too large
			if (auto bufferMax = buffer.maxSize(); bufferMax && (!maxBytes || (*bufferMax < *maxBytes)))
				maxBytes = *bufferMax;
				//NB: When a terminating null is required, deduct this when a max size for the destination is specified
			String::size_type byteCount = maxBytes ?
			string_function::get_valid_byte_count(string.data(), *maxBytes - (isNullAdded ? sizeof(char32_t) : 0), std::nullopt, UTF32) :
			(uniString->size() * sizeof(char32_t));
				//Byte-swap the data as required (no action if platform endianess matches requirement)
			Memory::byteSwap(uniString->data(), byteCount / sizeof(char32_t), is_big_endian);
			buffer.write(reinterpret_cast<const char*>(uniString->data()), byteCount);
		}
		if (isNullAdded)
			buffer.writeBinary(char32_t());
		return buffer;
	} //writeUTF32

}

// MARK: - Constructors

/*--------------------------------------------------------------------
	Default constructor
  --------------------------------------------------------------------*/
BufferOut::BufferOut() {
	initialise();
} //BufferOut::BufferOut


/*--------------------------------------------------------------------
	Constructor
 
	destFile: The destination data file
	bufferSize: Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
	format: The destination data format
  --------------------------------------------------------------------*/
BufferOut::BufferOut(File& destFile, Memory::sizeOption bufferSize, text_format format) {
	m_bufferSizeTip = bufferSize.value_or(defaultBufferSize);
	m_format = format;
	initialise(nullptr, &destFile);
} //BufferOut::BufferOut


/*--------------------------------------------------------------------
	Constructor
 
	memory: The destination memory block
	bufferSize: Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
	format: The destination data format
  --------------------------------------------------------------------*/
BufferOut::BufferOut(Memory& memory, Memory::sizeOption bufferSize, text_format format) {
	m_bufferSizeTip = bufferSize.value_or(defaultBufferSize);
	m_format = format;
	initialise(&memory);
} //BufferOut::BufferOut


/*--------------------------------------------------------------------
	Constructor
 
	memory: The destination memory block
	bufferSize: Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
	format: The destination data format
  --------------------------------------------------------------------*/
BufferOut::BufferOut(Memory&& memory, Memory::sizeOption bufferSize, text_format format) {
	m_bufferSizeTip = bufferSize.value_or(defaultBufferSize);
	m_cache = std::make_unique<Memory>(std::move(memory));
	m_format = format;
	initialise(m_cache.get());
} //BufferOut::BufferOut


/*--------------------------------------------------------------------
	Constructor
 
	destString: The destination string
	bufferSize: Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
	format: The destination data format
  --------------------------------------------------------------------*/
BufferOut::BufferOut(String& destString, Memory::sizeOption bufferSize, text_format format) {
	m_bufferSizeTip = bufferSize.value_or(defaultBufferSize);
	m_format = format;
	initialise(nullptr, nullptr, &destString);
} //BufferOut::BufferOut


/*--------------------------------------------------------------------
	Move constructor
 
	source: The object to move
  --------------------------------------------------------------------*/
BufferOut::BufferOut(BufferOut&& source) noexcept :
		m_buffer{std::move(source.m_buffer)},
		m_file(std::exchange(source.m_file, nullptr)),
		m_str(std::exchange(source.m_str, nullptr)),
		m_memory(std::exchange(source.m_memory, nullptr)),
		m_bufferPos(std::exchange(source.m_bufferPos, 0)) {
	m_format = source.m_format;
	m_bufferSizeTip = source.m_bufferSizeTip;
} //BufferOut::BufferOut


/*--------------------------------------------------------------------
	Destructor
  --------------------------------------------------------------------*/
BufferOut::~BufferOut() {
	flushBuffer();
} //BufferOut::~BufferOut

// MARK: - Operators

/*--------------------------------------------------------------------
	Assignment with move operator
 
	source: The object to move
 
	return: A reference to this object
  --------------------------------------------------------------------*/
BufferOut& BufferOut::operator= (BufferOut&& source) noexcept {
	if (this == &source)
		return *this;
	m_buffer = std::move(source.m_buffer);
	m_file = std::exchange(source.m_file, nullptr);
	m_str = std::exchange(source.m_str, nullptr);
	m_memory = std::exchange(source.m_memory, nullptr);
	m_bufferPos = std::exchange(source.m_bufferPos, 0);
	return *this;
} //BufferOut::operator=

// MARK: - Functions (const)

/*--------------------------------------------------------------------
	Get the maximum size of the output buffer (when a fixed-sized memory buffer is used)
 
	return: The maximum number of bytes the buffer can hold (null-opt for any dynamic buffer target)
  --------------------------------------------------------------------*/
Memory::sizeOption BufferOut::maxSize() const {
	return ((m_memory == nullptr) || m_memory->owned()) ? std::nullopt : Memory::sizeOption(m_memory->size());
} //BufferOut::maxSize


/*--------------------------------------------------------------------
	Get the current write position in the destination (not the write position in the buffer)
 
	return: The write position (e.g. the write position in a destination file, nullopt on error)
  --------------------------------------------------------------------*/
Memory::sizeOption BufferOut::getPosition() const {
	if (!good() || !confirmBuffer())
		return std::nullopt;
	if (m_file != nullptr)
		return m_file->size() + m_bufferPos;
	else if (m_memory != nullptr)
		return m_memory->size() + m_bufferPos;
	else if (m_str != nullptr)
		return m_str->data_size() + m_bufferPos;
	return std::nullopt;
} //BufferOut::getPosition


/*--------------------------------------------------------------------
	Flush the buffer to the destination
	
	return: True if no errors were encountered
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::flushBuffer() const {
	if (!good())
		return *this;
	validateSmallCache(true);
	if (m_bufferPos == 0)
		return *this;
	if (m_file != nullptr) {
		try {
			m_file->write(m_buffer, m_bufferPos);
		} catch(...) {
			setState(std::ios_base::failbit);
		}
		m_bufferPos = 0;
	} else if (m_memory != nullptr) {
		if (!m_memory->owned())
			setState(std::ios_base::eofbit);	//If the destination memory is not owned, we can't move or resize it
		else {
			if (isMyBuffer())
				m_memory->append(m_buffer, 0, m_bufferPos);	//Otherwise we can append the buffered data to the destination memory
			else {
				if (m_bufferPos < m_memory->size())
					m_memory->resize(m_bufferPos);	//If we didn't fill the destination, we need to resize to the end of the written data
				confirmBuffer();
			}
			m_bufferPos = 0;	//…and reset the buffer write position
		}
	} else if (m_str != nullptr) {
			//Ensure only whole chars are appended
		String::size_type charBytes = string_function::get_valid_byte_count(m_buffer.data(), m_bufferPos);
		if (charBytes == 0)
			setState(std::ios_base::failbit);	//The buffer content can't be written to a string
		else {
			m_str->append(String(m_buffer.data(), charBytes));
				//If we couldn't consume the full buffer with valid characters, we need to retain any remaining
			if (charBytes < m_bufferPos) {
				std::copy(m_buffer.data() + charBytes, m_buffer.data() + m_bufferPos, m_buffer.data());
				m_bufferPos -= charBytes;
			} else
				m_bufferPos = 0;
		}
	}
	return *this;
} //BufferOut::flushBuffer


/*--------------------------------------------------------------------
	Write the specified string
	
	toWrite: The string to write
	format: The data format
	isNullAdded: True to add a terminating null
	howMany: The number of characters to write (nullopt for all)
	maxBytes: The maximum number of bytes to write
	
	return: True if no errors occurred
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::write(const String& toWrite, text_format format, bool isNullAdded,
								  string_position howMany,
								  string_position maxBytes) const {
	if (m_str != nullptr)
		format.encoding = UTF8;	//Data written to a string must be UTF8
	if ((format.encoding == UTF8) || (format.encoding == ascii) || (format.encoding == ISO8859_1))
		return write(toWrite.data(), toWrite.data_size());	//NB: This isn't necessarily correct for ascii or ISO8859.1 - update when required
	switch (format.encoding) {
		case UTF8: case ascii: case ISO8859_1:
			return writeUTF8(toWrite, *this, isNullAdded, howMany, maxBytes);
		case UTF16:
			return writeUTF16(toWrite, *this, isNullAdded, format.is_big_endian, howMany, maxBytes);
		case UTF32:
			return writeUTF32(toWrite, *this, isNullAdded, format.is_big_endian, howMany, maxBytes);
	}
	return *this;
} //BufferOut::write


/*--------------------------------------------------------------------
	Write a specified memory block (NB: The data is not assumed to be text - simply a stream of bytes)
	
	toWrite: The block address
	length: The number of bytes to write
	
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::write(const char* toWrite, Memory::size_type length) const {
	if (!good())
		return *this;
		//Check if we need to write a BOM
	if ((getPosition() == 0) && m_smallCache.empty() && m_format.is_bom) {
		if (auto bom = Memory::BOM(m_format); bom)
			if (!performWrite(bom->data(), bom->size()))
				return *this;
	}
	return (length < smallDataBounds) ? writeToSmallCache(toWrite, length) : performWrite(toWrite, length);
} //BufferOut::write


/*--------------------------------------------------------------------
	Write a single byte
	
	toPut: The byte to write
	
	return: True if no errors occurred
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::write(unsigned char toWrite) const {
	if (good())
		m_smallCache.push_back(toWrite);
	return validateSmallCache();
} //BufferOut::write


/*--------------------------------------------------------------------
	Get the data written by the buffer (flushed and buffered)
 
	return: The written data
  --------------------------------------------------------------------*/
Memory BufferOut::getOutput() const {
	if (m_file != nullptr) {
		try {
				//If no data has been written, we can simply return a wrapper to the buffer content
			if (m_file->size() == 0)
				return Memory{m_buffer.data(), m_buffer.size()};
				//Otherwise we need to write the buffered data to the file and read the entire file content
			flushBuffer();
			m_file->setPosition(0, File::start);	//Read from the file start
			Memory result;
			result.resize(m_file->size());
			m_file->read(result, m_file->size());
			m_file->setPosition(0, File::end);	//Return to writing at the file end
			return result;
		} catch(...) {
			setState(std::ios_base::failbit);
		}
	} else {
			//String and memory buffers hold all the written data in memory, so return a wrapper to this data
		flushBuffer();
		if (m_memory != nullptr)
			return Memory(const_cast<char*>(m_memory->data()), m_memory->size());
		else if (m_str != nullptr)
			return Memory(const_cast<char*>(m_str->data()), m_str->data_size());
	}
	return Memory{};
} //BufferOut::getOutput

// MARK: - Functions (mutating)

/*--------------------------------------------------------------------
	Set the buffer output destination
 
	sourceFile: The destination data file
  --------------------------------------------------------------------*/
void BufferOut::setDestination(File* destFile) {
	flushBuffer();
	initialise(nullptr, destFile);
} //BufferOut::setDestination


/*--------------------------------------------------------------------
	Set the buffer output destination
 
	destHandle: The destination memory block
  --------------------------------------------------------------------*/
void BufferOut::setDestination(Memory* memory) {
	flushBuffer();
	initialise(memory);
} //BufferOut::setDestination


/*--------------------------------------------------------------------
	Set the buffer output destination
 
	sourceString: The destination string
  --------------------------------------------------------------------*/
void BufferOut::setDestination(String* destString) {
	flushBuffer();
	initialise(nullptr, nullptr, destString);
} //BufferOut::setDestination


/*--------------------------------------------------------------------
	Write to the small cache
 
	toWrite: The block address
	length: The number of bytes to write
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::writeToSmallCache(const char* toWrite, Memory::size_type length) const {
	auto readPos = toWrite;
	for ( ; length-- > 0; ++readPos)
		m_smallCache.push_back(*readPos);
	return validateSmallCache();
} //BufferOut::writeToSmallCache


/*--------------------------------------------------------------------
	Validate the small cache content (write when full)
 
	isWriteForced: True to force the cache content to written immediately
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::validateSmallCache(bool isWriteForced) const {
	if (m_smallCache.empty() || (!isWriteForced && (m_smallCache.size() < smallCacheBounds)))
		return *this;
	std::vector<char> cache(std::move(m_smallCache));
	m_smallCache = {};
	performWrite(cache.data(), cache.size());
	return *this;
} //BufferOut::validateSmallCache


/*--------------------------------------------------------------------
	Write a specified memory block
 
	toWrite: The block address
	length: The number of bytes to write
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::performWrite(const char* toWrite, Memory::size_type length) const {
	if ((length == 0) || !good() || !confirmBuffer())
		return *this;
	validateSmallCache(true);
	Memory::size_type start = 0;
	while (length > 0) {
		Memory::size_type count = std::min(length, m_buffer.size() - m_bufferPos);
		std::copy(toWrite + start, toWrite + start + count, m_buffer.data() + m_bufferPos);
		m_bufferPos += count;
		start += count;
		length -= count;
		if ((length > 0) && !flushBuffer())
			break;
	}
	return *this;
} //BufferOut::performWrite


/*--------------------------------------------------------------------
	Initialise the destination
	  
	memory: A block of memory to write the data
	fileSrc: A file to write the data to
	stringDest:A string to write the data to
  --------------------------------------------------------------------*/
void BufferOut::initialise(Memory* memory, File* fileDest, String* stringDest) {
	IOBase::clear();
	m_smallCache.reserve(smallCacheBounds);
	m_bufferPos = 0;
	m_file = fileDest;
	m_memory = memory;
		//We start by using the allocated memory as the buffer if possible - it will expand if the written data exceeds the allocation
	if ((memory != nullptr) && (memory->data() != nullptr))
		m_buffer = Memory{memory->data(), memory->size()};
	m_str = stringDest;
	if (m_file != nullptr) {
		try {
			m_file->setPosition(0, File::end);	//Start writing at the file end
		} catch(...) {
			setState(std::ios_base::failbit);
		}
	}
} //BufferOut::initialise


/*--------------------------------------------------------------------
	Confirm the buffer is prepared to receive data
 
	return: True if the buffer is confirmed (false = bad buffer state - writing will be rejected)
  --------------------------------------------------------------------*/
bool BufferOut::confirmBuffer() const {
	if (!isMyBuffer() || !m_buffer.empty())
		return true;
	return m_buffer.resize(m_bufferSizeTip);
} //BufferOut::confirmBuffer
