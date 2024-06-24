/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_FILE_DIRECTORY
#define ACTIVE_FILE_DIRECTORY

#include "Active/File/Interface/FSNode.h"
#include "Active/Utility/String.h"

namespace active::file {
	
	/// Class to represent a directory
	class Directory : public FSNode {
	public:
		
		// MARK: Types
		
			///Unique pointer
		using Unique = std::unique_ptr<Directory>;
		
		using iterator = std::filesystem::directory_iterator;
		
		// MARK: Constructors

		/*!
			Constructor
		*/
		Directory();
		/*!
			Constructor
			@param path A full path to the directory
		 	@param isMissingCreated True if the folder should be created when missing
			@param canReplaceFile True if the directory can be created and replace an existing file with the same name
			@throw std::ios_base::failure Thrown if isMissingCreated = true and the directory cannot be created
		*/
		Directory(const Path& path, bool isMissingCreated = false, bool canReplaceFile = false);
		/*!
			Constructor
			@param parent The parent directory
			@param name The directory name
		 	@param isMissingCreated True if the folder should be created when missing
			@param canReplaceFile True if the directory can be created and replace an existing file with the same name
			@throw std::ios_base::failure Thrown if isMissingCreated = true and the directory cannot be created
		*/
		Directory(const Directory& parent, const utility::String& name, bool isMissingCreated = false, bool canReplaceFile = false);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		Directory(const Directory& source) noexcept;
		
		/*!
			Clone method
			@return A clone of this object
		*/
		Directory* clonePtr() const override { return new Directory(*this); }
		
		//MARK: - Static functions

		/*!
			Get the current directory
			@throw std::filesystem::filesystem_error Throws if an error occurs
			@return The current directory
		*/
		static Directory current();

		/*!
			Get a directory for temporary files
			@throw std::bad_alloc::filesystem_error Throws if memory allocation occurs
			@return The directory for temporary files
		*/
		static Directory temporary();
		
		// MARK: Functions (const)
		
		/*!
			Get an iterator at the first directory entry
			@return An iterator at the first entry
		*/
		iterator begin() const;
		/*!
			Get an iterator at the directory end (past the last entry)
			@return The end iterator
		*/
		iterator end() const;
	};

}

#endif	//ACTIVE_FILE_DIRECTORY
