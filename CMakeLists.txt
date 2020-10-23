cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-march=native -O3 -std=c++17")
project(sudachi CXX)

add_subdirectory(thirdparties/TFHEpp)
add_subdirectory(thirdparties/taskflow)
add_subdirectory(src)