#include <chrono>   // std::chrono_literals
#include <cstdint>  // std::size_t
#include <iostream> // std::cout
#include <thread>   // std::this_thread::sleep_for(...)

#include "Cache/Cache.h"      // class Cache
#include "Cache/Wrapper.h"    // function wrap(...)
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

std::size_t ackermann(std::size_t m, std::size_t n)
{
	// we add a small delay to make this function much slower
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(50ms);

	if(m == 0) return n + 1;
	if(n == 0) return ackermann(m - 1, 1);

	return ackermann(m - 1, ackermann(m, n - 1));
}

// Wrap 'ackermann' in a 16-entry cache with LRU replacement policy
auto cached_ackermann = wrap<Policy::LRU>(ackermann, 16);

int main()
{
	std::cout << "Slow for the first time... " << std::endl;
	std::size_t ret1 = cached_ackermann(2, 5);

	std::cout << "Fast for the second time!" << std::endl;
	std::size_t ret2 = cached_ackermann(2, 5);

	std::cout << std::endl
		<< "First result = " << ret1
		<< ", second result = " << ret2 << std::endl;
}
