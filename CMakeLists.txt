cmake_minimum_required(VERSION 3.10)
project(pi3_sender_project)

set(CMAKE_CXX_STANDARD 17)

# IMU driver
add_library(icm20948
    IMU_processing/imudriver/icm20948_i2c.cpp
    IMU_processing/imu_settings/icm20948_utils.cpp
)

target_include_directories(icm20948 PUBLIC
    IMU_processing/imudriver
    IMU_processing/imu_settings
)

# IMU Publisher
add_library(imu_publisher
    IMU_processing/IMUPublisher/IMUPublisher.cpp
)

target_include_directories(imu_publisher PUBLIC
    IMU_processing/IMUPublisher
)

target_link_libraries(imu_publisher PUBLIC icm20948)

# Executable
add_executable(pi3sender pi3_sender.cpp)

target_link_libraries(pi3sender
    imu_publisher
    icm20948
    pthread
    gpiod
    mraa
    yaml-cpp
)