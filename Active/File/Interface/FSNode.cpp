/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/File/Interface/FSNode.h"

using namespace active::file;
using namespace active::utility;

/*--------------------------------------------------------------------
	Constructor

	source: The object to copy
  --------------------------------------------------------------------*/
FSNode::FSNode() noexcept {
	m_isMine = true;
} //FSNode::FSNode


/*--------------------------------------------------------------------
	Constructor
	@param path The file node path
  --------------------------------------------------------------------*/
FSNode::FSNode(const Path& path) {
	m_isMine = true;
	m_node = std::filesystem::directory_entry(path);
}


/*--------------------------------------------------------------------
	Copy constructor for FSNode

	source: The file to be copied
  --------------------------------------------------------------------*/
FSNode::FSNode(const FSNode& source) noexcept : m_node{source.m_node} {
	m_isMine = source.m_isMine;
} //FSNode::FSNode


/*--------------------------------------------------------------------
	Assignment operator
	
	source: The object to copy
 
	return: A reference to this
  --------------------------------------------------------------------*/
FSNode& FSNode::operator= (const FSNode& source) {
	if (*this != source) {
		m_node = source.m_node;
		m_isMine = source.m_isMine;
	}
	return *this;
} //FSNode::operator=


/*--------------------------------------------------------------------
	Equality operator
	
	ref: The object to compare
	
	return: True if ref is identical to this
  --------------------------------------------------------------------*/
bool FSNode::operator== (const FSNode& ref) const noexcept {
	return m_node == ref.m_node;
} //FSNode::operator==


/*--------------------------------------------------------------------
	Set the node path
 
	path: The node path
  --------------------------------------------------------------------*/
void FSNode::setPath(const Path& path) {
	m_node.assign(path);
} //FSNode::setPath


/*--------------------------------------------------------------------
	Move/rename the node path
 
	path: The new path to assign to the node (moves/renames the node accordingly)
  --------------------------------------------------------------------*/
void FSNode::moveTo(const Path& path) {
	if (path == getPath())
		return;
	std::filesystem::rename(getPath(), path);
	setPath(path);
} //FSNode::moveTo


/*--------------------------------------------------------------------
	Rename the node (just the last part of the name, i.e. the file/directory name)
 
	name: The new name
  --------------------------------------------------------------------*/
void FSNode::rename(const utility::String& name) {
	auto path{getPath()};
	path.replaceFileName(name);
	moveTo(path);
} //FSNode::rename


/*--------------------------------------------------------------------
	Remove the node path
 
	isRecursive: True if any subnodes should also be recursively removed
 
	return: True if the node was successfully removed
  --------------------------------------------------------------------*/
void FSNode::remove(bool isRecursive) {
	if (isRecursive)
		std::filesystem::remove_all(getPath());
	else
		std::filesystem::remove(getPath());
	auto currentPath = getPath();
	setPath(currentPath);	//Renew the node to reflect the new status
} //FSNode::remove
