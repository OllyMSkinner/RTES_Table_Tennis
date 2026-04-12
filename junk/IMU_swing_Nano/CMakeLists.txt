cmake_minimum_required(VERSION 3.10)
project(IMU_swing_Nano LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD   REQUIRED libgpiod)
pkg_check_modules(MRAA    REQUIRED mraa)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)

set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../PI5_thatNanoCopied)

add_executable(imu_swing
    main.cpp
    IMU_swingLogic/SwingCalibrator.cpp
    IMU_swingLogic/SwingProcessor.cpp
    IMU_swingLogic/SwingDetector.cpp
    ${SHARED_DIR}/Icm20948driver.cpp
    ${SHARED_DIR}/imureader.cpp
    ${SHARED_DIR}/positiondetector.cpp
)

target_include_directories(imu_swing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SHARED_DIR}
    ${GPIOD_INCLUDE_DIRS}
    ${MRAA_INCLUDE_DIRS}
    ${YAMLCPP_INCLUDE_DIRS}
)

target_link_directories(imu_swing PRIVATE
    ${GPIOD_LIBRARY_DIRS}
    ${MRAA_LIBRARY_DIRS}
    ${YAMLCPP_LIBRARY_DIRS}
)

target_link_libraries(imu_swing PRIVATE
    ${GPIOD_LIBRARIES}
    ${MRAA_LIBRARIES}
    ${YAMLCPP_LIBRARIES}
    pthread
)
