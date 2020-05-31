#include <cstdint>  // std::uint32_t, std::uint64_t
#include <iostream> // std::cout
#include <string>   // std::string

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// To create custom statistics, we just need to create our own templated class
// for it and provide a vary basic interface. The implementation of the functions
// will define the statistics behaviour
template<typename Key, typename Value>
class CustomStats
{
private:
	std::uint32_t m_Hits{};
	std::uint32_t m_Misses{};

public:
	// All custom statistics classes will have to provide the following functions:

	///////////////////////////////// Setters /////////////////////////////////

	// This function gets called when we flush() or clear() the cache
	void clear() noexcept {}

	// This function gets called when the cache registers a hit. Hits and misses
	// only come from functions like contains(), find(), erase(), count() and
	// flush(key). Never from insertion or modification functions (that is, at(),
	// operator[], insert(), lookup()) nor clear()/flush() functions.
	// The arameters given to this function are key and value of the hit. In this
	// example, we will only register hits & misses of key == "test"
	void hit(const Key& k, const Value&) noexcept { if(k == "test") m_Hits++; }

	// This function gets called when the cache registers a miss. Again, it is
	// given the key that provoked the miss. Just like before, we will track how
	// many misses the key "test" generated.
	void miss(const Key& k) noexcept { if(k == "test") m_Misses++; }

	// This function gets called when a key is erased() or flush()'d by the user.
	// The original key-value pair are passed to the function. In this short
	//example, we will do nothing.
	void erase(const Key&, const Value&) noexcept {}

	// This function gets called when a key is erased in order to make room for
	// a newer key. The original key-value pair are passed to the function. Again,
	// we will do nothing with the arguments.
	void evict(const Key&, const Value&) noexcept {}

	///////////////////////////////// Getters /////////////////////////////////

	// This function gets called whenever we call Cache::hit_count(). Thus, it
	// will return the number of hits for the key we are tracking.
	size_t hit_count() const noexcept { return m_Hits; }

	// This function gets called whenever we call Cache::miss_count(). Thus, it
	// will return the number of misses for the key we are tracking.
	size_t miss_count() const noexcept { return m_Misses; }

	// This function gets called whenever we call Cache::entry_invalidation_count()
	// (aka how many  objects have been erase()'d/flush()'d' by the user). Since
	// we are not recording any stats  for this parameter, we will just return 0.
	size_t entry_invalidation_count() const noexcept { return 0; }

	// This function gets called whenever we call Cache::cache_invalidation_count()
	// (aka how many  times has the cache been flush()'d or clear()'d). Since we
	// are not recording any stats for this parameter, we will just return 0.
	size_t cache_invalidation_count() const noexcept { return 0; }

	// This function gets called whenever we call Cache::evicted_count() (aka
	// how many entries have  been erased from the cache to make room for newer
	// entries). Again, we will return 0 since this is not important for us.
	size_t evicted_count() const noexcept { return 0; }
};

int main()
{
	// We create a small cache of 128 string-int key-value pairs, with LRU
	// replacement policy (Policy::LRU), no mutithread support (NullLock), and
	// our statistics (CustomStats)
	Cache<std::string, int, Policy::LRU, NullLock, CustomStats> cache(128);

	cache.contains("asdf"); // Miss, but ignored by our code
	cache.contains("test"); // Miss
	cache.contains("something"); // Miss, ignored by our code

	cache.insert("test", 42); // Insertion function, does not count as a hit/miss

	cache.contains("test"); // Hit
	auto it = cache.find("test"); // Hit

	if(it != cache.end())
		std::cout << "'test' is " << it->second << std::endl;

	cache["test"] = 9; // Modification function, does not count as a hit nor miss

	// operator[] again. In this case, it does not modify the value but if the
	// key is not present on the cache, it will insert a new one with a default
	// value (just like a std::map). Thus, it won't count as a hit nor a miss.
	std::cout << "Now 'test' is " << cache["test"] << std::endl << std::endl;

	// The following should print 2 hits and 1 miss:
	std::cout << "Hits for key 'test': " << cache.hit_count() << std::endl;
	std::cout << "Misses for key 'test': " << cache.miss_count() << std::endl;
}
