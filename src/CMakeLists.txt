cmake_minimum_required(VERSION 3.10)
project(PI5Receiver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiodcxx)

add_executable(rtes_main
    main.cpp
    libs/IMU_math/positiondetector.cpp
    libs/LEDs/ledcallback.cpp
    libs/LEDs/led_controller.cpp
    libs/IMU_math/SwingProcessor.cpp
    libs/IMU_math/SwingCalibrator.cpp
    libs/IMU_math/SwingDetector.cpp
    libs/Motor/swingfeedback.cpp
    libs/IMU/imureader.cpp
    libs/IMU/Icm20948driver.cpp
    libs/Piezo/ads1115rpi.cpp
    libs/Piezo/event_detector.cpp
)

target_include_directories(rtes_main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/IMU/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/IMU_math/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/LEDs/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/Motor/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/Piezo/Include
    ${GPIOD_INCLUDE_DIRS}
)

target_link_directories(rtes_main PRIVATE
    ${GPIOD_LIBRARY_DIRS}
)

target_link_libraries(rtes_main PRIVATE
    ${GPIOD_LIBRARIES}
    gpiod
    yaml-cpp
    pthread
)

option(BUILD_TESTS "Build GTest targets" OFF)

if(BUILD_TESTS)
    enable_testing()

    macro(add_gtest TARGET)
        target_include_directories(${TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/IMU/Include
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/IMU_math/Include
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/LEDs/Include
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/Motor/Include
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/Piezo/Include
            ${GPIOD_INCLUDE_DIRS}
        )
        target_link_directories(${TARGET} PRIVATE
            ${GPIOD_LIBRARY_DIRS}
        )
        target_link_libraries(${TARGET} PRIVATE
            ${GPIOD_LIBRARIES}
            gpiod
            gtest
            gtest_main
            pthread
        )
        add_test(NAME ${TARGET} COMMAND ${TARGET})
    endmacro()

    add_executable(SwingDetector_GTest
        tests/SwingDetector_GTest.cpp
        libs/IMU_math/SwingDetector.cpp
    )
    add_gtest(SwingDetector_GTest)

    add_executable(SwingCalibrator_GTest
        tests/SwingCalibrator_GTest.cpp
        libs/IMU_math/SwingCalibrator.cpp
    )
    add_gtest(SwingCalibrator_GTest)

    add_executable(PiezoEventDetector_GTest
        tests/PiezoEventDetector_GTest.cpp
        libs/Piezo/event_detector.cpp
        libs/LEDs/ledcallback.cpp
        libs/LEDs/led_controller.cpp
    )
    add_gtest(PiezoEventDetector_GTest)

    add_executable(SwingFeedback_GTest
        tests/SwingFeedback_GTest.cpp
        libs/Motor/swingfeedback.cpp
    )
    add_gtest(SwingFeedback_GTest)

    add_executable(PositionDetector_GTest
        tests/PositionDetector_GTest.cpp
        libs/IMU_math/positiondetector.cpp
    )
    add_gtest(PositionDetector_GTest)

    add_executable(SwingProcessor_GTest
        tests/SwingProcessor_GTest.cpp
        libs/IMU_math/SwingProcessor.cpp
        libs/IMU_math/SwingCalibrator.cpp
        libs/IMU_math/positiondetector.cpp
    )
    add_gtest(SwingProcessor_GTest)
endif()