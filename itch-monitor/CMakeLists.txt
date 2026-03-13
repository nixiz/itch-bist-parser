cmake_minimum_required(VERSION 3.15)


# Create executable
add_executable(itch-monitor itch-monitor.cc)

# Link with required libraries
target_link_libraries(itch-monitor
    PRIVATE
        bist-algo
        itch-core
)
