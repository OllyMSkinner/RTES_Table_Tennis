cmake_minimum_required(VERSION 3.10)
project(RTES_Table_Tennis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- IMU library ---
add_library(ICM20948
    Example/icm20948_i2c.cpp
    Example/icm20948_utils.cpp
)
target_include_directories(ICM20948 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Example
)

# --- Position detector library ---
add_library(Nat_Positiondetector
    Nat_Positiondetector/positiondetector.cpp
)
target_include_directories(Nat_Positiondetector PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Nat_Positiondetector
)
target_link_libraries(Nat_Positiondetector PUBLIC ICM20948)

# --- Main executable ---
add_executable(rtes_main
    Example/main.cpp
)

# main needs to find positiondetector.h
target_include_directories(rtes_main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Nat_Positiondetector
    ${CMAKE_CURRENT_SOURCE_DIR}/Example
)

# Find and link external deps used by the IMU code
find_package(yaml-cpp REQUIRED)
find_library(MRAA_LIB mraa REQUIRED)

target_link_libraries(rtes_main
    Nat_Positiondetector
    ${MRAA_LIB}
    yaml-cpp
)
