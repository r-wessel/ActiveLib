/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_FILE_FILE
#define ACTIVE_FILE_FILE

#include "Active/File/Interface/FSNode.h"
#include "Active/File/Interface/IOBase.h"
#include "Active/Utility/Memory.h"

#include <fstream>
#include <optional>

namespace active::file {
	
	class Directory;

	/// Class to represent a file
	class File : public FSNode, public IOBase {
	public:

		using enum utility::TextEncoding;

		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<File>;
			///Shared pointer
		using Shared = std::shared_ptr<File>;
			///File position anchors
		enum Anchor {
			start,
			current,
			end,
		};
		
			///File size or absolute position type
		using size_type = std::fstream::off_type;
			///Optional size - used for unspecified size or position
		using sizeOption = std::optional<size_type>;
			///File position type
		using pos_type = std::fstream::pos_type;
			///Optional position
		using posOption = std::optional<pos_type>;
		
			///File permissions
		enum class Permission {
			readOnly,
			writeOnly,
			overWrite,
			readWrite,
			append
		};
		
		using enum Permission;
		
		// MARK: - Constants
		
			/// The default buffer size for file i/o
		static const size_type defBufferSize;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		File();
		/*!
			Constructor
			@param path The full path to the file
			@param perm Access permission
		 	@param isMissingCreated True if the file should be created when missing
			@param canReplaceDirectory True if the file can be created and replace an existing directory with the same name
			@throw std::ios_base::failure Thrown if isMissingCreated = true and the file cannot be created
		*/
		File(const Path& path, Permission perm = readWrite, bool isMissingCreated = false, bool canReplaceDirectory = false);
		/*!
			Constructor
			@param parent The parent directory
			@param name The file name
			@param perm Access permission
		 	@param isMissingCreated True if the file should be created when missing
			@param canReplaceDirectory True if the file can be created and replace an existing directory with the same name
			@throw std::ios_base::failure Thrown if isMissingCreated = true and the file cannot be created
		*/
		File(const Directory& parent, const utility::String& name, Permission perm = readWrite,
			 bool isMissingCreated = false, bool canReplaceDirectory = false);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		File(const File& source);
		/*!
			Destructor
		*/
		virtual ~File();
		
		/*!
			Clone method
			@return A clone of this object
		*/
		File* clonePtr() const override	{ return new File(*this); }
		
		// MARK: - Operators
		
			//Operators
		/*!
			Assignment operator
			@param source The object to copy
		*/
		void operator= (const File& source);
		/*!
			Equality operator
			@param ref The object to compare
			@return True if ref is identical to this
		*/
		bool operator== (const File& ref) const;
		
		// MARK: - Functions (const)
		
		/*!
			Open the file
			@param perm Optional access permission - otherwise existing permissions are used
			@throw std::ios_base::failure Thrown if the file cannot be opened
		*/
		void open(std::optional<Permission> perm = std::nullopt) const;
		/*!
			Close the file
		*/
		void close() const noexcept;
		/*!
			Test if the file is open
			@return True if the file is open
		*/
		bool isOpen() const noexcept;
		/*!
			Determine if the file is writable
			@return True if the file is writable
		*/
		bool isWritable() const noexcept { return m_permission != readOnly; }
		/*!
			Get the read/write position in the file
			@throw std::ios_base::failure Thrown on error
			@return The file position (nullopt if file not open)
		*/
		pos_type getPosition() const;
		/*!
			Set the read/write position in the file to an absolute position
			@param pos The file position
			@throw std::ios_base::failure Thrown on error
		*/
		void setPosition(pos_type pos) const;
		/*!
			Set the read/write position in the file relative to a specified anchor (start, current, end) position
			@param pos The relative file position
			@param anchor The anchor to set the position relative to
			@throw std::ios_base::failure Thrown on error
		*/
		void setPosition(size_type pos, Anchor anchor = Anchor::start) const;
		/*!
			Get the file size
			@throw std::ios_base::failure Thrown on error
			@return The file size
		*/
		size_type size() const;
		/*!
			Get the number of bytes remaining (from current position to eof)
			@throw std::ios_base::failure Thrown on error
			@return The bytes remaining
		*/
		size_type remaining() const;
		/*!
			Read from the file into a string (from the current position)
			@param text The string to read the data into
			@param howMany The number of bytes to read (nullopt = entire file)
			@throw std::ios_base::failure Thrown on error
			@return The number of bytes read
		*/
		size_type read(utility::String& text, sizeOption howMany = std::nullopt, utility::TextEncoding encoding = UTF8) const;
		/*!
			Read from the file
			@param memory The memory to read the data into
			@param howMany The number of bytes requested (nullopt = use allocated memory size)
			@throw std::ios_base::failure Thrown on error
			@return The number of bytes read
		*/
		size_type read(utility::Memory& memory, sizeOption howMany = std::nullopt) const;
		/*!
			Determine if the last operation was successful, i.e. eof, fail and bad bits are all 0)
			@return True if the last operation was successful
		*/
		bool good() const noexcept override { return m_file ? m_file->good() : false; }
		/*!
			Determine if end of input has been seen
			@return True if end of input has been seen
		*/
		bool eof() const noexcept override { return m_file ? m_file->eof() : true; }
		/*!
			Determine if the next operation will fail
			@return True if the next operation will fail
		*/
		bool fail() const noexcept override { return m_file ? m_file->fail() : true; }
		/*!
			Determine if the stream is bad
			@return True if the stream is bad
		*/
		bool bad() const noexcept override { return m_file ? m_file->bad() : true; }
		
		// MARK: - Functions (mutating)
		/*!
			Resize the file
			@param fileSize The new file size
			@throw std::ios_base::failure Thrown on error
		*/
		void resize(size_type fileSize);
		/*!
			Write to the file from a string
			@param text The data to write
			@param howMany The number of characters to write (nullopt = full string)
			@param encoding The text encoding to write
			@throw std::ios_base::failure Thrown on error
		*/
		void write(const utility::String& text, utility::String::sizeOption howMany = std::nullopt, utility::TextEncoding encoding = UTF8);
		/*!
			Write to the file from a block of memory
			@param data The data to write
			@param howMany The data length (nullopt = write allocated memory size)
			@throw std::ios_base::failure Thrown on error
		*/
		void write(const utility::Memory& data, sizeOption howMany = std::nullopt);
		/*!
			Write the contents of another file to this
			@param file The file to write
			@return True if the data was successfully written
		*/
		bool write(const File& file);
		/*!
			Remove the node path
			@param isRecursive True if any subnodes should also be recursively removed
			@throw std::ios_base::failure Thrown on error
		*/
		void remove(bool isRecursive = true) override;
		/*!
			Flush any cached data to the file
			@throw std::ios_base::failure Thrown on error
		*/
		void flush();
		/*!
			Set the stream state flags
			@param f The stream state flags
			@throw std::ios_base::failure Thrown on error
		*/
		void clear(flag_type f = std::ios_base::goodbit) override { if (m_file) m_file->clear(f); }
		/*!
			Modify the stream state flags
			@param f The flag to modify
		*/
		void setState(flag_type f) noexcept override { if (m_file) m_file->setstate(f); }

	private:
		/*!
			Confirm a file is open - throw an exception otherwise
			@param confirmOpen True if the file must also be open
			@throw std::ios_base::failure Thrown on error
		*/
		void validate(bool confirmOpen = true) const;
		
			///The file
		mutable std::unique_ptr<std::fstream> m_file;
			///The file permissions
		mutable Permission m_permission;
	};

}

#endif	//ACTIVE_FILE_FILE
