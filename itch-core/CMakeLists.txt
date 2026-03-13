cmake_minimum_required(VERSION 3.15)

# Source files
set(ITCH_CORE_SOURCES
    src/event.cc
    src/helix.cc
    src/order_book.cc
    src/order_book_agent.cpp
    src/nasdaq/itch_bist_handler.cc
    src/nasdaq/itch_bist_protocol.cc
    src/parity/pmd_handler.cc
    src/parity/pmd_protocol.cc
)

# Header files
set(ITCH_CORE_HEADERS
    include/helix.hh
    include/net.hh
    include/order_book.hh
    include/order_book_agent.h
    include/compat/endian.h
    include/extern-c/helix.h
    include/nasdaq/binaryfile.hh
    include/nasdaq/itch_bist_handler.hh
    include/nasdaq/itch_bist_messages.h
    include/nasdaq/itch_bist_protocol.hh
    include/nasdaq/moldudp.hh
    include/nasdaq/moldudp64.hh
    include/nasdaq/moldudp64_messages.h
    include/nasdaq/moldudp_messages.h
    include/nasdaq/soupfile.hh
    include/parity/pmd_handler.hh
    include/parity/pmd_messages.h
    include/parity/pmd_protocol.hh
)

# Create static library
add_library(itch-core STATIC ${ITCH_CORE_SOURCES} ${ITCH_CORE_HEADERS})

# Include directories
target_include_directories(itch-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${Boost_INCLUDE_DIRS}
)

# Link Boost (header-only libraries)
target_link_libraries(itch-core PUBLIC Boost::boost fmt::fmt)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(itch-core PUBLIC ws2_32)
endif()
