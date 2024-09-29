cmake_minimum_required(VERSION 3.20)

IF (WIN32)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
ENDIF()

# ---- Dependencies ----

include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.11
  FIND_PACKAGE_ARGS NAMES doctest
)
FetchContent_MakeAvailable(doctest)

# ---- Create binary ----
add_executable(activelibtests
    main.cpp
    BufferInTests.cpp
    ContainerTests.cpp
    EventTests/Button.cpp
    EventTests/Calculation.cpp
    EventTests/CalcApp.cpp
    EventTests/CalculatorRPN.cpp
    EventTests/EventTests.cpp
    FileTests.cpp
    GeometryTests/PolygonTests.cpp
    GuidTests.cpp
    Serialisation/Base64Tests.cpp
    Serialisation/DocumentTests.cpp
    Serialisation/JSONTests.cpp
    Serialisation/SerialiseArrayTester.cpp
    Serialisation/SerialiseTester.cpp
    Serialisation/XMLTests.cpp
    Settingtests/MeasurementTests.cpp
    SHA256Tests.cpp
    StringTests.cpp
    TimeTests.cpp
)

IF (WIN32)
	target_compile_definitions(activelibtests PRIVATE
        WINDOWS
        _LIB
    )
ENDIF()

target_link_libraries(activelibtests ActiveLib doctest::doctest)

enable_testing()

if(DEFINED doctest_SOURCE_DIR)
  list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)
endif()

include(doctest)
doctest_discover_tests(activelibtests)
