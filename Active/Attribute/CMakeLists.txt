set(SourceFiles
	${CMAKE_CURRENT_SOURCE_DIR}/Colour.cpp
)
set(HeaderFiles
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/Colour.h
	${CMAKE_CURRENT_SOURCE_DIR}/Finish.h
	${CMAKE_CURRENT_SOURCE_DIR}/TextStyle.h
	${CMAKE_CURRENT_SOURCE_DIR}/Pen.h
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
