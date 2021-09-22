# Options
option(CODE_COVERAGE "Builds targets with code coverage instrumentation. (Requires GCC or Clang)" OFF)

if(CODE_COVERAGE AND NOT CODE_COVERAGE_ADDED)
    set(CODE_COVERAGE_ADDED ON)

    if(CMAKE_BUILD_TYPE)
        string(TOUPPER ${CMAKE_BUILD_TYPE} upper_build_type)
        if(NOT ${upper_build_type} STREQUAL "DEBUG")
            message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
        endif()
    else()
        message(WARNING "Code coverage results with an optimized (non-Debug) build may be misleading")
    endif()
endif()


# Adds code coverage instrumentation to all targets in the current directory and
# any subdirectories. To add coverage instrumentation to only specific targets,
# use `target_code_coverage`.
function(add_code_coverage)
    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang" OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        link_libraries(gcov)
    else()
        message(STATUS "Code coverage requires Clang or GCC (ignored)")
    endif()
endfunction()

# target_code_coverage(<name> # Name of the target to enable code coverage
# [PRIVATE]                   # Enable code coverage only for that target
# [PUBLIC]                    # Enable code coverage for this target and
#                             # everything that links to it
# [INTERFACE]                 # Enable code coverage to anything that links
#                             # to this target
# )
function(target_code_coverage TARGET_NAME)
    cmake_parse_arguments(target_code_coverage
        "PRIVATE;PUBLIC;INTERFACE"
        ""
        ""
        ${ARGN})

    # Set the visibility of target functions to PUBLIC, INTERFACE or default to PRIVATE
    if(target_code_coverage_PUBLIC)
        set(TARGET_VISIBILITY PUBLIC)
    elseif(target_code_coverage_INTERFACE)
        set(TARGET_VISIBILITY INTERFACE)
    else()
        set(TARGET_VISIBILITY PRIVATE)
    endif()

    if(CODE_COVERAGE)
        # Add code coverage instrumentation to the target's linker command
        if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang" OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
            target_compile_options(${TARGET_NAME} ${TARGET_VISIBILITY} -fprofile-arcs -ftest-coverage)
            target_link_options(${TARGET_NAME} ${TARGET_VISIBILITY} --coverage)
        elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${TARGET_NAME} ${TARGET_VISIBILITY} -fprofile-arcs -ftest-coverage)
            target_link_libraries(${TARGET_NAME} ${TARGET_VISIBILITY} gcov)
        else()
            message(STATUS "Code coverage for target ${TARGET_NAME} requires Clang or GCC (ignored)")
        endif()
    endif()
endfunction()
