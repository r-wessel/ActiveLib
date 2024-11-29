/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_MEMORY
#define ACTIVE_UTILITY_MEMORY

#include "Active/Utility/Cloner.h"

#include <bit>
#include <memory>
#include <optional>

namespace active::utility {
	
		///Class representing (and optionally allocating) memory with a specified location and size
	class Memory: public utility::Cloner {
	public:

		inline static constexpr bool defaultEndian{std::endian::native == std::endian::big};
		
		// MARK: - Types
		
			///Memory size/position type
		using size_type = std::size_t;
			///Optional memory size/position
		using sizeOption = std::optional<size_type>;
			///Unique pointer
		using Option = std::optional<Memory>;
			///Unique pointer
		using Unique = std::unique_ptr<Memory>;
			///Shared pointer
		using Shared = std::shared_ptr<Memory>;
		
		// MARK: - Static functions
		
		/*!
			Fill memory with a specified character
			@param start The memory start
			@param size The memory size
			@param fillChar The char to fill the memory with
		*/
		static void fill(void* start, size_type size, char fillChar = 0);
		/*!
			Erase the memory of a target data structure (reset content to 0)
			@param target The target data structure
		*/
		template<typename T>
		static void erase(T& target) { fill(&target, sizeof(target)); }
		/*!
			Copy memory from source to destination memory
			@param dest The destination memory
			@param source The source memory
			@param destSize The number of bytes to copy into the destination (can be less than the total destination allocation)
			@param sourceSize The source memory size
			@return The number of bytes copied
		*/
		static size_type copy(char* dest, const char* source, size_type destSize, size_type sourceSize);
		/*!
			Make an integer value big-endian (no action if the host processor is already big-endian)
			@param val The target value
			@return The integer as big-endian
		*/
		template<typename T> requires (std::is_arithmetic_v<T>)
		static T toBigEndian(T val) {
				//Return the given value unchanged if the processor is big-endian
			if (defaultEndian)
				return val;
			byteSwap(val);
			return val;
		}
		/*!
			Make a big-endian integer value suited to the host processor (no action if the host processor is already big-endian)
			@param val The target value
			@return The integer suited to the host processor
		*/
		template<typename T> requires (std::is_arithmetic_v<T>)
		static T fromBigEndian(T val) {
				//Return the given value unchanged if the processor is big-endian
			if (defaultEndian)
				return val;
			byteSwap(val);
			return val;
		}
		/*!
			Byte-swap an integer value (reverse the order of bytes to change endianess)
			@param val The target value
		*/
		template<typename T> requires (std::is_arithmetic_v<T>)
		static void byteSwap(T& val) {
			auto* data = reinterpret_cast<unsigned char*>(&val);
			auto bytes = sizeof(T);
			for (auto i = bytes-- / 2; i--; )
				std::swap(data[i], data[bytes - i]);
		}
		/*!
			Byte-swap an array of integer values (in the native byte-order)
			@param val A pointer to the array start
			@param howMany How many values are in the array
			@param toBigEndian True if the end result should be big-endian
		*/
		template<typename T> requires (std::is_arithmetic_v<T>)
		static void byteSwap(T* val, size_type howMany, bool toBigEndian) {
			if ((howMany < 1) || (toBigEndian == defaultEndian))
				return;
			for (; howMany--; ++val) {
				auto data = reinterpret_cast<unsigned char*>(val);
				auto bytes = sizeof(T);
				for (auto i = bytes-- / 2; i--; )
					std::swap(data[i], data[bytes - i]);
			}
		}

		// MARK: - Constructors
		
		/*!
			Default constructor
		*/
		Memory() {}
		/*!
			Constructor
		 	@param buffer A buffer (where the length can be established with sizeof(T)
			@param makeCopy True to make a private (owned) copy of the data
			@param takeOwnership True for this object to take ownership of the data (must be allocated with new char[] - ignored if makeCopy == true)
		*/
		template<typename T> requires (sizeof(T) > 1) && (!std::is_pointer<T>())
		Memory(const T& buffer, bool makeCopy = false, bool takeOwnership = false) : Memory{&buffer, sizeof(buffer), makeCopy, takeOwnership} {}
		/*!
			Constructor
		 	@param location A pointer to the data location
			@param size The length of the data source in bytes (0 = null-terminated string)
			@param makeCopy True to make a private (owned) copy of the data
			@param takeOwnership True for this object to take ownership of the data (must be allocated with new char[] - ignored if makeCopy == true)
		*/
		Memory(const void* location, size_type size, bool makeCopy = false, bool takeOwnership = false);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Memory(const Memory& source);
		/*!
			Move constructor
			@param source The object to move
		*/
		Memory(Memory&& source) noexcept;
		
		/*!
			Clone method
		 	@return A clone of this object
		*/
		Memory* clonePtr() const override { return new Memory{*this}; }
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to be copied (a complete copy of the allocated memory will be made)
			@return A reference to this object
		*/
		Memory& operator= (const Memory& source);
		/*!
			Assignment with move operator
			@param source The object to move
			@return A reference to this object
		*/
		Memory& operator= (Memory&& source) noexcept;
		/*!
			Conversion operator
			@return True if the memory allocation is not empty
		*/
		operator bool() const { return !empty(); }
		/*!
			Subscript operator
			@param index Index of the byte to access
			@return A reference to this object
		*/
		char operator[] (size_type index) const;
		/*!
			Subscript operator
			@param index Index of the byte to access
			@return A reference to this object
		*/
		char& operator[] (size_type index);
		
		// MARK: - Functions (const)
		
		/*!
			Get the location of the allocated memory
			@return The  allocated memory location
		*/
		char* data() const { return m_location; }
		/*!
			Get the size of the allocated memory
			@return The size of the allocated memory
		*/
		size_type size() const { return (m_location == nullptr) ? 0 : m_size.value_or(m_allocSize); }
		/*!
			Determine if the memory allocation is empty
			@return True if the memory allocation is empty (size() == 0 or location == nullptr)
		*/
		bool empty() const { return (size() == 0); }
		/*!
			Determine if the memory is owned by this object
			@return True if the memory is owned by this object (otherwise it points to a fixed-length allocation)
		*/
		bool owned() const { return m_store.operator bool() || (m_location == nullptr); }
		
		// MARK: - Functions (mutating)
		
		/*!
			Get the location of the allocated memory
			@return The allocated memory location (writable)
		*/
		char* data() { return m_location; }

		/*!
			Resize the memory allocation. NB: Makes a private copy of unowned data
		 	@param newSize The new allocation size
			@param fillChar Optional character to fill additional allocated memory with (nullopt = don't fill)
			@return A reference to this
		*/
		Memory& resize(size_type newSize, std::optional<char> fillChar = std::nullopt);
		/*!
			Append data to the memory allocation. NB: Makes a private copy of unowned data
		 	@param source The data source
			@param startPos The start position of the data to append
			@param howMany The number of bytes to append from the source (nullopt = to end)
			@return A reference to this
		*/
		Memory& append(const Memory& source, size_type startPos = 0, std::optional<size_type> howMany = std::nullopt);
		/*!
			Fill allocated memory with a specified character
			@param fillChar The character to fill the allocated memory with (0 by default)
		*/
		void fill(char fillChar = 0);
		/*!
			Clear data copied to this memory allocation
			@param isReleased True to release any memory allocation held by the object
		*/
		void clear(bool isReleased = true);
		/*!
			Release ownership of the memory allocation to the caller (this will become empty)
			@return The allocated memory location (caller takes ownership - nullptr returned if this object doesn't own the allocation)
		*/
		char* release();

	private:
		/*!
			Reallocate the memory size
			@param size The new memory allocation size
			@param fillChar Optional character to fill additional allocated memory with (nullopt = don't fill)
			@param retainExisting True to retain any existing data (within the bounds of the newly allocated space)
			@return A reference to this
		*/
		Memory& reallocate(size_type size, std::optional<char> fillChar = std::nullopt, bool retainExisting = true);
			
			///The memory location (whether owned externally or by this object)
		char* m_location = nullptr;
			///The used memory size (nullopt = same as allocated size)
		sizeOption m_size;
			///The (original) allocated memory size
		size_type m_allocSize = 0;
			///Storage for memory allocated by this object
		std::unique_ptr<char[]> m_store;
	};
	
}  // namespace active::utility

#endif	//ACTIVE_UTILITY_MEMORY
