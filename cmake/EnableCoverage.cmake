# function to enable code coverage on GCC/G++/Clang/AppleClang/MSVC
function(target_enable_coverage target)
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(${target} PUBLIC -O0 -g --coverage)

		if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
			target_link_options(${target} PUBLIC --coverage)
		else()
			target_link_libraries(${target} PUBLIC --coverage)
		endif()
	endif()
endfunction()
