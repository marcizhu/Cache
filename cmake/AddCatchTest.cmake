# Function to create a common main
function(common_catch_main)
	if(TARGET common_catch_main_object)
		return()
	endif()

	file(WRITE "${CMAKE_BINARY_DIR}/common_catch_main.cc"
		"#define CATCH_CONFIG_MAIN\n"
		"#include \"catch2/catch.hpp\"\n"
	)

	add_library(common_catch_main_object OBJECT "${CMAKE_BINARY_DIR}/common_catch_main.cc")
	target_link_libraries(common_catch_main_object PRIVATE Catch2::Catch2)
endfunction()

# add_catch_test_with_seed(<name>  # Name of the test instance
#    <exec>                   # Executable of the test, generally test_<name>
#    <seed>                   # A number
#    [NOCATCHLABEL]           # Do not add ``catch`` as a CTest label
#    [WORKING_DIRECTORY dir]  # Directory from where to launch the test
#    [TIMEOUT timeout]        # Set test timeout, in seconds
#    [LABELS lab1 lab2...]    # CTest labels to add to the test.
#    [ARGUMENTS arg1 arg2...] # Extra arguments when running the test.
# )

# A function to create a test, once a an executable exists
function(add_catch_test_with_seed testname testexec seed)
	cmake_parse_arguments(catch "NOCATCHLABEL" "WORKING_DIRECTORY;TIMEOUT" "LABELS;ARGUMENTS" ${ARGN})

	unset(EXTRA_ARGS)
	if(catch_WORKING_DIRECTORY)
		set(EXTRA_ARGS WORKING_DIRECTORY ${catch_WORKING_DIRECTORY})
	endif()

	set(arguments ${catch_ARGUMENTS})
	if(NOT "${seed}" STREQUAL "")
		list(APPEND arguments --rng-seed ${seed})
	else()
		list(APPEND arguments --rng-seed time)
	endif()

	if(CATCH_JUNIT)
		add_test(NAME ${testname}
			COMMAND ${testexec}
					${arguments}
					-r junit
					-o ${PROJECT_BINARY_DIR}/Testing/${testname}.xml
		)
	else()
		add_test(NAME ${testname} COMMAND ${testexec} ${arguments} ${EXTRA_ARGS})
	endif()

	if(catch_TIMEOUT)
		set_tests_properties(${testname} PROPERTIES TIMEOUT ${catch_TIMEOUT})
	endif()

	if(NOT catch_NOCATCHLABEL)
		list(APPEND catch_LABELS catch)
	endif()
	set_tests_properties(${testname} PROPERTIES LABELS "${catch_LABELS}")
endfunction()

# add_catch_test(<name>       # The test will be called test_<name>
#    [source1 source2...]     # Sources for the test. Defaults to ``<name>.cc`` if no sources are given.
#    [NOMAIN]                 # By default, a standard main is added to the test.
#                             # This option specifies that one will be given, either in the sources,
#                             # or as a target (object archive) called common_catch_main_object
#    [NOTEST]                 # Do not add to CTest (don't run on make test). Many of the options below
#                             # will not be useful if NOTEST is enabled.
#    [NOCATCHLABEL]           # Do not add ``catch`` as a CTest label
#    [SEED number|time]       # Adds a specific seed. Defaults to time.
#    [WORKING_DIRECTORY dir]  # Directory from where to launch the test
#    [COMMON_MAIN obj_tgt]    # Specifies an object target to be used as a common main for all tests
#    [PRECOMMAND command]     # Specifies a command to be executed *before* the test is executed
#    [CXX_STANDARD ver]       # Set C++ Standard version (98/03/11/14/17/20)
#    [TIMEOUT timeout]        # Set the test timeout, in seconds
#    [LIBRARIES lib1 lib2...] # Libraries the executable should link against. 
#    [DEPENDS dep1 dep2 ...]  # Targets the executable depends on.
#    [INCLUDES inc1 inc2...]  # Include directories the executable requires for compilation
#    [LABELS lab1 lab2...]    # CTest labels to add to the test.
#    [ARGUMENTS arg1 arg2...] # Extra arguments when running the test.
# )

# Then adds a function to create a test
function(add_catch_test testname)
	cmake_parse_arguments(catch
		"NOMAIN;NOTEST;NOCATCHLABEL"
		"SEED;WORKING_DIRECTORY;COMMON_MAIN;PRECOMMAND;CXX_STANDARD;TIMEOUT"
		"LIBRARIES;DEPENDS;INCLUDES;LABELS;ARGUMENTS"
		${ARGN}
	)

	# Source deduce from testname if possible
	unset(source)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${testname}.cc")
		set(source ${testname}.cc)
	elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${testname}.cpp")
		set(source ${testname}.cpp)
	elseif("${catch_UNPARSED_ARGUMENTS}" STREQUAL "")
		message(FATAL_ERROR "No source given or found for ${testname}")
	endif()

	# By default, uses a common main function for all, compiled once
	# We create here
	if(catch_NOMAIN)
		add_executable(${testname} ${source} ${catch_UNPARSED_ARGUMENTS})
	elseif(catch_COMMON_MAIN)
		add_executable(${testname}
			${source} $<TARGET_OBJECTS:${catch_COMMON_MAIN}> ${catch_UNPARSED_ARGUMENTS})
	else()
		common_catch_main()
		add_executable(${testname}
			${source} $<TARGET_OBJECTS:common_catch_main_object> ${catch_UNPARSED_ARGUMENTS})
	endif()

	if(catch_CXX_STANDARD)
		set_target_properties(${testname} PROPERTIES CXX_STANDARD ${catch_CXX_STANDARD})
		set_target_properties(${testname} PROPERTIES CXX_STANDARD_REQUIRED ON)
	endif()

	if(catch_LIBRARIES)
		target_link_libraries(${testname} ${catch_LIBRARIES} Catch2::Catch2)
	endif()

	if(catch_INCLUDES)
		target_include_directories(${testname} PRIVATE ${catch_INCLUDES})
	endif()

	if(catch_DEPENDS)
		add_dependencies(${testname} ${catch_DEPENDS})
	endif()

	if(TARGET lookup_dependencies)
		add_dependencies(${testname} lookup_dependencies)
	endif()

	if(catch_NOCATCHLABEL)
		set(catch_NOCATCHLABEL "NOCATCHLABEL")
	else()
		unset(catch_NOCATCHLABEL)
	endif()

	set(test_command ${testname})
	if(catch_PRECOMMAND)
		set(test_command "${catch_PRECOMMAND} ${test_command}")
	endif()

	if(NOT catch_NOTEST)
		add_catch_test_with_seed(
			${testname} "${testname}" "${catch_SEED}" ${catch_UNPARSED_ARGUMENTS}
			${catch_NOCATCHLABEL} WORKING_DIRECTORY ${catch_WORKING_DIRECTORY} TIMEOUT ${catch_TIMEOUT}
			LABELS ${catch_LABELS} ARGUMENTS ${catch_ARGUMENTS}
		)
	endif()
endfunction()
