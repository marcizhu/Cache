#include <string>

#include "Cache/Cache.h"
#include "Cache/Policy/LFU.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Cache w/ LFU replacement policy: LFU behaviour", "[cache][behaviour][lfu]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("Replaced item is the least frequently used (1/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			REQUIRE(cache.contains(std::to_string(i)) == true);

		for(size_t i = 2; i <= MAX_SIZE; i++)
			REQUIRE(cache.contains(std::to_string(i)) == true);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.evicted_count() == 0);

		cache.insert("asdf", 42);
		CHECK(cache.contains("1") == false);
		CHECK(cache.evicted_count() == 1);
	}

	SECTION("Replaced item is the least frequently used (2/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			REQUIRE(cache.find(std::to_string(i)) != cache.end());

		for(size_t i = 2; i <= MAX_SIZE; i++)
			REQUIRE(cache.find(std::to_string(i)) != cache.end());

		cache.insert("asdf", 42);
		CHECK(cache.contains("1") == false);
		CHECK(cache.evicted_count() == 1);
	}

	SECTION("Replaced item is the least frequently used (3/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			REQUIRE(cache.contains(std::to_string(i)) == true);

		for(size_t i = 1; i < MAX_SIZE; i++)
			REQUIRE(cache.contains(std::to_string(i)) == true);

		cache.insert("asdf", 42);
		CHECK(cache.contains(std::to_string(MAX_SIZE)) == false);
		CHECK(cache.evicted_count() == 1);
	}

	SECTION("Replaced item is the least frequently used (4/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			REQUIRE(cache.find(std::to_string(i)) != cache.end());

		for(size_t i = 1; i < MAX_SIZE; i++)
			REQUIRE(cache.find(std::to_string(i)) != cache.end());

		cache.insert("asdf", 42);
		CHECK(cache.contains(std::to_string(MAX_SIZE)) == false);
		CHECK(cache.evicted_count() == 1);
	}
}
