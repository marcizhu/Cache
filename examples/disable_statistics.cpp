#include <cstdint>  // std::uint64_t
#include <iostream> // std::cout
#include <string>   // std::string

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)
#include "Cache/Stats/None.h" // Stats::None

int main()
{
	// We create a small cache of string-int64_t key-value pairs, with LRU replacement
	// policy (Policy::LRU), no mutithread support (NullLock), and no statistics
	// (Stats::None)
	Cache<std::string, std::uint64_t, Policy::LRU, NullLock, Stats::None> cache(100);

	cache.contains("non-existing key");

	// If stats are disabled, all statistical functions will return zero.
	std::cout << "This should be zero: " << cache.miss_count() << std::endl;
}
