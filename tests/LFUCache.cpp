#include <string>

#include "Cache/Cache.h"
#include "Cache/Policy/LFU.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Cache w/ LFU replacement policy: Preconditions", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("Original size is 0")
	{
		CHECK(cache.size() == 0);
		CHECK(cache.empty());
	}

	SECTION("Original stats are set to zero")
	{
		CHECK(cache.access_count() == 0);
		CHECK(cache.evicted_count() == 0);
		CHECK(cache.hit_count () == 0);
		CHECK(cache.entry_invalidation_count() == 0);
		CHECK(cache.cache_invalidation_count() == 0);
		CHECK(cache.miss_count() == 0);

		CHECK(std::isnan(cache.hit_ratio ()));
		CHECK(std::isnan(cache.miss_ratio()));
	}

	SECTION("max_size() is OK")
	{
		CHECK(cache.max_size() == MAX_SIZE);
	}
}

TEST_CASE("Cache w/ LFU replacement policy: Size", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

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

		for(size_t i = 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.size() == MAX_SIZE);
			CHECK(cache.max_size() == MAX_SIZE);
		}
	}

	SECTION("clear() resets size to 0 (1/2)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() != 0);
		cache.clear();
		CHECK(cache.size() == 0);
	}

	SECTION("clear() resets size to 0 (2/2)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= 3 * MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() != 0);
		cache.clear();
		CHECK(cache.size() == 0);
	}

	SECTION("clear() invalidates all entries")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() != 0);
		REQUIRE(cache.entry_invalidation_count() == 0);
		REQUIRE(cache.cache_invalidation_count() == 0);

		cache.clear();

		CHECK(cache.entry_invalidation_count() == 0);
		CHECK(cache.cache_invalidation_count() == 1);
	}
}

TEST_CASE("Cache w/ LFU replacement policy: Hits & misses", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("Every new insertion counts as a miss")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.hit_count () == 0);
			CHECK(cache.miss_count() == i);
		}
	}

	SECTION("Every access to existing items counts as a hit")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == MAX_SIZE);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.exists(std::to_string(i));
			CHECK(cache.hit_count() == i);
			CHECK(cache.miss_count() == MAX_SIZE);
		}

		CHECK(cache.hit_count() == MAX_SIZE);
		CHECK(cache.miss_count() == MAX_SIZE);
	}

	SECTION("Every access to non-existing items counts as a miss (using exists)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.miss_count() == MAX_SIZE);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			cache.exists(std::to_string(i));
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

TEST_CASE("Cache w/ LFU replacement policy: find()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

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

		for(size_t i = MAX_SIZE + 1; i <= 10 * MAX_SIZE; i++)
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

TEST_CASE("Cache w/ LFU replacement policy: exists()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("exists() for an existing item returns true")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			bool exists = cache.exists(std::to_string(i));
			CHECK(exists == true);
		}
	}

	SECTION("exists() for a non-existing item returns false")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			bool exists = cache.exists(std::to_string(i));
			CHECK(exists == false);
		}
	}
}

TEST_CASE("Cache w/ LFU replacement policy: flush()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("flush() resets size to 0")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() != 0);
		cache.flush();
		CHECK(cache.size() == 0);
	}

	SECTION("flush() invalidates all entries")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.entry_invalidation_count() == 0);
		REQUIRE(cache.cache_invalidation_count() == 0);

		cache.flush();

		CHECK(cache.entry_invalidation_count() == 0);
		CHECK(cache.cache_invalidation_count() == 1);
	}

	SECTION("flush(key) reduces size by 1")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t old_size = cache.size();

		REQUIRE(cache.size() != 0);
		REQUIRE(cache.size() == old_size);
		cache.flush(std::to_string(((size_t)rand() % MAX_SIZE) + 1));
		CHECK(cache.size() == old_size - 1);
	}

	SECTION("flush(key) invalidates 1 object")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t old_inv = cache.entry_invalidation_count();

		REQUIRE(cache.entry_invalidation_count() == old_inv);
		cache.flush(std::to_string(((size_t)rand() % MAX_SIZE) + 1));
		CHECK(cache.entry_invalidation_count() == old_inv + 1);
	}

	SECTION("Flushed key is not cached")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss_count = cache.miss_count();

		REQUIRE(cache.miss_count() == miss_count);
		std::string key = std::to_string(((size_t)rand() % MAX_SIZE) + 1);
		cache.flush(key);
		bool exists = cache.exists(key);
		CHECK(exists == false);
		CHECK(cache.miss_count() == miss_count + 1);
	}

	SECTION("flush() of a non-existing key is a noop")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t size = cache.size();

		REQUIRE(cache.size() == size);
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = MAX_SIZE + 1; i <= 10 * MAX_SIZE; i++)
			cache.flush(std::to_string(i));

		CHECK(cache.size() == size);
		CHECK(cache.evicted_count() == 0);
	}

	SECTION("flush() of a non-existing key is a miss")
	{
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss = cache.miss_count();

		REQUIRE(cache.miss_count() == miss);
		cache.erase("asdf");
		CHECK(cache.miss_count() == miss + 1);
	}
}

TEST_CASE("Cache w/ LFU replacement policy: erase()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("erase(key) reduces size by 1")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t old_size = cache.size();

		REQUIRE(cache.size() != 0);
		REQUIRE(cache.size() == old_size);
		cache.erase(std::to_string(((size_t)rand() % MAX_SIZE) + 1));
		CHECK(cache.size() == old_size - 1);
	}

	SECTION("Erased key is not cached")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss_count = cache.miss_count();

		REQUIRE(cache.miss_count() == miss_count);
		std::string key = std::to_string(((size_t)rand() % MAX_SIZE) + 1);
		cache.erase(key);
		bool exists = cache.exists(key);
		CHECK(exists == false);
		CHECK(cache.miss_count() == miss_count + 1);
	}

	SECTION("erase() of a non-existing key is a noop")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t size = cache.size();

		REQUIRE(cache.size() == size);
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = MAX_SIZE + 1; i <= 10 * MAX_SIZE; i++)
			cache.erase(std::to_string(i));

		CHECK(cache.size() == size);
		CHECK(cache.evicted_count() == 0);
	}

	SECTION("erase() of a non-existing key is a miss")
	{
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss = cache.miss_count();

		REQUIRE(cache.miss_count() == miss);
		cache.erase("asdf");
		CHECK(cache.miss_count() == miss + 1);
	}
}

TEST_CASE("Cache w/ LFU replacement policy: at()", "[cache]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

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

TEST_CASE("Cache w/ LFU replacement policy: LFU behaviour", "[cache][lfu]")
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, Policy::LFU> cache(MAX_SIZE);

	SECTION("Replaced item is the least frequently used (1/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			REQUIRE(cache.exists(std::to_string(i)) == true);

		for(size_t i = 2; i <= MAX_SIZE; i++)
			REQUIRE(cache.exists(std::to_string(i)) == true);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.evicted_count() == 0);

		cache.insert("asdf", 42);
		CHECK(cache.exists("1") == false);
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
		CHECK(cache.exists("1") == false);
		CHECK(cache.evicted_count() == 1);
	}

	SECTION("Replaced item is the least frequently used (3/4)")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.evicted_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			REQUIRE(cache.exists(std::to_string(i)) == true);

		for(size_t i = 1; i < MAX_SIZE; i++)
			REQUIRE(cache.exists(std::to_string(i)) == true);

		cache.insert("asdf", 42);
		CHECK(cache.exists(std::to_string(MAX_SIZE)) == false);
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
		CHECK(cache.exists(std::to_string(MAX_SIZE)) == false);
		CHECK(cache.evicted_count() == 1);
	}
}
