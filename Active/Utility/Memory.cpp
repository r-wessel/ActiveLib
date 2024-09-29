/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>

using namespace active::utility;

/*--------------------------------------------------------------------
	Fill memory with a specified character
 
	start: The memory start
	size: The memory size
	fillChar: The char to fill the memory with
  --------------------------------------------------------------------*/
void Memory::fill(void* start, size_type size, char fillChar) {
	std::fill_n(reinterpret_cast<char*>(start), size, fillChar);
} //Memory::fill


/*--------------------------------------------------------------------
	Copy memory from source to destination memory
 
	dest The destination memory
	source: The source memory
	destSize: The number of bytes to copy into the destination (can be less than the total destination allocation)
	sourceSize: The source memory size
 
	return: The number of bytes copied
  --------------------------------------------------------------------*/
Memory::size_type Memory::copy(char* dest, const char* source, Memory::size_type destSize, Memory::size_type sourceSize) {
	auto copySize = std::min(sourceSize, destSize);
	std::memcpy(dest, source, copySize);
	return copySize;
} //Memory::copy


/*--------------------------------------------------------------------
	Constructor
 
	location: A pointer to the data location
	size: The length of the data source in bytes (0 = null-terminated string)
	makeCopy: True to make a private (owned) copy of the data
	takeOwnership: True for this object to take ownership of the data (ignored if makeCopy == true)
  --------------------------------------------------------------------*/
Memory::Memory(void* location, size_type size, bool makeCopy, bool takeOwnership) {
	m_location = reinterpret_cast<char*>(location);
	m_allocSize = (size == 0) ? String::getValidByteCount(m_location) : size;
	if (makeCopy)
		reallocate(m_allocSize);
	else if (takeOwnership)
		m_store = std::unique_ptr<char[]>(m_location);
} //Memory::Memory


/*--------------------------------------------------------------------
	Copy constructor

	source: The object to copy
  --------------------------------------------------------------------*/
Memory::Memory(const Memory& source) {
	m_location = source.m_location;
	m_allocSize = source.m_allocSize;
	m_size = source.m_size;
	if (source.m_store)
		reallocate(m_allocSize);
} //Memory::Memory


/*--------------------------------------------------------------------
	Move constructor
 
	source: The object to move
  --------------------------------------------------------------------*/
Memory::Memory(Memory&& source) noexcept :
		m_location(std::exchange(source.m_location, nullptr)),
		m_allocSize(std::exchange(source.m_allocSize, 0)),
		m_size(std::exchange(source.m_size, std::nullopt)),
		m_store(std::move(source.m_store)) {
}


/*--------------------------------------------------------------------
	Assignment operator

	source: The object to be copied (a complete copy of the allocated memory will be made)
	return: A reference to this object
  --------------------------------------------------------------------*/
Memory& Memory::operator= (const Memory& source) {
	if (this == &source)
		return *this;
	m_location = source.m_location;
	m_allocSize = source.m_allocSize;
	m_size = source.m_size;
	if (source.m_store) {
		reallocate(m_allocSize);
	} else
		m_store.reset();
	return *this;
} //Memory::operator=


/*--------------------------------------------------------------------
	Assignment with move operator
 
	source: The object to move
 
	return: A reference to this object
  --------------------------------------------------------------------*/
Memory& Memory::operator= (Memory&& source) noexcept {
	if (this == &source)
		return *this;
	m_location = std::exchange(source.m_location, nullptr);
	m_allocSize = std::exchange(source.m_allocSize, 0);
	m_size = std::exchange(source.m_size, std::nullopt);
	m_store = std::move(source.m_store);
	return *this;
}


/*--------------------------------------------------------------------
	Subscript operator
 
	index: Index of the byte to access
 
	return: A reference to this object
  --------------------------------------------------------------------*/
char Memory::operator[] (size_type index) const {
	if (index >= size())
		throw std::out_of_range("");
	return m_location[index];
} //Memory::operator[]


/*--------------------------------------------------------------------
	Subscript operator
 
	index: Index of the byte to access
 
	return: A reference to this object
  --------------------------------------------------------------------*/
char& Memory::operator[] (size_type index) {
	if (index >= size())
		throw std::out_of_range("");
	return m_location[index];
} //Memory::operator[]


/*--------------------------------------------------------------------
	Resize the allocated block
 
	size: The new block size
 	fillChar: Optional character to fill additional allocated memory with (nullopt = don't fill)

	return: A reference to this
  --------------------------------------------------------------------*/
Memory& Memory::resize(Memory::size_type size, std::optional<char> fillChar) {
	reallocate(size, fillChar);
	return *this;
} //Memory::resize


/*--------------------------------------------------------------------
	Append data to the memory allocation
 
	source: The data source
	startPos: The start position of the data to append
	howMany: The number of bytes to append from the source (nullopt = to end)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Memory& Memory::append(const Memory& source, size_type startPos, std::optional<size_type> howMany) {
	if (source.empty() || (startPos > source.size()))
		return *this;
	auto toAppend = howMany ? *howMany : source.size();
	if (source.size() < startPos + toAppend)
		toAppend = source.size() - startPos;
	auto originalSize = size();
	reallocate(originalSize + toAppend);
	copy(m_location + originalSize, source.m_location + startPos, toAppend, toAppend);
	return *this;
} //Memory::append


/*--------------------------------------------------------------------
	Clear data copied to this memory allocation
 
	isReleased: True to release any memory allocation held by the object
  --------------------------------------------------------------------*/
void Memory::clear(bool isReleased) {
	m_size = 0;
	if (isReleased) {
		m_location = nullptr;
		m_allocSize = 0;
		m_store.reset();
	}
} //Memory::clear


/*--------------------------------------------------------------------
	Release ownership of the memory allocation to the caller (this will become empty)
 
	return: The allocated memory location (caller takes ownership - nullptr returned if this object doesn't own the allocation)
  --------------------------------------------------------------------*/
char* Memory::release() {
	if (!m_store)
		return nullptr;	//If this object doesn't own the memory, it can't assign ownership either
	char* result = m_store.release();
	m_location = nullptr;
	m_allocSize = 0;
	m_size.reset();
	return result;
} //Memory::release


/*--------------------------------------------------------------------
	Reallocate the memory size
 
	newSize: The new memory allocation size
	fillChar: Optional character to fill additional allocated memory with (nullopt = don't fill)
	retainExisting: True to retain any existing data (within the bounds of the newly allocated space)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Memory& Memory::reallocate(Memory::size_type newSize, std::optional<char> fillChar, bool retainExisting) {
		//If the allocation is mine and the size is correct, we're done. If reallocating and not mine, assume we need a copy
	if (m_store && (size() == newSize))
		return *this;
	if (newSize > 0) {
		if (newSize > m_allocSize) {
				//Reallocate the memory if a larger block is required
			auto* previousLocation = m_location;
			auto newStore = std::make_unique<char[]>(newSize);
			if (fillChar)
				fill(newStore.get(), newSize, *fillChar);
			m_location = newStore.get();
			if (retainExisting)
				copy(m_location, previousLocation, newSize, size());
			m_allocSize = newSize;
			m_size.reset();
			m_store = std::move(newStore);
		} else {
			if (fillChar) {
				if (auto delta = newSize - size(); delta > 0)
					fill(m_location + size(), delta, *fillChar);
			}
			m_size = newSize;
		}
	} else {
		m_location = nullptr;
		m_allocSize = 0;
		m_size.reset();
		m_store.reset();
	}
	return *this;
} //Memory::reallocate
