cmake_minimum_required(VERSION 3.24)

project(orderbook VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Quickfix
find_package(Quickfix REQUIRED)

file(GLOB SOURCES src/*.h src/*.cpp src/datasources/*.h src/datasources/*.cpp)

# Add a static library target for the orderbook
add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${QUICKFIX_DYLIB} OpenSSL::SSL)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${QUICKFIX_INCLUDE_PATH})

# Add a CLI target for the orderbook
add_executable(${CMAKE_PROJECT_NAME}_cli ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME}_cli ${CMAKE_PROJECT_NAME} ${QUICKFIX_DYLIB} OpenSSL::SSL)
target_include_directories(${CMAKE_PROJECT_NAME}_cli PRIVATE ${QUICKFIX_INCLUDE_PATH})

# Testing configuration
enable_testing()

# Find Google Test package
find_package(GTest REQUIRED)

file(GLOB TEST_SOURCES tests/*.cpp)

# Add a test target for the orderbook
add_executable(${CMAKE_PROJECT_NAME}_test ${TEST_SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME}_test GTest::gtest_main
                      ${CMAKE_PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME}_test)
