#include <cstdint>    // std::uint32_t, std::uint64_t
#include <functional> // std::function
#include <iostream>   // std::cout
#include <string>     // std::string

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// This example will show how to implement more advanced callbacks using custom
// statistics. For moree information about custom statistics and callbacks,
// please take a look to files 'custom_statistics.cpp' and 'custom_callbacks.cpp'

template<typename Key, typename Value>
class CustomCallbacks
{
private:
	// For this example, we will have 5 std::functions, one for each event.
	// Additionally, we will have 5 extra functions to set our callbacks and
	// change them at will.

	std::function<void(void)> clearCback;
	std::function<void(const Key&, const Value&)> hitCback;
	std::function<void(const Key&)> missCback;
	std::function<void(const Key&, const Value&)> eraseCback;
	std::function<void(const Key&, const Value&)> evictCback;

	// we won't be tracking any stats, so we don't need any variables
	// But of course, you could define some variables and track statistics at the
	// same time you have callbacks.

public:
	///////////////////////////////// Setters /////////////////////////////////

	void clear() noexcept { clearCback(); }
	void hit  (const Key& k, const Value& v) noexcept { hitCback(k, v); }
	void miss (const Key& k) noexcept { missCback(k); }
	void erase(const Key& k, const Value& v) noexcept { eraseCback(k, v); }
	void evict(const Key& k, const Value& v) noexcept { evictCback(k, v); }

	// Our additional functions for setting callbacks
	void setClearCb(const std::function<void(void)>& cb) { clearCback = cb;}
	void setHitCb(const std::function<void(const Key&, const Value&)>& cb) { hitCback = cb; }
	void setMissCb(const std::function<void(const Key&)>& cb) { missCback = cb; }
	void setEraseCb(const std::function<void(const Key&, const Value&)>& cb) { eraseCback = cb; }
	void setEvictCb(const std::function<void(const Key&, const Value&)>& cb) { evictCback = cb; }

	///////////////////////////////// Getters /////////////////////////////////

	// Since we won't be tracking any statistics, we will return 0 for all the
	// following functions.

	size_t hit_count() const noexcept { return 0; }
	size_t miss_count() const noexcept { return 0; }
	size_t entry_invalidation_count() const noexcept { return 0; }
	size_t cache_invalidation_count() const noexcept { return 0; }
	size_t evicted_count() const noexcept { return 0; }
};

void function(int param)
{
	std::cout << "function(" << param << ") called!" << std::endl;
}

int main()
{
	CustomCallbacks<int, int> callbacks;

	// On clear, call our lambda function
	callbacks.setClearCb([]() { std::cout << "My clear callback!" << std::endl; });

	// On miss, call 'function()'
	callbacks.setMissCb(function);

	// Ignore the rest of events...
	callbacks.setHitCb([](const int&, const int&) {});
	callbacks.setEraseCb([](const int&, const int&) {});
	callbacks.setEvictCb([](const int&, const int&) {});

	// We create a small cache of 128 int-int key-value pairs, with LRU
	// replacement policy (Policy::LRU), no mutithread support (NullLock), and
	// our statistics/callbacks (CustomCallbacks).
	// Because we need the cache to use the 'callbacks' object, we will pass it
	// to our cache constructor
	Cache<int, int, Policy::LRU, NullLock, CustomCallbacks> cache(128, Policy::LRU<int>(), callbacks);

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
