cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_FLAGS "-march=native -O3 -g -std=c++17")
project(sudachi CXX)
OPTION(USE_M12 "Use M_12 encoded message space" OFF) 

if(USE_M12)
	add_compile_definitions(USE_M12)
endif()

if(USE_SPQLIOX_AARCH64)
  add_compile_definitions(USE_SPQLIOX_AARCH64)
endif()

#For Taskflow
find_package(Threads REQUIRED)
# For OpenMP
find_package(OpenMP)
if(OpenMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(USE_80BIT_SECURITY OFF CACHE BOOL "")
add_subdirectory(thirdparties/oveus-tfhe)
add_subdirectory(src)
add_subdirectory(test)
