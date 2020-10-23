cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-march=native -O3 -std=c++17")
project(sudachi CXX)

find_package(Threads REQUIRED)

set(USE_80BIT_SECURITY OFF CACHE BOOL "")
add_subdirectory(thirdparties/TFHEpp)
add_subdirectory(src)