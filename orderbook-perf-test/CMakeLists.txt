cmake_minimum_required(VERSION 3.15)

# Create executable
add_executable(orderbook-perf-test orderbook-perf-test.cpp)

# Link with required libraries
target_link_libraries(orderbook-perf-test
    PRIVATE
        itch-core
        fmt::fmt
)
