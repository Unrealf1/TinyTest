cmake_minimum_required(VERSION 3.19)

project(TinyTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

try_compile(
    HAS_SOURCE_LOCATION
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp)

if (!${HAS_SOURCE_LOCATION})
    add_compile_definitions(TINY_TEST__NO_SOURCE_LOCATION)
endif()

add_library(tiny_test INTERFACE tiny_test.hpp)
