/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_BUFFER_OUT
#define ACTIVE_UTILITY_BUFFER_OUT

#include "Active/File/Interface/IOBase.h"
#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"
#include "Active/Utility/DataFormat.h"

namespace active::file {
	
	class File;

}

namespace active::utility {
	
		//Class to buffer data to a specified destination
	class BufferOut: public file::IOBase {
	public:
		using enum utility::TextEncoding;
		
		/*!
			Default constructor
		*/
		BufferOut();
		/*!
			Constructor
			@param destFile The destination data file
			@param bufferSize Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
		 	@param format The destination data format
		*/
		BufferOut(file::File& destFile, Memory::sizeOption bufferSize = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Constructor
			@param memory The destination memory block
			@param bufferSize Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
		 	@param format The destination data format
		*/
		BufferOut(Memory& memory, Memory::sizeOption bufferSize = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Constructor
			@param memory The destination memory block
			@param bufferSize Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
			@param format The destination data format
		*/
		BufferOut(Memory&& memory, Memory::sizeOption bufferSize = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Constructor
			@param destString The destination string
			@param bufferSize Suggested buffer size (can minimise overheads if a small number is suggested - large values will be ignored)
			@param format The destination data format
		*/
		BufferOut(String& destString, Memory::sizeOption bufferSize = std::nullopt, DataFormat format = DataFormat{});
		/*!
			Move constructor
			@param source The object to move
		*/
		BufferOut(BufferOut&& source) noexcept;
			//No copy constructor
		BufferOut(const BufferOut& source) = delete;
		/*!
			Destructor
		*/
		virtual ~BufferOut();
		
		//MARK: - Operators
		
		/*!
			Assignment with move operator
			@param source The object to move
			@return A reference to this object
		*/
		BufferOut& operator= (BufferOut&& source) noexcept;
			//No assignment
		BufferOut& operator= (const BufferOut&) = delete;
		/*!
			Bool operator
			@return True if the source is not is a failed state
		*/
		operator bool () const { return !fail(); }
		/*!
			Write operator
			@param val A character to write
			@return A reference to this
		*/
		const BufferOut& operator<<(char val) const { return write(val); }
		/*!
			Write operator
			@param str A string to write
			@return A reference to this
		*/
		const BufferOut& operator<<(const String& str) const { return write(str); }
		/*!
			Write operator
			@param val A 16-bit integer to write
			@return A reference to this
		*/
		const BufferOut& operator<<(int16_t val) const { return write(String{val}); }
		/*!
			Write operator
			@param val A 32-bit integer to write
			@return A reference to this
		*/
		const BufferOut& operator<<(int32_t val) const { return write(String{val}); }
		/*!
			Write operator
			@param val An unsigned 32-bit integer to write
			@return A reference to this
		*/
		const BufferOut& operator<<(uint32_t val) const { return write(String{val}); }
		/*!
			Write operator
			@param val A 64-bit integer to write
			@return A reference to this
		*/
		const BufferOut& operator<<(int64_t val) const { return write(String{val}); }
#ifndef WINDOWS
		/*!
			Write operator
			@param val An unsigned 64-bit integer to write
			@return A reference to this
		*/
		const BufferOut& operator<<(uint64_t val) const { return write(String{val}); }
#endif
		/*!
			Write operator
			@param val A double value to write
			@return A reference to this
		*/
		const BufferOut& operator<<(float val) const { return write(String{val}); }
		/*!
			Write operator
			@param val A double value to write
			@return A reference to this
		*/
		const BufferOut& operator<<(double val) const { return write(String{val}); }
		
		// MARK: - Functions (const)

		/*!
			Get the maximum size of the output buffer (when a fixed-sized memory buffer is used)
			@return The maximum number of bytes the buffer can hold (null-opt for any dynamic buffer target)
		*/
		Memory::sizeOption maxSize() const;
		/*!
			Get the current write position in the destination (not the write position in the buffer)
			@return The write position (e.g. the write position in a destination file, nullopt on error)
		*/
		Memory::sizeOption getPosition() const;
		/*!
			Get the destination data format
			@return The data format
		*/
		DataFormat format() const { return m_format; }
		/*!
			Get the source text encoding (NB: for text input functionality only)
			@return The text encoding type
		*/
		TextEncoding getEncoding() const { return m_format.encoding; }
		/*!
			Write the specified string (using the buffer text encoding)
			@param toWrite The string to write
			@return A reference to this
		*/
		const BufferOut& write(const String& toWrite) const { return write(toWrite, m_format); }
		/*!
			Write the specified string (using the specified text encoding)
			@param toWrite The string to write
			@param format The data format
			@return A reference to this
		*/
		const BufferOut& write(const String& toWrite, DataFormat format) const;
		/*!
			Write a specified memory block (NB: The data is not assumed to be text - simply a stream of bytes. Avoid when buffering to a string)
			@param toWrite The block address
		 	@param length The number of bytes to write
			@return A reference to this
		*/
		const BufferOut& write(const char* toWrite, Memory::size_type length) const;
		/*!
			Write a single byte
			@param toWrite The byte to write
			@return A reference to this
		*/
		const BufferOut& write(unsigned char toWrite) const;
		/*!
			Write an object to the buffer as binary data (NB: Avoid when buffering to a string)
			@param obj The object to write
			@return A reference to this
		*/
		template<class T>
		const BufferOut& writeBinary(const T& obj) const {
			return write(reinterpret_cast<const char*>(&obj), sizeof(obj));
		}
		/*!
			Flush the buffer to the destination
			@return A reference to this
		*/
		const BufferOut& flush() const { return flushBuffer(); }
		
		// MARK: - Functions (mutating)
		
		/*!
			Set the buffer output destination
			@param destFile The destination data file
		*/
		void setDestination(file::File* destFile);
		/*!
			Set the buffer output destination
			@param memory The destination memory block
		*/
		void setDestination(Memory* memory);
		/*!
			Set the buffer output destination
			@param destString The destination string
		*/
		void setDestination(String* destString);
		/*!
			Set The source text encoding
			@param encoding The text encoding type
		*/
		void setEncoding(TextEncoding encoding) { m_format.encoding = encoding; }
		
	protected:
		/*!
			Determine is the buffer is owned by this object
			@return True if this object owns the buffer
		*/
		virtual bool isMyBuffer() const { return m_buffer.owned(); }
		/*!
			Get the buffer memory
			@return The buffer memory
		*/
		const Memory& getBuffer() const { return m_buffer; }
		/*!
			Set the buffer memory
			@param buffer The buffer memory
			@param size The buffer size
		*/
		void setBuffer(void* buffer, Memory::size_type size) const { m_buffer = Memory{buffer, size}; }
		/*!
			Confirm the buffer is mine and prepared to receive data
			@return True if the buffer is confirmed (false = bad buffer state - writing will be rejected)
		*/
		virtual bool confirmBuffer() const;

	private:
		static constexpr Memory::size_type defaultBufferSize = 0xF000;

		/*!
			Write a specified memory block
			@param toWrite The block address
		 	@param length The number of bytes to write
			@return A reference to this
		*/
		const BufferOut& performWrite(const char* toWrite, Memory::size_type length) const;
		/*!
			Flush the buffer to the destination
			@return True if no errors were encountered
		*/
		const BufferOut& flushBuffer() const;
		/*!
			Initialise the destination
			@param memory A block of memory to write the data
			@param fileDest A file to write the data to
			@param stringDest A string to write the data to
		*/
		void initialise(Memory* memory = nullptr, file::File* fileDest = nullptr, utility::String* stringDest = nullptr);
		
			///A file destination for buffered data
		file::File* m_file;
			///A memory destination for buffered data
		Memory* m_memory;
			///A string destination for buffered data
		String* m_str;
			///Cached memory when passed an rvalue
		Memory::Unique m_cache;
			///The buffer for outgoing data
		mutable Memory m_buffer;
			///Text encoding of the destination data (relevant only to text writes)
		DataFormat m_format;
			///The write position in the buffer
		mutable Memory::size_type m_bufferPos;
			///Recommended buffer size
		Memory::size_type m_bufferSizeTip = defaultBufferSize;
	};
	
}

#endif	//ACTIVE_UTILITY_BUFFER_OUT
