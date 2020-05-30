#include <cstdint>  // std::uint64_t
#include <iostream> // std::cout

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

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
	Cache<std::uint64_t, std::uint64_t, Policy::LRU> cache(100);

	std::size_t hits_before   = cache.hit_count ();
	std::size_t misses_before = cache.miss_count();

	// Since the cache is not static, a new cache is created for each function call. Thus, you will see
	// that before calling there is always 0 hits and 0 misses and after calling there is (n - 3) hits
	// and (n - 1) misses.

	// The misses correspond to the first `cache.contains(n)` call from the first recursive call for each
	// value of n, and the hits correspond to the `cache.contains(n)` call from the second recursive call
	// for all differnt values of n

	std::cout << "Statistics before calling function: "
		<< hits_before << " hits, "
		<< misses_before << " misses" << std::endl;

	auto value = cached_fibonacci(n, cache);

	std::cout << "Statistics after calling function: "
		<< cache.hit_count() << " hits, "
		<< cache.miss_count() << " misses" << std::endl << std::endl;

	return value;
}

int main()
{
	cached_fibonacci( 0);
	cached_fibonacci( 2);
	cached_fibonacci(40);
	cached_fibonacci(45);
	cached_fibonacci(50);
	cached_fibonacci(90);
}
