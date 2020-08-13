cmake_minimum_required(VERSION 3.1)

#---------------------------------------------------------------------------------------
# set default build to debug
#---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose Release or Debug" FORCE)
endif()

#---------------------------------------------------------------------------------------
# project config
#---------------------------------------------------------------------------------------
add_executable(API.test  CacheAPI.cpp)
add_executable(FIFO.test FIFOCache.cpp)
add_executable(LFU.test  LFUCache.cpp)
add_executable(LIFO.test LIFOCache.cpp)
add_executable(LRU.test  LRUCache.cpp)
add_executable(MRU.test  MRUCache.cpp)

target_link_libraries(API.test  PRIVATE Catch2::Catch2 Cache)
target_link_libraries(FIFO.test PRIVATE Catch2::Catch2 Cache)
target_link_libraries(LFU.test  PRIVATE Catch2::Catch2 Cache)
target_link_libraries(LIFO.test PRIVATE Catch2::Catch2 Cache)
target_link_libraries(LRU.test  PRIVATE Catch2::Catch2 Cache)
target_link_libraries(MRU.test  PRIVATE Catch2::Catch2 Cache)

add_test(NAME API.test  COMMAND ${CMAKE_BINARY_DIR}/tests/API.test)
add_test(NAME FIFO.test COMMAND ${CMAKE_BINARY_DIR}/tests/FIFO.test)
add_test(NAME LFU.test  COMMAND ${CMAKE_BINARY_DIR}/tests/LFU.test)
add_test(NAME LIFO.test COMMAND ${CMAKE_BINARY_DIR}/tests/LIFO.test)
add_test(NAME LRU.test  COMMAND ${CMAKE_BINARY_DIR}/tests/LRU.test)
add_test(NAME MRU.test  COMMAND ${CMAKE_BINARY_DIR}/tests/MRU.test)

set_target_properties(API.test  PROPERTIES CXX_STANDARD 14)
set_target_properties(FIFO.test PROPERTIES CXX_STANDARD 14)
set_target_properties(LFU.test  PROPERTIES CXX_STANDARD 14)
set_target_properties(LIFO.test PROPERTIES CXX_STANDARD 14)
set_target_properties(LRU.test  PROPERTIES CXX_STANDARD 14)
set_target_properties(MRU.test  PROPERTIES CXX_STANDARD 14)

set_target_properties(API.test  PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(FIFO.test PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(LFU.test  PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(LIFO.test PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(LRU.test  PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(MRU.test  PROPERTIES CXX_STANDARD_REQUIRED ON)

#---------------------------------------------------------------------------------------
# Turn on compiler warnings
#---------------------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
	target_compile_options(API.test  PUBLIC -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion -Wdouble-promotion -Wformat=2)
	target_compile_options(FIFO.test PUBLIC -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion -Wdouble-promotion -Wformat=2)
	target_compile_options(LFU.test  PUBLIC -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion -Wdouble-promotion -Wformat=2)
	target_compile_options(LIFO.test PUBLIC -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion -Wdouble-promotion -Wformat=2)
	target_compile_options(LRU.test  PUBLIC -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion -Wdouble-promotion -Wformat=2)
	target_compile_options(MRU.test  PUBLIC -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion -Wdouble-promotion -Wformat=2)

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(API.test  PUBLIC /W3)
	target_compile_options(FIFO.test PUBLIC /W3)
	target_compile_options(LFU.test  PUBLIC /W3)
	target_compile_options(LIFO.test PUBLIC /W3)
	target_compile_options(LRU.test  PUBLIC /W3)
	target_compile_options(MRU.test  PUBLIC /W3)

	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_definitions(_USE_MATH_DEFINES)
endif()

#---------------------------------------------------------------------------------------
# Enable (or disable) features based on the given options
#---------------------------------------------------------------------------------------
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(API.test  PUBLIC -O0 -g --coverage)
	target_compile_options(FIFO.test PUBLIC -O0 -g --coverage)
	target_compile_options(LFU.test  PUBLIC -O0 -g --coverage)
	target_compile_options(LIFO.test PUBLIC -O0 -g --coverage)
	target_compile_options(LRU.test  PUBLIC -O0 -g --coverage)
	target_compile_options(MRU.test  PUBLIC -O0 -g --coverage)

	if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
		target_link_options(API.test  PUBLIC --coverage)
		target_link_options(FIFO.test PUBLIC --coverage)
		target_link_options(LFU.test  PUBLIC --coverage)
		target_link_options(LIFO.test PUBLIC --coverage)
		target_link_options(LRU.test  PUBLIC --coverage)
		target_link_options(MRU.test  PUBLIC --coverage)
	else()
		target_link_libraries(API.test  PUBLIC --coverage)
		target_link_libraries(LFU.test  PUBLIC --coverage)
		target_link_libraries(LIFO.test PUBLIC --coverage)
		target_link_libraries(LRU.test  PUBLIC --coverage)
		target_link_libraries(MRU.test  PUBLIC --coverage)
	endif()
endif()
