cmake_minimum_required(VERSION 3.16)
project(icm20948_i2c_coords LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(MRAA REQUIRED mraa)
include_directories(${MRAA_INCLUDE_DIRS})

find_package(yaml-cpp REQUIRED)

add_executable(imu_coords
    main.cpp
    icm20948_i2c.cpp
    icm20948_utils.cpp
)

target_include_directories(imu_coords PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(imu_coords PRIVATE
    ${MRAA_LIBRARIES}
    yaml-cpp
)
