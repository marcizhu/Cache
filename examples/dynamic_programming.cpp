#include <cstdint>  // std::uint64_t
#include <iostream> // std::cout

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// Simple uncached recursive fibonacci function
std::uint64_t uncached_fibonacci(std::uint64_t n)
{
	if(n < 2) return n;

	return uncached_fibonacci(n - 1) + uncached_fibonacci(n - 2);
}

// Optimized, dynamic-programming style fibonacci function using a cache
template<typename Cache>
std::uint64_t cached_fibonacci(std::uint64_t n, Cache& cache)
{
	if(n < 2) return n;

	// If the cache contains the pair (n, result), return the result directly
	if(cache.contains(n)) return cache[n];

	// If not, calculate it using recursive calls and store it in cache
	auto value = cached_fibonacci(n - 1, cache) + cached_fibonacci(n - 2, cache);
	cache.insert(n, value);

	// Finally, return the calculated value
	return value;
}

std::uint64_t cached_fibonacci(std::uint64_t n)
{
	// This is a small wrapper function that will store a cache for our cached_fibonacci() function.
	// It will forward this cache to the 'client' function

	// We create a cache with 100 entries and LRU replacement policy
	static Cache<std::uint64_t, std::uint64_t, Policy::LRU> cache(100);
	return cached_fibonacci(n, cache);
}

int main()
{
	std::cout << "uncached_fibonacci( 0) = " << std::flush << uncached_fibonacci( 0) << std::endl; // should be 0
	std::cout << "uncached_fibonacci( 2) = " << std::flush << uncached_fibonacci( 2) << std::endl; // should be 1
	std::cout << "uncached_fibonacci(40) = " << std::flush << uncached_fibonacci(40) << std::endl; // should be 102334155
	std::cout << "uncached_fibonacci(45) = " << std::flush << uncached_fibonacci(45) << std::endl; // should be 1134903170

	// The following two lines take too long. uncached_fibonacci(50) might be doable but uncached_fibonacci(90)
	// will take an unreasonable amount of time. Don't even bother trying that one! >.<

//	std::cout << "uncached_fibonacci(50) = " << std::flush << uncached_fibonacci(50) << std::endl; // should be 12586269025
//	std::cout << "uncached_fibonacci(90) = " << std::flush << uncached_fibonacci(90) << std::endl; // should be 2880067194370816120

	std::cout << std::endl;

	// In contrast, all of the following take almost no time at all
	std::cout << "cached_fibonacci( 0) = " << std::flush << cached_fibonacci( 0) << std::endl; // should be 0
	std::cout << "cached_fibonacci( 2) = " << std::flush << cached_fibonacci( 2) << std::endl; // should be 1
	std::cout << "cached_fibonacci(40) = " << std::flush << cached_fibonacci(40) << std::endl; // should be 102334155
	std::cout << "cached_fibonacci(45) = " << std::flush << cached_fibonacci(45) << std::endl; // should be 1134903170
	std::cout << "cached_fibonacci(50) = " << std::flush << cached_fibonacci(50) << std::endl; // should be 12586269025
	std::cout << "cached_fibonacci(90) = " << std::flush << cached_fibonacci(90) << std::endl; // should be 2880067194370816120
}
