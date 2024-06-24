/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#ifndef ACTIVE_FILE_FS_NODE
#define ACTIVE_FILE_FS_NODE

#include "Active/File/Path.h"
#include "Active/Utility/Cloner.h"
#include "Active/Utility/String.h"
#include "Active/Utility/Time.h"

#include <filesystem>

namespace active::file {

		/// Class to represent a file system node (directory, file etc)
	class FSNode: public utility::Cloner {
	public:
		
		// MARK: - Types
		
			///Unique pointer
		using Unique = std::unique_ptr<FSNode>;
			///Unique pointer
		using Shared = std::shared_ptr<FSNode>;
		
		// MARK: - Constants
		
		/// The path delimiter expression for the current platform
		static const utility::String pathDelimiter;
		
		// MARK: - Constructors
		
		/*!
			Constructor
		*/
		FSNode() noexcept;
		/*!
			Constructor
			@param path The file node path
			@throw std::bad_alloc Thrown if memory allocation fails
		*/
		FSNode(const Path& path);
		/*!
			Copy constructor
			@param source The object to copy
		*/
		FSNode(const FSNode& source) noexcept;
		/*!
			Destructor
		*/
		virtual ~FSNode() noexcept = default;
		
		// MARK: - Operators
		
		/*!
			Assignment operator
			@param source The object to copy
			@return A reference to this
		*/
		FSNode& operator= (const FSNode& source);
		/*!
			Equality operator
			@param ref The object to compare
			@return True if ref is identical to this
		*/
		bool operator== (const FSNode& ref) const noexcept;
		
		// MARK: - Functions (const)
		
		/*!
			Get the node path
			@return The node path
		*/
		Path getPath() const noexcept { return m_node.path(); }
		/*!
			Test if the node actually exists
			@throw std::bad_alloc Thrown if memory allocation fails
			@return True if the node exists
		*/
		virtual bool exists() const { return m_node.exists(); }
		/*!
			Test if the node data points to a directory/folder
			@throw std::bad_alloc Thrown if memory allocation fails
			@return True if this is a folder
		*/
		virtual bool isDirectory() const { return m_node.is_directory(); }
		/*!
			Get the node modification time
			@throw std::bad_alloc Thrown if memory allocation fails
			@return The node modification time
		*/
		utility::Time getModificationTime() const { return m_node.last_write_time(); }
		
		// MARK: - Functions (mutating)
		
		/*!
			Assign a node path (points to a new node without changing the previous node)
			@param path The new node path
			@throw std::bad_alloc Thrown if memory allocation fails
		*/
		virtual void setPath(const Path& path);
		/*!
			Move/rename the node path
			@param path The new path to assign to the node (moves/renames the node accordingly)
			@throw std::bad_alloc Thrown if memory allocation fails
		*/
		virtual void moveTo(const Path& path);
		/*!
			Rename the node (just the last part of the name, i.e. the file/directory name)
			@param name The new name
			@throw std::bad_alloc Thrown if memory allocation fails
		*/
		virtual void rename(const utility::String& name);
		/*!
			Remove the node path
			@param isRecursive True if any subnodes should also be recursively removed
			@throw std::bad_alloc Thrown if memory allocation fails
		*/
		virtual void remove(bool isRecursive = true);
		
	protected:
		/*!
			Determine if the node was created by this
			@return True if the node was created by this
		*/
		bool isMine() const noexcept	{ return m_isMine; }
		/*!
			Set whether the node was created by this
			@param state True if the node was created by this
		*/
		void setMine(bool state) noexcept	{ m_isMine = state; }
		
	private:
		
		// MARK: - Variables
		
			///The file system node
		std::filesystem::directory_entry m_node;
			///True if the file node was created by this. Mutable because even read-only (const) access must report status
		bool m_isMine = true;
	};
	
}

#endif	//ACTIVE_FILE_FS_NODE
