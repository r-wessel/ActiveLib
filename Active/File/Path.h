/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_FILE_PATH
#define ACTIVE_FILE_PATH

#include "Active/Utility/Cloner.h"
#include "Active/Utility/String.h"

#include <filesystem>

namespace active::file {

		/// Class to represent the path of a node in the local file system
	class Path: public utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Path>;
			///Unique pointer
		using Shared = std::shared_ptr<Path>;
			///Optional
		using Option = std::optional<Path>;
		
		// MARK: - Constants
		
		/// The path delimiter expression for the current platform
		static const utility::String delimiter;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		Path();
		/*!
			Constructor
			@param path The path (typically plaform-specific)
			@param isRelative True if the path is relative to the current path
		*/
		Path(const utility::String& path, bool isRelative = false);
		/*!
			Constructor
			@param path A file system path
		*/
		Path(const std::filesystem::path& path);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Path(const Path& source);
		
		/*!
			Destructor
		*/
		~Path() {}
		
		/*!
			Object cloning
			@return A clone of this object
		*/
		Path* clonePtr() const { return new Path(*this); }
		
		//MARK: - Static functions

		/*!
			Get the current path
			@return The current path
		*/
		static Path current();
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		Path& operator= (const Path& source);
		/*!
			Equality operator
			@param ref The object to compare
			@return True if ref is identical to this
		*/
		bool operator== (const Path& ref) const;
		/*!
			Inequality operator
			@param ref The object to compare
			@return True if ref is not identical to this
		*/
		bool operator!= (const Path& ref) const { return !(*this == ref); }
		/*!
			Create a path with a name appended to this path with a directory separator
			@param toAppend The string to append to this path (with a directory separator)
			@return The new path
		*/
		Path operator/ (const utility::String& toAppend) const { return Path{*this}.append(toAppend); }
		/*!
			Addition operator
			@param toAppend The string to append to this path (as a file node name)
			@return The new path
		*/
		Path operator+ (const utility::String& toAppend) const;
		/*!
			Append a name to the path (with a prefixed directory separator)
			@param toAppend The string to append to this path (with a directory separator)
			@return A reference to this
		*/
		Path& operator/= (const utility::String& toAppend) { return append(toAppend); }
		/*!
			Concatenate a name to the path (with a prefixed directory separator)
			@param toConcat The string to concatenate to this path (with a directory separator)
			@return A reference to this
		*/
		Path& operator+= (const utility::String& toConcat) { return concat(toConcat); }
		/*!
			Conversion operator
			@return The path as a filesystem::path
		*/
		operator const std::filesystem::path&() const { return m_path; }
		/*!
			Conversion operator
			@return The path as a string
		*/
		operator utility::String() const;
		
		// MARK: - Functions (const)
		
		/*!
			Determine if the path is empty
			@return True if the path is empty
		*/
		bool empty() const { return m_path.empty(); }
		/*!
			Get the name of the node (last name in the path)
		 	@param isExtensionDeleted True if the extension should be deleted
			@return The name of the node
		*/
		utility::String getNodeName(bool isExtensionDeleted = false) const;
		/*!
			Get the node name extension, e.g. "xml" (no dot)
			@return The node name extension
		*/
		utility::String getExtension() const;
		/*!
			Get the path as a platform-specific string
			@return The path as a platform-specific string
		*/
		utility::String platformSpecific() const;
		/*!
			Get the path root directory
			@return The path root directory path
		*/
		Path rootDirectory() const { return m_path.root_directory(); }
		/*!
			Get the path root name
			@return The path root name
		*/
		Path rootName() const { return m_path.root_name(); }
		/*!
			Get the path root directory
			@return The path root directory
		*/
		Path rootPath() const { return m_path.root_path(); }
		/*!
			Get a relative path (from the root directory)
			@return The relative path
		*/
		Path relativePath() const { return m_path.relative_path(); }
		/*!
			Get a lexically normal path from this path (e.g. resolve ../ into the previous directory)
			@return The lexically normal path
		*/
		Path lexicallyNormal() const { return m_path.lexically_normal(); }
		/*!
			Determine if the path has a file name (if any name remains past the last '/')
			@return True if the path has a file name
		*/
		bool hasFileName() const;
		
		// MARK: - Functions (mutating)
		
		/*!
			Appends a name to the path (with a prefixed directory separator)
			@param toAppend The string to append to this path (with a directory separator)
			@return A reference to this
		*/
		Path& append(const utility::String& toAppend);
		/*!
			Concatenates text to the path (without inserting a path separator - also use /=)
			@param toConcat The string to concatenate to this path (as a directory node name - also use +=)
			@return A reference to this
		*/
		Path& concat(const utility::String& toConcat);
		/*!
			Remove the filename component of the path (if any name remains past the last '/')
			@return A reference to this
		*/
		Path& removeFileName();
		/*!
			Replace the file name
			@param replacement The new name (an emply string simply removes the name)
			@return A reference to this
		*/
		Path& replaceFileName(const utility::String& replacement = utility::String{});
		/*!
			Replace the file name extension
			@param replacement The new extension (an emply string simply removes the extension)
			@return A reference to this
		*/
		Path& replaceExtension(const utility::String& replacement = utility::String{});
		
	private:
		
		// MARK: - Variables
		
			///The file system path
		std::filesystem::path m_path;
	};
	
}

#endif	//ACTIVE_FILE_PATH
