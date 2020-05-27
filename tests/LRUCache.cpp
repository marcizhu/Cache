#include <string>

#include "Cache/Cache.h"
#include "Cache/LRUCachePolicy.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Cache w/ LRU replacement policy: Preconditions", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("Original size is 0")
	{
		CHECK(cache.size() == 0);
		CHECK(cache.empty());
	}

	SECTION("Original Hit & Miss ratio are NaN")
	{
		CHECK(cache.hit_count () == 0);
		CHECK(cache.miss_count() == 0);

		CHECK(std::isnan(cache.hit_ratio ()));
		CHECK(std::isnan(cache.miss_ratio()));
	}

	SECTION("max_size() is OK")
	{
		CHECK(cache.max_size() == MAX_SIZE);
	}
}

TEST_CASE("Cache w/ LRU replacement policy: Size", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("Size grows after each insertion")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.size() == i);
		}
	}

	SECTION("Cache is only empty() if size() == 0")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.empty() == true);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.empty() == false);
		}
	}

	SECTION("Size stops growing after size() == max_size()")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.size() == MAX_SIZE);
			CHECK(cache.max_size() == MAX_SIZE);
		}
	}

	SECTION("clear() resets size to 0")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() != 0);
		cache.clear();
		CHECK(cache.size() == 0);
	}
}

TEST_CASE("Cache w/ LRU replacement policy: Hits & misses", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("Every new insertion counts as a miss")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.miss_count() == i);
		}
	}

	SECTION("Every access to existing items counts as a hit")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.has(std::to_string(i));
			CHECK(cache.hit_count() == i);
		}

		CHECK(cache.hit_count() == MAX_SIZE);
		CHECK(cache.miss_count() == MAX_SIZE);
	}

	SECTION("Every access to non-existing items counts as a miss (using has)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.miss_count() == MAX_SIZE);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			cache.has(std::to_string(i));
			CHECK(cache.hit_count() == 0);
			CHECK(cache.miss_count() == i);
		}

		CHECK(cache.hit_count() == 0);
		CHECK(cache.miss_count() == 2 * MAX_SIZE);
	}

	SECTION("Every access to non-existing items counts as a miss (using find)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.miss_count() == MAX_SIZE);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			cache.find(std::to_string(i));
			CHECK(cache.hit_count() == 0);
			CHECK(cache.miss_count() == i);
		}

		CHECK(cache.hit_count() == 0);
		CHECK(cache.miss_count() == 2 * MAX_SIZE);
	}
}

TEST_CASE("Cache w/ LRU replacement policy: find()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("find() for an existing item returns != end()")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			auto it = cache.find(std::to_string(i));
			CHECK(it != cache.end());
		}
	}

	SECTION("find() for a non-existing item returns end()")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			auto it = cache.find(std::to_string(i));
			CHECK(it == cache.end());
		}
	}

	SECTION("find() for an existing item returns != end()")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			auto it = cache.find(std::to_string(i));
			CHECK(it != cache.end());
		}
	}

	SECTION("find() for an existing item returns the correct item")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			auto it = cache.find(std::to_string(i));
			REQUIRE(it != cache.end());
			CHECK(it->first == std::to_string(i));
			CHECK(it->second == (int)i);
		}
	}
}

TEST_CASE("Cache w/ LRU replacement policy: has()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("has() for an existing item returns true")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			bool has = cache.has(std::to_string(i));
			CHECK(has == true);
		}
	}

	SECTION("has() for a non-existing item returns false")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			bool has = cache.has(std::to_string(i));
			CHECK(has == false);
		}
	}
}

TEST_CASE("Cache w/ LRU replacement policy: flush()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("flush() resets size to 0")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() != 0);
		cache.flush();
		CHECK(cache.size() == 0);
	}

	SECTION("flush(key) reduces size by 1")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t old_size = cache.size();

		REQUIRE(cache.size() != 0);
		REQUIRE(cache.size() == old_size);
		cache.flush(std::to_string((rand() % 128) + 1));
		CHECK(cache.size() == old_size - 1);
	}

	SECTION("Flushed key is not cached")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss_count = cache.miss_count();

		REQUIRE(cache.miss_count() == miss_count);
		std::string key = std::to_string((rand() % 128) + 1);
		cache.flush(key);
		bool has = cache.has(key);
		CHECK(has == false);
		CHECK(cache.miss_count() == miss_count + 1);
	}
}

TEST_CASE("Cache w/ LRU replacement policy: erase()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("erase(key) reduces size by 1")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t old_size = cache.size();

		REQUIRE(cache.size() != 0);
		REQUIRE(cache.size() == old_size);
		cache.erase(std::to_string((rand() % 128) + 1));
		CHECK(cache.size() == old_size - 1);
	}

	SECTION("Erased key is not cached")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss_count = cache.miss_count();

		REQUIRE(cache.miss_count() == miss_count);
		std::string key = std::to_string((rand() % 128) + 1);
		cache.erase(key);
		bool has = cache.has(key);
		CHECK(has == false);
		CHECK(cache.miss_count() == miss_count + 1);
	}
}

TEST_CASE("Cache w/ LRU replacement policy: at()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("at() return value for existing keys")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			auto item = cache.at(std::to_string(i));
			CHECK(item == (int)i);
		}
	}

	SECTION("at() throws for non-exisiting keys")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
			CHECK_THROWS(cache.at(std::to_string(i)));
	}
}

TEST_CASE("Cache w/ LRU replacement policy: LRU behaviour", "[cache][lru]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, LRUCachePolicy<std::string>> cache(MAX_SIZE);

	SECTION("Replaced item is the least recently used (1/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());

		cache.insert("asdf", 42);
		CHECK(cache.has("1") == false);
	}

	SECTION("Replaced item is the least recently used (2/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());

		CHECK(cache.has("1") == true);
		cache.insert("asdf", 42);
		CHECK(cache.has("2") == false);
	}

	SECTION("Replaced item is the least recently used (3/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			CHECK(cache.has(std::to_string(i)) == true);

		REQUIRE(cache.size() == cache.max_size());

		cache.insert("asdf", 42);
		CHECK(cache.has("1") == false);
	}

	SECTION("Replaced item is the least recently used (4/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			CHECK(cache.find(std::to_string(i)) != cache.end());

		REQUIRE(cache.size() == cache.max_size());

		cache.insert("asdf", 42);
		CHECK(cache.has("1") == false);
	}
}
