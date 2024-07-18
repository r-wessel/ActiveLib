set(SourceFiles
)
set(HeaderFiles
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/Container.md
	${CMAKE_CURRENT_SOURCE_DIR}/HashMap.h
	${CMAKE_CURRENT_SOURCE_DIR}/List.h
	${CMAKE_CURRENT_SOURCE_DIR}/Map.h
	${CMAKE_CURRENT_SOURCE_DIR}/MultiMap.h
	${CMAKE_CURRENT_SOURCE_DIR}/vector.h
)
set(ModuleFiles
	${SourceFiles}
	${HeaderFiles}
	PARENT_SCOPE
)
target_sources(ActiveLib
	PRIVATE
		${SourceFiles}
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS ${PROJECT_SOURCE_DIR}
	FILES
		${HeaderFiles}
)
