cmake_minimum_required(VERSION 3.19)

project(TinyTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tiny_test INTERFACE tiny_test.hpp)
