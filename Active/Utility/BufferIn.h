/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_UTILITY_BUFFER_IN
#define ACTIVE_UTILITY_BUFFER_IN

#include "Active/File/Interface/IOBase.h"
#include "Active/Utility/Memory.h"
#include "Active/Utility/String.h"
#include "Active/Utility/TextEncoding.h"

namespace active::file {
	
	class File;
	
}

namespace active::utility {
	
	class Memory;
	
	/*!
		Class to buffer incoming data from a specified source
	*/
	class BufferIn: public file::IOBase {
	public:
		
		using enum TextEncoding;
		
		// MARK: - Constructors
		
		/*!
			Constructor
			@param sourceFile The source data file. NB: The buffer does not take ownership of this object - must be maintained for the buffer
		 	@param format The source data format (nullopt = attempt to discover format from source)
		*/
		BufferIn(const file::File& sourceFile, DataFormat::Option format = DataFormat{});
		/*!
			Constructor
			@param sourceMem A source block of memory. NB: The buffer does not take ownership of this object - must be maintained for the buffer
		 	@param format The source data format (nullopt = attempt to discover format from source)
		*/
		BufferIn(const Memory& sourceMem, DataFormat::Option format = DataFormat{});
		/*!
			Constructor
			@param sourceString The source data string. NB: The buffer does not take ownership of this object - must be maintained for the buffer
		 	@param format The source data format (nullopt = attempt to discover format from source)
		*/
		BufferIn(const String& sourceString, DataFormat::Option format = DataFormat{});
		/*!
			Move constructor
			@param source The object to move
		*/
		BufferIn(BufferIn&& source)noexcept;
		
			///No copy constructor
		BufferIn(const BufferIn&) = delete;
		
		// MARK: - Operators
		
		/*!
			Assignment with move operator
			@param source The object to move
			@return A reference to this object
		*/
		BufferIn& operator= (BufferIn&& source) noexcept;
			//No assignment
		BufferIn& operator= (const BufferIn&) = delete;
		/*!
			Bool operator
			@return True if the source is not is a failed state
		*/
		operator bool () const { return !fail(); }
		/*!
			Read operator (the next word)
			@param str A string to read
			@return A reference to this
		*/
		const BufferIn& operator>>(String& str) const { str = readWord(); return *this; }
		/*!
			Read operator
			@param val A 16-bit integer to read
			@return A reference to this
		*/
		const BufferIn& operator>>(int16_t& val) const { return getValue(val); }
		/*!
			Read operator
			@param val A 32-bit integer to read
			@return A reference to this
		*/
		const BufferIn& operator>>(int32_t& val) const { return getValue(val); }
		/*!
			Read operator
			@param val An unsigned 32-bit integer to read
			@return A reference to this
		*/
		const BufferIn& operator>>(uint32_t& val) const { return getValue(val); }
		/*!
			Read operator
			@param val A 64-bit integer to read
			@return A reference to this
		*/
		const BufferIn& operator>>(int64_t& val) const { return getValue(val); }
		/*!
			Read operator
			@param val An unsigned 64-bit integer to read
			@return A reference to this
		*/
		const BufferIn& operator>>(uint64_t& val) const { return getValue(val); }
		/*!
			Read operator
			@param val A double to read
			@return A reference to this
		*/
		const BufferIn& operator>>(double& val) const;
		
		// MARK: - Functions (const)
		
		/*!
			Get the number of bytes to be supplied from the buffer (unread in the buffer plus remainder in the source)
			@return The number of bytes still to be supplied
		*/
		Memory::size_type getSupplyCount() const { return (m_bufferLen - m_readPos) + m_remaining; }
		/*!
			Get the total number of bytes in the source, e.g. the source file/string/memory
			@return The total number of bytes in the source
		*/
		Memory::size_type sourceSize() const;
		/*!
			Get the current row position
			@return The current row position
		*/
		Memory::size_type getLastRow() const { return m_lastRow; }
		/*!
			Get the current column position
			@return The current column position
		*/
		Memory::size_type getLastColumn() const { return m_lastColumn; }
		/*!
			Get the current read position in the source data (not the read position in the buffer)
			@return The read position (e.g. the read position in a source file)
		*/
		Memory::size_type getPosition() const;
		/*!
			Get The source text encoding (NB: for text input functionality only)
			@return The text encoding type
		*/
		TextEncoding textEncoding() const { return m_format.encoding; }
		/*!
			Apply a function to the buffered characters
			@param func The character function
			@param pool Optional string to collect the values returned from the function (nullptr = discard)
		*/
		void forEach(const String::Function& func, String* pool = nullptr) const;
		/*!
			Find the specified string within this using a filter
			@param filter The string filter
			@param pool Optional string to collect the skipped characters (nullptr = discard)
			@param isFoundSkipped True if the buffer read position should skip over the found byte
			@return True if a match is found
		*/
		bool findIf(const String::Filter& filter, String* pool = nullptr, bool isFoundSkipped = false) const;
		/*!
			Find a specified character in the buffered content (skipping over all all non-matching data)
			@param toFind The character to find (UTF-32)
			@param pool Optional string to collect the skipped characters (nullptr = discard)
			@param isFoundSkipped True if the buffer read position should skip over the found byte
			@return True if a match is found
		*/
		bool find(char32_t toFind, String* pool = nullptr, bool isFoundSkipped = false) const;
		/*!
			Find a specified string in the buffered content (skipping over all all non-matching data)
			@param toFind The string to find (UTF-32)
			@param pool Optional string to collect the skipped characters (nullptr = discard)
			@param isFoundSkipped True if the buffer read position should skip over the found string
			@return True if a match is found
		*/
		bool find(const String& toFind, String* pool = nullptr, bool isFoundSkipped = false) const
				{ return seek(toFind, pool, true, true, true, false, isFoundSkipped, false); }
		/*!
			Find the first character from a specified string in the buffered content (skipping over all all non-matching data)
			@param toFind A string containing the character(s) to find
			@param pool Optional pool string to collect the skipped characters (nullptr = discard)
			@param isContiguousMatch True if contiguous matching characters should be found, e.g. any grouping of CR and LF chars
			@param isRepeatMatch Used when isContiguousMatch = true; true to match found chars multiple times (e.g. all contiguous white-chars)
			@param isFoundSkipped True if the buffer read position should skip over the found character
			@param isFoundPooled True to add found character(s) to the pool string (ignored if isFoundSkipped = false)
			@param escapeChar An optional escape char signifying the next character is escaped
			@return True if a match is found
		*/
		bool findFirstOf(const String& toFind, String* pool = nullptr, bool isContiguousMatch = false, bool isRepeatMatch = false,
						 bool isFoundSkipped = false, bool isFoundPooled = false, std::optional<char32_t> escapeChar = std::nullopt) const
				{ return seek(toFind, pool, isContiguousMatch, false, false, isRepeatMatch, isFoundSkipped, isFoundPooled, escapeChar); }
		/*!
			Find the first character not matching a specified set (so it will be the next character read)
			@param toFind A string containing the character(s) to skip
			@param pool Optional string to collect the skipped characters (nullptr = discard)
			@param escapeChar An optional escape char signifying the next character is escaped
			@return True if a match is found
		*/
		bool findFirstNotOf(const String& toFind, String* pool = nullptr, std::optional<char32_t> escapeChar = std::nullopt) const
				{ return seekNot(toFind, pool); }
		/*!
			Get a single char
			@param dest The incoming char
			@return A reference to this
		*/
		const BufferIn& get(unsigned char& dest) const;
		/*!
			Get the next character from the buffer, encoding as necessary
		 	@param encodedChar The next encoded character (empty on failure)
			@param isConsumed True to consume the character bytes from the buffer
			@return The number of bytes consumed by the next character (0 bytes = no char read)
		*/
		Memory::size_type getEncodedChar(String& encodedChar, bool isConsumed = true) const;
		/*!
			Get the next character from the buffer as a UTF32 char
			@param isConsumed True to consume the character bytes from the buffer
			@return The character coupled with the number of bytes consumed by the next character (0 bytes = no char read)
		*/
		std::pair<char32_t, String::size_type> getEncodedChar(bool isConsumed = true) const;
		/*!
			Get a single char in a string (supporting multi-byte chars)
			@param dest The incoming char (supporting unicode)
			@return A reference to this
		*/
		const BufferIn& get(String& dest) const;
		/*!
			Read a stream of bytes
			@param dest The data destination
			@param howMany The number of bytes to read (contains the number of bytes actually read on return)
			@return A reference to this
		*/
		const BufferIn& read(char* dest, Memory::size_type& howMany) const;
		/*!
			Read a word (starting from the first non-dividing character and up to next dividing character)
			@param division The dividing character(s)
			@return The word read (empty if not found)
		*/
		String readWord(const String& division = String::allWhiteSpace) const;
		/*!
			Read multiple words from the buffer
			@param howMany Optional limit on the number of words to read (nullopt = to end of buffer)
			@param division The dividing character(s)
			@return The found words
		*/
		std::vector<String> readWords(Memory::sizeOption howMany = std::nullopt, const String& division = String::allWhiteSpace) const;
		/*!
			Get a value of a specified type from a buffer
			@param val The value to read
			@return A reference to this
		*/
		template<class T>
		const BufferIn& getValue(T& val) const {
			if (std::optional<T> temp{ readWord() }; temp)
				val = *temp;
			else {
				setState(std::ios_base::failbit);
				val = T();
			}
			return *this;
		}
		/*!
			Get a string of a specified number of characters
			@param dest The incoming string
		 	@param howMany The number of whole characters to get (nullopt to read all)
			@return A reference to this
		*/
		const BufferIn& getString(String& dest, String::sizeOption howMany = std::nullopt) const;
		/*!
			Get a single line (terminating at any known line ending)
			@param line The incoming line
		 	@param keepStop True to keep the line terminator(s)
			@return A reference to this
		*/
		const BufferIn& getLine(String& line, bool keepStop = true) const;
		/*!
			Skip a specified number of bytes forward in the input source
			@param howMany The number of bytes to skip forward
			@return A reference to this
		*/
		const BufferIn& skip(Memory::size_type howMany) const;
		/*!
			Rewind a specified number of bytes backward in the input source
			@param howMany The number of bytes to rewind back
			@return A reference to this
		*/
		const BufferIn& rewind(Memory::size_type howMany) const;
		/*!
			Set the current read position in the source data (not the read position in the buffer)
			@param pos The read position (e.g. the read position in a source file)
		*/
		void setPosition(Memory::size_type pos) const;
		/*!
			Get a pointer to the raw buffer data (NB: intended for low-level APIs that require pointer/length buffering)
			@return A pointer to the buffer data (nullptr if the buffer has no source - dataSize will return the number of bytes in the buffer)
		*/
		const char* data() const { return !good() ? nullptr : m_buffer + m_readPos; }
		/*!
			Get the number of bytes remaining in the buffer (NB: intended for low-level APIs that require pointer/length buffering
			@return The number of bytes remaining in the buffer
		*/
		Memory::size_type dataSize() const { return (!good() || (m_readPos >= m_bufferLen)) ? 0 : m_bufferLen - m_readPos; }
		/*!
			Attempt to ensure a minimum number of bytes is buffered (NB: intended for low-level APIs that require pointer/length buffering
			@param minLength The minimum buffer length required (refill of lower)
			@return The number of bytes available in the buffer
		*/
		Memory::size_type bufferMin(Memory::size_type minLength) const;

		// MARK: - Functions (mutating)
		
		/*!
			Swap this buffer with another
			@param other The buffer to swap with this
		*/
		void swap(BufferIn& other);
		/*!
			Set The source text encoding
			@param format The source data format
		*/
		void setFormat(DataFormat format) { m_format = format; }
		/*!
			Set the source encoding type
			@param encoding The source encoding type
		*/
		void setSourceEncoding(TextEncoding encoding) { m_format.encoding = encoding; }
		/*!
			Set the data source
			@param sourceFile The source data file. NB: The buffer does not take ownership of this object - must be maintained for the buffer
		 	@param format The source data format (nullopt = discover format from source)
		*/
		void setSource(const file::File& sourceFile, DataFormat::Option format = DataFormat{});
		/*!
			Set the data source
			@param sourceMem A source block of memory. NB: The buffer does not take ownership of this object - must be maintained for the buffer
		 	@param format The source data format (nullopt = discover format from source)
		*/
		void setSource(const Memory& sourceMem, DataFormat::Option format = DataFormat{});
		/*!
			Set the data source
			@param sourceString The source data string. NB: The buffer does not take ownership of this object - must be maintained for the buffer
		 	@param format The source data format (nullopt = discover format from source)
		*/
		void setSource(const String& sourceString, DataFormat::Option format = DataFormat{});
		
	private:
		/*!
			Bump the read position by the specified number of bytes (and checking eof)
			@param howMany The number of bytes to bump the read position by
		*/
		void bumpReadPos(Memory::size_type howMany) const;
		/*!
			Check if the end of file has been reached (and set the eof flag accordingly)
			@return True if eof has been reached
		*/
		bool checkEndOfFile() const;
		/*!
			Refill the buffer from the current data source
			@return True if no errors were encountered
		*/
		bool refillBuffer() const;
		/*!
			Seek specified char(s) in the buffered data
			@param toFind A string containing the character(s) to find
			@param pool Optional string to collect the skipped characters (nullptr = discard)
			@param isAllMatched True if all chars should be found
			@param isContiguousMatch True if contiguous matching characters should be found, e.g. any grouping of CR and LF chars
			@param isOrderedMatch True if the matching characters must be in the specified order (only applicable when isContiguousMatch is true)
			@param isRepeatMatch Used when isContiguousMatch=true; true to match found chars multiple times (e.g. to find all contiguous white-space)
			@param isFoundSkipped True if the buffer read position should skip over the found character
			@param isFoundPooled True if the found character is added to the pool string (ignored if isFoundSkipped = false)
			@param escapeChar An optional escape char signifying the next character is escaped
			@return True if a match was found
		*/
		bool seek(const String& toFind, String* pool, bool isAllMatched, bool isContiguousMatch, bool isOrderedMatch,
				  bool isRepeatMatch, bool isFoundSkipped, bool isFoundPooled, std::optional<char32_t> escapeChar = std::nullopt) const;
		/*!
			Seek the first character not matching a specified set
			@param toFind A string containing the character(s) to skip
			@param pool Optional string to collect the skipped characters (nullptr = discard)
			@return True if a match was found
		*/
		bool seekNot(const String& toFind, String* pool, std::optional<char32_t> escapeChar = std::nullopt) const;
		/*!
			Update row/col position
		 	@param incoming The incoming character (test for line endings)
			@param size Size of the incoming char (allowing for multi-byte chars)
		*/
		void updatePosition(unsigned char incoming, uint8_t size = 1) const;
		/*!
			Get the buffer capacity, i.e. the largest number of bytes it can hold
			@return The buffer capacity
		*/
		Memory::size_type getCapacity() const;
		/*!
			Discover the source format by analysing the content
			@return The source format
		*/
		DataFormat discoverFormat();
		/*!
			Initialise the buffer source data
			@param pointerSrc A pointer to the source data
			@param fileSrc A pointer to the source file
			@param dataLen The data length
		*/
		void initialise(char* pointerSrc, const file::File* fileSrc, Memory::size_type dataLen);
		
			///A pointer to the current position in the buffer
		char* m_buffer = nullptr;
			///The source data file (nullptr if a file is not the source)
		const file::File* m_file = nullptr;
			///A buffer allocated for file input
		std::unique_ptr<Memory> m_fileCache;
			///Text encoding of the source data (relevant only to text sources)
		DataFormat m_format;
			///The current read position in the data source
		mutable Memory::size_type m_readPos = 0;
			///The number of bytes in the buffer
		mutable Memory::size_type m_bufferLen = 0;
			///The number of bytes remaining in the source that has not been accessed by the buffer
		mutable Memory::size_type m_remaining = 0;
			///The row position (relevant only to text sources, based on the number of line-terminators encountered)
		mutable Memory::size_type m_lastRow = 0;
			///The coloum position in the current row (relevant only to text sources)
		mutable Memory::size_type m_lastColumn = 0;
			///True if a CR character has been encountered (helps avoid double-counting lin terminators in Windows)
		mutable bool m_foundCR = false;
	};
	
	void swap(BufferIn& v1, BufferIn& v2);
}

#endif	//ACTIVE_UTILITY_BUFFER_IN
