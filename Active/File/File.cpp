/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/File/File.h"

#include "Active/File/Directory.h"

#include <system_error>

using namespace active::file;
using namespace active::utility;

const File::size_type File::defBufferSize = 0x4000UL;

namespace {
	
	/*!
		Get the standard mode for the specified file permissions
		@param permission The requested permissions
		@return The mode equivalent to the specified permission
	*/
	std::ios_base::openmode getModeFor(File::Permission permission) {
		switch(permission) {
			case File::readOnly:
				return std::ios_base::in;
			case File::writeOnly:
				return std::ios_base::ate;
			case File::overWrite:
				return std::ios_base::out | std::ios_base::trunc;
			case File::readWrite:
				return std::ios_base::in | std::ios_base::out;
			case File::append:
				return std::ios_base::out | std::ios_base::app;
		}
		return std::ios_base::in;
	}

	
	/*!
		Get the standard direction for the specified file anchor
		@param anchor The anchor
		@return The mode equivalent to the specified anchor
	*/
	std::ios_base::seekdir getOffsetFor(File::Anchor anchor) {
		switch(anchor) {
			case File::start:
				return std::ios_base::beg;
			case File::current:
				return std::ios_base::cur;
			case File::end:
				return std::ios_base::end;
		}
		return std::ios_base::end;
	}
	
}  // namespace

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
File::File() : FSNode() {
	m_permission = readWrite;
} //File::File


/*--------------------------------------------------------------------
	Constructor

	path: The full path to the file
	perm: Access permission
	isMissingCreated: True if the file should be created when missing
	canReplaceDirectory: True if the file can be created and replace an existing directory with the same name
  --------------------------------------------------------------------*/
File::File(const Path& path, Permission perm, bool isMissingCreated, bool canReplaceDirectory) : FSNode(path) {
	m_permission = perm;
	if (!isMissingCreated || (exists() && (!canReplaceDirectory || !isDirectory())))
		return;
		//Ensure the parent path exists
	auto parentPath{path};
	parentPath.removeFileName();
	Directory parent{parentPath, true};
		//Attempt to create the file
	std::fstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	file.open(utility::String{path}, std::ios_base::out);
	setPath(path);	//The path for a new node has to be refreshed or reports as non-existent
	file.close();
} //File::File


/*--------------------------------------------------------------------
	Constructor
	
	parent: The parent directory
	name: The file name
	perm: The available access permission
	isMissingCreated: True if the file should be created when missing
	canReplaceDirectory: True if the file can be created and replace an existing directory with the same name
  --------------------------------------------------------------------*/
File::File(const Directory& parent, const String& name, Permission perm, bool isMissingCreated, bool canReplaceDirectory) :
		File(parent.getPath() + name, perm, isMissingCreated, canReplaceDirectory) {
} //File::File


/*--------------------------------------------------------------------
	Copy constructor for File

	source: The file to be copied
  --------------------------------------------------------------------*/
File::File(const File& source) : FSNode(source) {
	m_permission = source.m_permission;
} //File::File


/*--------------------------------------------------------------------
	Destructor
  --------------------------------------------------------------------*/
File::~File() {
	close();
} //File::~File


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
  --------------------------------------------------------------------*/
void File::operator= (const File& source) {
	if (this != &source) {
		close();
		FSNode::operator=(source);
		m_file = nullptr;
		m_permission = source.m_permission;
	}
} //File::operator=


/*--------------------------------------------------------------------
	Equality operator
	
	ref: The object to compare
	
	return: True if ref is identical to this
  --------------------------------------------------------------------*/
bool File::operator== (const File& ref) const {
	return (getPath() == ref.getPath());
} //File::operator==


/*--------------------------------------------------------------------
	Open the file

	perm: Optional access permission - otherwise existing permissions are used
  --------------------------------------------------------------------*/
void File::open(std::optional<Permission> perm) const {
	auto permission = perm ? perm.value() : m_permission;
	File::sizeOption originalPosition{0};
		//Check if the file is already open with the specified permissions
	if (isOpen()) {
		if (m_permission == permission)
			return;
		if (auto position = getPosition(); position)
			originalPosition = position;
		close();
	}
	m_file = std::make_unique<std::fstream>();
	m_file->exceptions(std::ifstream::failbit | std::ifstream::badbit);
	m_file->open(utility::String{getPath()}, getModeFor(permission));
	m_permission = permission;
	if (originalPosition)
		setPosition(*originalPosition);
} //File::Open


/*--------------------------------------------------------------------
	Close the file
  --------------------------------------------------------------------*/
void File::close() const noexcept {
	if (m_file && isOpen())
		m_file->close();
	return;
} //File::close


/*--------------------------------------------------------------------
	Get the read/write position in the file
 
	return: The file position
  --------------------------------------------------------------------*/
File::pos_type File::getPosition() const {
	validate();
	pos_type result;
	return static_cast<File::size_type>(m_file->tellg());
} //File::getPosition


/*--------------------------------------------------------------------
	Set the read/write position in the file to an absolute position

	pos: The file position
  --------------------------------------------------------------------*/
void File::setPosition(pos_type pos) const {
	validate();
	m_file->seekg(pos);
} //File::setPosition


/*--------------------------------------------------------------------
	Set the read/write position in the file relative to a specified anchor (start, current, end) position

	pos: The file position
	anchor: The anchor to set the position relative to
  --------------------------------------------------------------------*/
void File::setPosition(size_type pos, Anchor anchor) const {
	validate();
	m_file->seekg(pos, getOffsetFor(anchor));
} //File::setPosition


/*--------------------------------------------------------------------
	Test if the file is open
	
	return: True if the file is open
  --------------------------------------------------------------------*/
bool File::isOpen() const noexcept {
	return m_file && m_file->is_open();
} //File::isOpen


/*--------------------------------------------------------------------
	Get the file size
	
	return: The file size
  --------------------------------------------------------------------*/
File::size_type File::size() const {
	validate();
	return std::filesystem::file_size(getPath());
} //File::getSize


/*--------------------------------------------------------------------
	Get the number of bytes remaining (from current position to eof)
 
	return: The bytes remaining (nullopt on failure)
  --------------------------------------------------------------------*/
File::size_type File::remaining() const {
	return size() - getPosition();
} //File::remaining


/*--------------------------------------------------------------------
	Read from the file into a string (from the current position)
 
	text: The string to read the data into
	howMany: The number of bytes to read (nullopt = entire file)
 
	return: The number of bytes read (bytes populated into a valid string, i.e. stops if nil or invalid coding found)
  --------------------------------------------------------------------*/
File::size_type File::read(String& text, sizeOption howMany, TextEncoding encoding) const {
	validate();
		//Ensure we don't ask for more than the remaining bytes
	auto available = remaining();
	if ((howMany == std::nullopt) || (howMany > remaining())) {
		howMany = available;
		if (howMany == 0) {
			text.clear();
			return 0;	//No remaining data is not an error
		}
	}
	Memory buffer;
	buffer.resize(static_cast<Memory::size_type>(*howMany));
	size_type bytesRead = 0;
	m_file->read(buffer.data(), *howMany);
	bytesRead = m_file->gcount();
	auto charBytes = String::getValidByteCount(buffer.data(), bytesRead, std::nullopt, encoding);
		///Move the read position if not all the bytes can be consumed by the string as valid chars
	if (charBytes < static_cast<String::size_type>(bytesRead))
		setPosition(charBytes - bytesRead, current);
	text = String{buffer.data(), charBytes, encoding};
	return charBytes;
} //File::read


/*--------------------------------------------------------------------
	Read from the file
	
	memory: The memory to read the data into
	howMany The number of bytes requested (nullopt = use allocated memory size)
	
	return: The number of bytes read (nullopt on read failure)
  --------------------------------------------------------------------*/
File::size_type File::read(Memory& memory, File::sizeOption howMany) const {
	validate();
	if (howMany == 0)
		return 0;
	if (!howMany || (*howMany < 0) || (static_cast<Memory::size_type>(*howMany) > memory.size())) {
		howMany = memory.size();
		if (howMany == 0)
			return 0;	//A request for 0 bytes is not an error
	}
	m_file->read(memory.data(), *howMany);
	return m_file->gcount();
} //File::read


/*--------------------------------------------------------------------
	Resize the file
	
	fileSize: The new file size
	
	return: True if the size was successfully applied
  --------------------------------------------------------------------*/
void File::resize(size_type fileSize) {
	validate();
	std::filesystem::resize_file(getPath(), fileSize);
} //File::resize


/*--------------------------------------------------------------------
	Write to the file from a string
 
	text: The data to write
	howMany: The number of characters to write (nullopt = full string)
	encoding: The text encoding to write
 
	return: True if the data was successfully written
--------------------------------------------------------------------*/
void File::write(const String& text, String::sizeOption howMany, TextEncoding encoding) {
	validate();
	if (text.empty())
		return;	//No data isn't an error
	String::Unique temp;
	const String* target = &text;
	if (howMany) {
		temp = std::make_unique<String>(text.substr(0, *howMany));
		target = temp.get();
	}
	switch (encoding) {
		case UTF8: case ascii: case ISO8859_1:
			m_file->write(target->data(), target->dataSize());	//NB: No checks for multi-byte chars when written as ASCII - might update in future
			break;
		case UTF16: {
			std::u16string string16 = *target;
			m_file->write(reinterpret_cast<const char*>(string16.data()), string16.size() * sizeof(char16_t));
			break;
		}
		case UTF32: {
			std::u32string string32 = *target;
			m_file->write(reinterpret_cast<const char*>(string32.data()), string32.size() * sizeof(char32_t));
			break;
		}
	}
} //File::write


/*--------------------------------------------------------------------
	Write to the file from a block of memory
 
	data: The data to write
	howMany: The data length (0 = use allocated memory size)
 
	return: True if the data was successfully written
  --------------------------------------------------------------------*/
void File::write(const Memory& data, sizeOption howMany) {
	validate();
	if (!howMany)
		howMany = data.size();
	if (howMany == 0)
		return;	//No data to write is not an error
	m_file->write(data.data(), *howMany);
} //File::write


/*--------------------------------------------------------------------
	Write the contents of another file to this
	
	file: The file to write
	
	return: True if the data was successfully written
  --------------------------------------------------------------------*/
bool File::write(const File& file) {
	validate();
	return std::filesystem::copy_file(getPath(), file.getPath());
} //File::write


/*--------------------------------------------------------------------
	Remove the node path
 
	isRecursive: True if any subnodes should also be recursively removed
 
	return: True if the node was successfully removed
  --------------------------------------------------------------------*/
void File::remove(bool isRecursive) {
	close();	//Ensure the file is closed first
	FSNode::remove(isRecursive);
} //File::remove


/*--------------------------------------------------------------------
	Flush any cached data to the file
	
	return: True if the data was successfully flushed
  --------------------------------------------------------------------*/
void File::flush() {
	validate();
	m_file->flush();
} //File::flush


/*--------------------------------------------------------------------
	Confirm a file is open - throw an exception otherwise
 
	confirmOpen: True if the file must also be open
  --------------------------------------------------------------------*/
void File::validate(bool confirmOpen) const {
	if (!m_file || (confirmOpen && !isOpen()))
		throw std::ios_base::failure("File not open", std::io_errc::stream);
} //File::validate
