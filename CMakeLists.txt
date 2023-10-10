cmake_minimum_required(VERSION 3.24)

project(orderbook VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

add_library(${CMAKE_PROJECT_NAME}_lib STATIC ${SOURCES})

include(FetchContent)

# OpenSSL
find_package(OpenSSL REQUIRED)

# QuickFix
FetchContent_Declare(
    quickfix
    URL https://github.com/quickfix/quickfix/archive/868e5d1141904318c0178f6ba809721c7a51074d.zip
)
FetchContent_MakeAvailable(quickfix)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}_lib
  quickfix
  OpenSSL::SSL
)
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  quickfix
  OpenSSL::SSL
)

# Google Test
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/2dd1c131950043a8ad5ab0d2dda0e0970596586a.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Testing configuration
enable_testing()

file(GLOB TEST_SOURCES
    tests/*.cpp
)

add_executable(
  ${CMAKE_PROJECT_NAME}_test
  ${TEST_SOURCES}
)
target_link_libraries(
  ${CMAKE_PROJECT_NAME}_test
  GTest::gtest_main
  ${CMAKE_PROJECT_NAME}_lib
)

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME}_test)
