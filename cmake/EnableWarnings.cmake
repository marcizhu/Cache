# function to enable most warnings on GCC/G++/Clang/AppleClang/MSVC
# -Wfloat-equal
function(target_enable_warnings target)
	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
	target_compile_options(${target} PRIVATE
		-Wall -Wextra -Wdouble-promotion -Wshadow -Wformat=2 -Wno-variadic-macros
		-Wcast-align -Wstrict-aliasing=2 -Wstrict-overflow=5
		-Wwrite-strings -Wnon-virtual-dtor -Wcast-align -Wunused
		-Woverloaded-virtual -Wconversion -pedantic -Wsign-conversion)
	elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		target_compile_options(${target} PRIVATE /W3)
		target_compile_options(${target} PRIVATE /bigobj)
		add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
		add_compile_definitions(_USE_MATH_DEFINES)
	endif()
endfunction()
