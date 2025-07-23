cmake_minimum_required(VERSION 3.21)

project(meta_state_machine)

set(LANGUAGE CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/state_machine.hpp)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(${PROJECT_NAME}_BUILD_TESTS "Build tests" ON)

if (${PROJECT_NAME}_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
