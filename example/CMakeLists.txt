cmake_minimum_required(VERSION 3.19)

project(TinyTest-Example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(..)

add_executable(example main.cpp)
target_link_libraries(example INTERFACE tiny_test)
