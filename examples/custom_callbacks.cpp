#include <cstdint>  // std::uint32_t, std::uint64_t
#include <iostream> // std::cout
#include <string>   // std::string

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// This example will show how to use custom statistics to provide callbacks on
// hit or miss. Take a look at file 'custom_statistics.cpp' for more details on
// how custom statistics are implemented

template<typename Key, typename Value>
class CustomCallbacks
{
private:
	// we won't be tracking any stats, so we don't need any variables
	// But of course, you could define some variables and track statistics at the
	// same time you have callbacks.

public:
	///////////////////////////////// Setters /////////////////////////////////

	void clear() noexcept
	{
		std::cout << "ClearCallback: cache cleared!" << std::endl;
	}

	void hit(const Key& k, const Value& v) noexcept
	{
		std::cout << "HitCallback: Hit for entry ("
			<< k << ", " << v << ")" << std::endl;
	}

	void miss(const Key& k) noexcept
	{
		std::cout << "MissCallback: Miss for key '" << k << "'" << std::endl;
	}

	void erase(const Key& k, const Value& v) noexcept
	{
		std::cout << "EraseCallback: Erasing entry ("
			<< k << ", " << v << ")" << std::endl;
	}

	void evict(const Key& k, const Value& v) noexcept
	{
		std::cout << "EvictCallback: Evicted entry ("
			<< k << ", " << v << ")" << std::endl;
	}

	///////////////////////////////// Getters /////////////////////////////////

	// Since we won't be tracking any statistics, we will return 0 for all the
	// following functions.

	size_t hit_count() const noexcept { return 0; }
	size_t miss_count() const noexcept { return 0; }
	size_t entry_invalidation_count() const noexcept { return 0; }
	size_t cache_invalidation_count() const noexcept { return 0; }
	size_t evicted_count() const noexcept { return 0; }
};

int main()
{
	// We create a small cache of 128 int-int key-value pairs, with LRU
	// replacement policy (Policy::LRU), no mutithread support (NullLock), and
	// our statistics/callbacks (CustomCallbacks)
	Cache<int, int, Policy::LRU, NullLock, CustomCallbacks> cache(128);

	// Perform some manipulations on the cache:
	cache.contains(23);
	cache[23] = 4;
	cache.erase(40);
	cache.insert(33, 0);

	auto it = cache.find(90);
	if(it != cache.end())
		std::cout << "key 90 not found!" << std::endl;

	cache.contains(23);
	cache.erase(23);

	// Fill the cache. This will evict some entries
	for(int i = 0; i < 130; i++)
		cache[i] = i;

	// And finally:
	cache.clear();
}
