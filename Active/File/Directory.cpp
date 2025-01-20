/*!
Copyright 2024 Ralph Wessel and Hugh Wessel
Distributed under the MIT License (See accompanying file LICENSE.txt or copy at https://opensource.org/license/mit/)
*/

#include "Active/File/Directory.h"

#ifdef WINDOWS
#include "shlobj_core.h"
#endif
#ifdef __APPLE__
#include <glob.h>
#include <sysdir.h>
#include <wordexp.h>
#endif

using namespace active::file;
using namespace active::utility;

namespace {
	
#ifdef __APPLE__
	/*!
	 Expand a specified path, e.g. replacing "~" with user home directory path
	 @param path The target path
	 @return The expanded path (nullopt on failure)
	 */
	String::Option expandPath(const String& path) {
		String::Option result;
		String shellPath{path};
			//Shell paths can't contain unescaped spaces
		shellPath.replaceAll("\\ ", " ");	//Can't be certain that some spaces aren't already escaped - reduce all to spaces
		shellPath.replaceAll(" ", "\\ ");
		wordexp_t exp_result;
		if (wordexp(shellPath.data(), &exp_result, 0) == 0)
			result = String{exp_result.we_wordv[0]};
		wordfree(&exp_result);
		return result;
	} //expandPath
#endif
							  
}

/*--------------------------------------------------------------------
	Constructor
  --------------------------------------------------------------------*/
Directory::Directory() {
} //Directory::Directory


/*--------------------------------------------------------------------
	Constructor

	path: The full path to the file
 	isMissingCreated: True if the folder should be created when missing
	canReplaceFile: True if the directory can be created and replace an existing file with the same name
  --------------------------------------------------------------------*/
Directory::Directory(const Path& path, bool isMissingCreated, bool canReplaceFile) : FSNode(path) {
	if (!isMissingCreated || (exists() && (!canReplaceFile || isDirectory())))
		return;
	std::filesystem::create_directory(path);
	setPath(path);	//The path for a new node has to be refreshed or reports as non-existent
} //Directory::Directory


/*--------------------------------------------------------------------
	Constructor
	
	parent: The parent directory
	name: The directory name
	isMissingCreated: True if the folder should be created when missing
	canReplaceFile: True if the directory can be created and replace an existing file with the same name
  --------------------------------------------------------------------*/
Directory::Directory(const Directory& parent, const String& name, bool isMissingCreated, bool canReplaceFile) :
		Directory(parent.getPath() + name, isMissingCreated, canReplaceFile) {
} //Directory::Directory


/*--------------------------------------------------------------------
	Copy constructor for Directory

	source: The file to be copied
  --------------------------------------------------------------------*/
Directory::Directory(const Directory& source) noexcept : FSNode(source) {
} //Directory::Directory


/*--------------------------------------------------------------------
	Get the current directory
 
	return: The current directory
  --------------------------------------------------------------------*/
Directory Directory::current() {
	return Directory(std::filesystem::current_path());
}


/*--------------------------------------------------------------------
	Get a directory for temporary files
 
	return: A directory for temporary files
  --------------------------------------------------------------------*/
Directory Directory::temporary() {
	return Directory(std::filesystem::temp_directory_path());
}


/*--------------------------------------------------------------------
	Get a directory for application data
 
	return: The directory for application data
  --------------------------------------------------------------------*/
Directory::Option Directory::appData() {
	Directory::Option result;
#ifdef WINDOWS
	wchar_t* directorypath = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &directorypath) == S_OK)
		result = Directory{String{reinterpret_cast<char16_t*>(directorypath)}};
	CoTaskMemFree(static_cast<void*>(directorypath));
#endif
#ifdef __APPLE__
	char directorypath[4 * PATH_MAX];
	auto state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_APPLICATION_SUPPORT,
													  SYSDIR_DOMAIN_MASK_USER);
	if ((state = sysdir_get_next_search_path_enumeration(state, directorypath))) {
			//Expand the returned path, e.g. replacing "~" with user home directory path
		if (auto expandedPath = expandPath(directorypath); expandedPath)
			result = Directory{*expandedPath};
	}
#endif
	return result;
} //Directory::appData


/*--------------------------------------------------------------------
	Get a directory for user-based configuration data (for *nix contexts only - prefer appData in most cases) *
 
	return: The directory for user-based configuration data
  --------------------------------------------------------------------*/
Directory::Option Directory::config() {
	Directory::Option result;
#ifdef WINDOWS
	result = Directory::appData();	//Always use app data in Windows
#endif
#ifdef __APPLE__
	if (auto expandedPath = expandPath("~/.config"); expandedPath)
		result = Directory{*expandedPath};
#endif
	return result;
} //Directory::config


/*--------------------------------------------------------------------
	Get the iterator for the directory content
 
	return: The current directory
  --------------------------------------------------------------------*/
Directory::iterator Directory::begin() const {
	return iterator{getPath()};
} //Directory::begin


/*--------------------------------------------------------------------
	Get an iterator at the directory end (past the last entry)
 
	return: The end iterator
  --------------------------------------------------------------------*/
Directory::iterator Directory::end() const {
	return iterator{};
}
