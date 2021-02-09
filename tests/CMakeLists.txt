cmake_minimum_required(VERSION 3.1)

#---------------------------------------------------------------------------------------
# compiler config
#---------------------------------------------------------------------------------------
add_catch_test(API.test  CacheAPI.cpp  LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)
add_catch_test(FIFO.test FIFOCache.cpp LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)
add_catch_test(LFU.test  LFUCache.cpp  LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)
add_catch_test(LIFO.test LIFOCache.cpp LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)
add_catch_test(LRU.test  LRUCache.cpp  LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)
add_catch_test(MRU.test  MRUCache.cpp  LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)
add_catch_test(Wrap.test Wrapper.cpp   LIBRARIES Catch2::Catch2 Cache CXX_STANDARD 14 TIMEOUT 10)

#---------------------------------------------------------------------------------------
# Turn on compiler warnings
#---------------------------------------------------------------------------------------
target_enable_warnings(API.test)
target_enable_warnings(FIFO.test)
target_enable_warnings(LFU.test)
target_enable_warnings(LIFO.test)
target_enable_warnings(LRU.test)
target_enable_warnings(MRU.test)
target_enable_warnings(Wrap.test)

#---------------------------------------------------------------------------------------
# Enable (or disable) features based on the given options
#---------------------------------------------------------------------------------------
if(CODE_COVERAGE)
	target_enable_coverage(API.test)
	target_enable_coverage(FIFO.test)
	target_enable_coverage(LFU.test)
	target_enable_coverage(LIFO.test)
	target_enable_coverage(LRU.test)
	target_enable_coverage(MRU.test)
	target_enable_coverage(Wrap.test)
endif()
