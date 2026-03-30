/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/File/Path.h"

using namespace active::file;
using namespace active;

#ifdef WINDOWS
const string Path::delimiter = "\\";
#else
const string Path::delimiter = "/";
#endif


/*--------------------------------------------------------------------
	Constructor

	source: The object to copy
  --------------------------------------------------------------------*/
Path::Path() {
} //Path::Path


/*--------------------------------------------------------------------
	Constructor
 
	path: The path (typically plaform-specific)
	isRelative: True if the path is relative to the current path
  --------------------------------------------------------------------*/
Path::Path(const string& path, bool isRelative) {
	m_path = isRelative ? (Path::current() + path).m_path : std::filesystem::path{path.data()};
}


/*--------------------------------------------------------------------
	Constructor
	@param path A file system path
  --------------------------------------------------------------------*/
Path::Path(const std::filesystem::path& path) {
	m_path = path;
} //Path::Path


/*--------------------------------------------------------------------
	Copy constructor

	source: The file to be copied
  --------------------------------------------------------------------*/
Path::Path(const Path& source) {
	m_path = source.m_path;
} //Path::Path


/*--------------------------------------------------------------------
	Get the current path
 
	return: The current path
  --------------------------------------------------------------------*/
Path Path::current() {
	return Path(std::filesystem::current_path());
}


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
  --------------------------------------------------------------------*/
Path& Path::operator= (const Path& source) {
	if (this != &source)
		m_path = source.m_path;
	return *this;
} //Path::operator=


/*--------------------------------------------------------------------
	Equality operator
	
	ref: The object to compare
	
	return: True if ref is identical to this
  --------------------------------------------------------------------*/
bool Path::operator== (const Path& ref) const {
	return m_path == ref.m_path;
} //Path::operator==


/*--------------------------------------------------------------------
	Addition operator
 
	toAppend: The string to append to this path (as a file node name)
 
	return: The new path
  --------------------------------------------------------------------*/
Path Path::operator+ (const string& toAppend) const {
	return std::filesystem::path(m_path) /= toAppend.data();
} //Path::operator+


/*--------------------------------------------------------------------
	Conversion operator
 
	return: The path as a string
  --------------------------------------------------------------------*/
Path::operator string() const {
	std::u8string pathString = m_path.u8string();
	return string(pathString);
} //Path::operator string


/*--------------------------------------------------------------------
	Get the name of the node (last name in the path)
 
	isExtensionDeleted: True if the extension should be deleted
 
	return: The name of the node
  --------------------------------------------------------------------*/
string Path::getNodeName(bool isExtensionDeleted) const {
	return string(m_path.filename().string());
} //Path::getNodeName


/*--------------------------------------------------------------------
	Get the node name extension, e.g. "xml" (no dot)
 
	return: The node name extension
  --------------------------------------------------------------------*/
string Path::getExtension() const {
	return string(m_path.filename().extension().string());
} //Path::getExtension


/*--------------------------------------------------------------------
	Get the path as a platform-specific string
 
	return: The path as a platform-specific string
  --------------------------------------------------------------------*/
string Path::platformSpecific() const {
	return string(m_path.filename().make_preferred().string());
} //Path::getPath


/*--------------------------------------------------------------------
	Determine if the path has a file name (if any name remains past the last '/')
 
	return: True if the path has a file name
  --------------------------------------------------------------------*/
bool Path::hasFileName() const {
	return m_path.has_filename();
} //Path::hasFileName


/*--------------------------------------------------------------------
	Determine if the path exists
 
	return: True if the path exists
  --------------------------------------------------------------------*/
bool Path::exists() const {
	return std::filesystem::exists(m_path);
} //Path::exists


/*--------------------------------------------------------------------
	Appends a name to the path (with a prefixed directory separator)
 
	toAppend: The string to append to this path (with a directory separator)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Path& Path::append(const string& toAppend) {
	m_path /= toAppend.string();
	return *this;
} //Path::append


/*--------------------------------------------------------------------
	Concatenates text to the path (without inserting a path separator - also use /=)
 
	toConcat: The string to concatenate to this path (as a directory node name - also use +=)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Path& Path::concat(const string& toConcat) {
	m_path += toConcat.string();
	return *this;
} //Path::concat


/*--------------------------------------------------------------------
	Remove the filename component of the path (if any name remains past the last '/')
 
	return: A reference to this
  --------------------------------------------------------------------*/
Path& Path::removeFileName() {
	m_path.remove_filename();
	return *this;
} //Path::removeFileName


/*--------------------------------------------------------------------
	Replace the file name
 
	replacement: The new name (an emply string simply removes the name)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Path& Path::replaceFileName(const string& replacement) {
	m_path.replace_filename(replacement.string());
	return *this;
} //Path::replaceFileName


/*--------------------------------------------------------------------
	Replace the file name extension
 
	replacement: The new extension (an emply string simply removes the extension)
 
	return: A reference to this
  --------------------------------------------------------------------*/
Path& Path::replaceExtension(const string& replacement) {
	m_path.replace_extension(replacement.string());
	return *this;
} //Path::replaceExtension
