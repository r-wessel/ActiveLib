
# File

## Contents
1. [Purpose](#purpose)
2. [Overview](#over)
3. [Application](#application)

## Purpose <a name="purpose"></a>

This module is a simple wrapper to combine several STL components into a simple interface, including `std::filesystem::directory_entry`, `std::fstream`, `std::filesystem::path` and some `std::filesystem` functionality. Use of `String` for all names ensures Unicode compliance.

The primary objects of any file system, `File` and `Directory`, are represented as concrete classes that are both nodes in a file system (derived from `FSNode`), meaning that both can collected in a single container and treated abstractly if required.

## Overview <a name="over"></a>

The documentation in the source is a straightforward guide to most of this module's functionality and requires very little additional explanation. In summary, the primary components are:

- `FSNode`: A node in a file system tree structure, e.g. pointing to a file or directory
- `Path`: A path to a `FSNode` comprises the name of each ancestor node in sequence leading to the node divided by a directory separator, e.g. `"/Users/someone/Documents/Sample.txt"`
- `File`: A file node
- `Directory`: A directory node


## Application <a name="application"></a>

The following example illustrates some of the basic operations of the File module by creating a file and directory, writing some data and then removing both nodes:
```Cpp
	//Create a directory called "Example" in a temporary directory
testDirectory = Directory{Directory::temporary(), "Example", true};
	//Create a file called "Sample.txt" in the new "Example" directory
testFile = File{testDirectory, testBufferName, File::readWrite, true};
	//Open the new file, write some data to it and close
testFile.open();
testFile.write("Some data");
testFile.close();
	//Remove the file and directory
testFile.remove();
testDirectory.remove();
```

Most functionality directly reflects the STL, but combines separate components like `directory_entry`, `fstream` and `path` into single entities.