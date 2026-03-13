cmake_minimum_required(VERSION 3.15)

# Source files
set(BIST_ALGO_SOURCES
    bist_algo_base.cpp
    symbol_tracker_algo.cpp
)

# Header files
set(BIST_ALGO_HEADERS
    bist_algo_base.h
    symbol_tracker_algo.h
)

# Create static library
add_library(bist-algo STATIC ${BIST_ALGO_SOURCES} ${BIST_ALGO_HEADERS})

# Include directories
target_include_directories(bist-algo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${Boost_INCLUDE_DIRS}
)

# Link with itch-core
target_link_libraries(bist-algo PUBLIC itch-core)
