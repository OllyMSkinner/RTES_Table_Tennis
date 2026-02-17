cmake_minimum_required(VERSION 3.10)

# Project name
project(Nat_Positiondetector)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings (good practice)
add_compile_options(-Wall -Wextra -Wpedantic)

# Include current source directory for headers
include_directories(${CMAKE_SOURCE_DIR})

# Create executable
add_executable(Nat_Positiondetector
    Positiondetector.cpp
)

# Define macro to include test main() inside Positiondetector.cpp
target_compile_definitions(Nat_Positiondetector PRIVATE POSITION_DETECTOR_TEST)

# Link required system libraries
target_link_libraries(Nat_Positiondetector
    i2c        # For I2C if you use actual IMU later
    pthread    # For threading in test loop
)
