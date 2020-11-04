cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-march=native -O3 -std=c++17")
project(sudachi CXX)

#For Taskflow
find_package(Threads REQUIRED)
# For OpenMP
find_package(OpenMP)
if(OpenMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(USE_80BIT_SECURITY OFF CACHE BOOL "")
add_subdirectory(thirdparties/TFHEpp)
add_subdirectory(src)
add_subdirectory(test)