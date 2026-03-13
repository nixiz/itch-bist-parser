cmake_minimum_required(VERSION 3.15)

# Create executable
add_executable(bist-algo-demo bist-algo-demo.cpp)

# Link with required libraries
target_link_libraries(bist-algo-demo
    PRIVATE
        bist-algo
        fmt::fmt
        itch-core
)
