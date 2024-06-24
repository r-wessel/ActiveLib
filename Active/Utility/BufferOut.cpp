/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/BufferOut.h"

#include "Active/File/File.h"
#include "Active/Utility/Memory.h"

#include <utility>

using namespace active::file;
using namespace active::utility;

using enum TextEncoding;

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
BufferOut::BufferOut(File& destFile, Memory::sizeOption bufferSize, DataFormat format) {
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
BufferOut::BufferOut(Memory& memory, Memory::sizeOption bufferSize, DataFormat format) {
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
BufferOut::BufferOut(Memory&& memory, Memory::sizeOption bufferSize, DataFormat format) {
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
BufferOut::BufferOut(String& destString, Memory::sizeOption bufferSize, DataFormat format) {
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
		return m_str->dataSize() + m_bufferPos;
	return std::nullopt;
} //BufferOut::getPosition


/*--------------------------------------------------------------------
	Flush the buffer to the destination
	
	return: True if no errors were encountered
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::flushBuffer() const {
	if (!good() || (m_bufferPos == 0))
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
		String::size_type charBytes = String::getValidByteCount(m_buffer.data(), m_bufferPos);
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
	
	return: True if no errors occurred
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::write(const String& toWrite, DataFormat format) const {
	if (m_str != nullptr)
		format.encoding = UTF8;	//Data written to a string must be UTF8
	if ((format.encoding == UTF8) || (format.encoding == ascii) || (format.encoding == ISO8859_1))
		return write(toWrite.data(), toWrite.dataSize());	//NB: This isn't necessarily correct for ascii or ISO8859.1 - update when required
	return toWrite.writeTo(*this, format, false);
} //BufferOut::write


/*--------------------------------------------------------------------
	Write a specified memory block (NB: The data is not assumed to be text - simply a stream of bytes)
	
	toWrite: The block address
	length: The number of bytes to write
	
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::write(const char* toWrite, Memory::size_type length) const {
	if (!good() || !confirmBuffer())
		return *this;
		//Check if we need to write a BOM
	if ((getPosition() == 0) && m_format.isBOM) {
		if (auto bom = m_format.toBOM(); bom)
			if (!performWrite(bom->data(), bom->size()))
				return *this;
	}
	performWrite(toWrite, length);
	return *this;
} //BufferOut::write


/*--------------------------------------------------------------------
	Write a single byte
	
	toPut: The byte to write
	
	return: True if no errors occurred
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::write(unsigned char toWrite) const {
	return write(reinterpret_cast<const char*>(&toWrite), 1);
} //BufferOut::write

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
	Write a specified memory block
 
	toWrite: The block address
	length: The number of bytes to write
 
	return: A reference to this
  --------------------------------------------------------------------*/
const BufferOut& BufferOut::performWrite(const char* toWrite, Memory::size_type length) const {
	if ((length == 0) || !good() || !confirmBuffer())
		return *this;
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
