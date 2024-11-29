cmake_minimum_required(VERSION 3.20)

IF (WIN32)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
ENDIF()

add_library(
    ActiveLib
)

	#Add the sources for a module
function(addModule moduleName)
	add_subdirectory(${moduleName})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${moduleName} PREFIX ${moduleName} FILES ${ModuleFiles})
endfunction()

	#Module sources
addModule(Attribute)
addModule(Container)
addModule(Database)
addModule(Environment)
addModule(Event)
addModule(File)
addModule(Geometry)
addModule(Serialise)
addModule(Setting)
addModule(Utility)
	#Add sources at the root
set(SourceFiles
)
set(HeaderFiles
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/Version.h
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
set(ModuleFiles
	${SourceFiles}
	${HeaderFiles}
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ModuleFiles})

add_library(${namespace}ActiveLib ALIAS ActiveLib)

IF (WIN32)
	target_compile_definitions(ActiveLib PRIVATE
        WINDOWS
        _LIB
    )
ENDIF()

target_compile_features(ActiveLib PUBLIC
	cxx_std_20
)

set_target_properties(ActiveLib PROPERTIES
    OUTPUT_NAME "${ActiveLib_lib_name}"
)

target_include_directories(
    ActiveLib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# ---- library installation ----
install(
    TARGETS ActiveLib
    EXPORT "${targets_export_name}"
    FILE_SET HEADERS
    INCLUDES DESTINATION "${include_install_dir}"
)
