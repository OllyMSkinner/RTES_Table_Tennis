cmake_minimum_required(VERSION 3.10)
project(IMUProject)

# Use modern C++ as recommended for thread safety and STL usage [3], [4]
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# COMPULSORY: Enable all compiler errors to ensure the code is stable [1], [2]
# This catches hardware-related data-type issues immediately
add_compile_options(-Wall -Wextra -Werror)

# Identify the source files
# IMUProcessor.h, IMUCallback.h, and IMUSample.h are headers and don't need listing
add_executable(imu_app main.cpp IMUPublisher.cpp)

# Link the essential libraries [5], [2]
# gpiod: Required for the event-driven hardware trigger on GPIO 17
# pthread: Required for the background worker thread
target_link_libraries(imu_app gpiod pthread)
