set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#---------------------------------------------------------------------------------------
# Create examples
#---------------------------------------------------------------------------------------
add_executable(function_wrapping function_wrapping.cpp)
add_executable(dynamic_programming dynamic_programming.cpp)
add_executable(statistics statistics.cpp)
add_executable(disable_stats disable_statistics.cpp)
add_executable(custom_stats custom_statistics.cpp)
add_executable(custom_callbacks custom_callbacks.cpp)
add_executable(custom_replacement_policy custom_replacement_policy.cpp)
add_executable(advanced_custom_callbacks advanced_custom_callbacks.cpp)
add_executable(multithread_cache multithread_cache.cpp)
add_executable(multithread_function_wrapping multithread_function_wrapping.cpp)

#---------------------------------------------------------------------------------------
# Link examples with the library
#---------------------------------------------------------------------------------------
target_link_libraries(function_wrapping             PRIVATE Cache::Cache)
target_link_libraries(dynamic_programming           PRIVATE Cache::Cache)
target_link_libraries(statistics                    PRIVATE Cache::Cache)
target_link_libraries(disable_stats                 PRIVATE Cache::Cache)
target_link_libraries(custom_stats                  PRIVATE Cache::Cache)
target_link_libraries(custom_callbacks              PRIVATE Cache::Cache)
target_link_libraries(custom_replacement_policy     PRIVATE Cache::Cache)
target_link_libraries(advanced_custom_callbacks     PRIVATE Cache::Cache)
target_link_libraries(multithread_cache             PRIVATE Cache::Cache Threads::Threads)
target_link_libraries(multithread_function_wrapping PRIVATE Cache::Cache Threads::Threads)

#---------------------------------------------------------------------------------------
# Set C++ standard
#---------------------------------------------------------------------------------------
set_target_properties(function_wrapping             PROPERTIES CXX_STANDARD 14)
set_target_properties(dynamic_programming           PROPERTIES CXX_STANDARD 14)
set_target_properties(statistics                    PROPERTIES CXX_STANDARD 14)
set_target_properties(disable_stats                 PROPERTIES CXX_STANDARD 14)
set_target_properties(custom_stats                  PROPERTIES CXX_STANDARD 14)
set_target_properties(custom_callbacks              PROPERTIES CXX_STANDARD 14)
set_target_properties(custom_replacement_policy     PROPERTIES CXX_STANDARD 14)
set_target_properties(advanced_custom_callbacks     PROPERTIES CXX_STANDARD 14)
set_target_properties(multithread_cache             PROPERTIES CXX_STANDARD 14)
set_target_properties(multithread_function_wrapping PROPERTIES CXX_STANDARD 14)

set_target_properties(function_wrapping             PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(dynamic_programming           PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(statistics                    PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(disable_stats                 PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(custom_stats                  PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(custom_callbacks              PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(custom_replacement_policy     PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(advanced_custom_callbacks     PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(multithread_cache             PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(multithread_function_wrapping PROPERTIES CXX_STANDARD_REQUIRED ON)
