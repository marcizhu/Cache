cmake_minimum_required(VERSION 3.1)
project(Cache VERSION 0.0.1 LANGUAGES CXX)

#---------------------------------------------------------------------------------------
# set default build to debug
#---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose Release or Debug" FORCE)
endif()

#---------------------------------------------------------------------------------------
# compiler config
#---------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(CACHE_BUILD_TESTS "Build unit tests" OFF)

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

add_library(Cache INTERFACE)
target_include_directories(Cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(CACHE_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
	add_subdirectory(deps/Catch2)
endif()
