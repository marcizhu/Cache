cmake_minimum_required(VERSION 3.11)
project(Cache VERSION 1.0.0 LANGUAGES CXX)

#---------------------------------------------------------------------------------------
# include CMake modules
#---------------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(AddCatchTest)   # Function add_catch_test()
include(EnableCoverage) # Function target_enable_coverage()
include(EnableWarnings) # Function target_enable_warnings()

#---------------------------------------------------------------------------------------
# set default build to debug
#---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose Release or Debug" FORCE)
endif()

#---------------------------------------------------------------------------------------
# compiler config
#---------------------------------------------------------------------------------------
option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(CACHE_BUILD_EXAMPLES "Build example programs" OFF)
option(CACHE_BUILD_TESTS "Build unit tests" OFF)

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

add_library(Cache INTERFACE)
target_include_directories(Cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Cache::Cache ALIAS Cache)

if(CACHE_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(CACHE_BUILD_TESTS)
	include(FetchContent)
	FetchContent_Declare(
	  Catch2
	  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	  GIT_TAG        v2.13.4)

	FetchContent_MakeAvailable(Catch2)

	enable_testing()
	add_subdirectory(tests)
endif()
