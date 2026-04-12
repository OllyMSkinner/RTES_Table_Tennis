cmake_minimum_required(VERSION 3.16)
project(thresholds LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(thresholds
    main.cpp
    imureader.cpp
    Icm20948driver.cpp
    positiondetector.cpp
)

target_include_directories(thresholds PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(thresholds PRIVATE
    Threads::Threads
    gpiod
    mraa
    yaml-cpp
)