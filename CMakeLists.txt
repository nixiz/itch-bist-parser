cmake_minimum_required(VERSION 3.15)
project(ITCH_Parser VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Boost
find_package(Boost 1.70 REQUIRED)

if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found. Please install Boost or set BOOST_ROOT.")
endif()

# Find fmt, preferring an installed package and falling back to FetchContent.
find_package(fmt CONFIG QUIET)

if(fmt_FOUND)
    message(STATUS "fmt version: ${fmt_VERSION}")
else()
    message(STATUS "fmt not found locally, fetching from upstream")
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.1.4
    )
    FetchContent_MakeAvailable(fmt)
endif()

# Platform-specific settings
if(WIN32)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _WIN32_WINNT=0x0A00
    )
endif()

# Add subdirectories
add_subdirectory(itch-core)
add_subdirectory(bist-algo)
add_subdirectory(bist-algo-demo)
add_subdirectory(itch-monitor)
add_subdirectory(orderbook-perf-test)
add_subdirectory(fix-parser-demo)
