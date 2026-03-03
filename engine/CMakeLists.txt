cmake_minimum_required(VERSION 3.20)
project(midas VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# External dependencies
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include directories
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/eigen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/json/include)

# Source files
set(MIDAS_CORE_SOURCES
    src/core/portfolio.cpp
    src/core/scenario.cpp
    src/core/calibration.cpp
    # src/core/simulator.cpp
)

# Core library
add_library(midas_core ${MIDAS_CORE_SOURCES})
target_link_libraries(midas_core PUBLIC pthread)

# Test executables
add_executable(midas_test tests/test_main.cpp)

add_executable(test_portfolio tests/test_portfolio.cpp)
target_link_libraries(test_portfolio midas_core)

add_executable(test_scenario tests/test_scenario.cpp)
target_link_libraries(test_scenario midas_core)

add_executable(test_calibration tests/test_calibration.cpp)
target_link_libraries(test_calibration midas_core)

# Enable testing
enable_testing()
add_test(NAME midas_test COMMAND midas_test)

message(STATUS "Midas configuration complete")
