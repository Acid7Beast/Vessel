cmake_minimum_required(VERSION 3.20)

# Set the project name and version
set(PROJECT_NAME Flow)
project("${PROJECT_NAME}")

# Set C++ standard and flags for compilation
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++20 -O3")

# Set installation directories
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

# Define the library
add_library(${PROJECT_NAME} INTERFACE)  # Or STATIC for static library

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

file(GLOB_RECURSE TARGET_HEADERS src/*.h)
file(GLOB_RECURSE TARGET_SOURCES src/*.cpp)

# Add source files
target_sources(${PROJECT_NAME} INTERFACE
    ${TARGET_HEADERS}
    ${TARGET_SOURCES}
)

# Add include directories
target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Fetch GoogleTest as a dependency
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.0
)

# Prevent overriding compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Collect test sources
file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

# Create test executable for start
add_executable(${PROJECT_NAME}Test ${TEST_SOURCES} ${TARGET_HEADERS} ${TARGET_SOURCES})

# Link libraries for test executable
target_link_libraries(${PROJECT_NAME}Test PRIVATE ${PROJECT_NAME} GTest::gtest GTest::gtest_main)

# Add src include for tests
target_include_directories(${PROJECT_NAME}Test PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

# Discover and register GoogleTest tests
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}Test)