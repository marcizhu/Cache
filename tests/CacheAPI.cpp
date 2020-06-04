#include <string>

#include "Cache/Cache.h"
#include "Cache/Policy/FIFO.h"
#include "Cache/Policy/LFU.h"
#include "Cache/Policy/LIFO.h"
#include "Cache/Policy/LRU.h"
#include "Cache/Policy/MRU.h"
#include "Cache/Policy/None.h"
#include "Cache/Policy/Random.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

template<template<typename> class Template>
struct wrapper
{
	template<typename Args>
	using apply = Template<Args>;
};

#define CACHE_REPLACEMENT_POLICIES \
	wrapper<Policy::FIFO>, \
	wrapper<Policy::LFU >, \
	wrapper<Policy::LIFO>, \
	wrapper<Policy::LRU >, \
	wrapper<Policy::MRU >, \
	wrapper<Policy::None>, \
	wrapper<Policy::Random>

TEMPLATE_TEST_CASE("Cache API: Initial conditionns", "[cache][init]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

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
		CHECK(cache.utilization() == 0.0f);

		CHECK(std::isnan(cache.hit_ratio ()));
		CHECK(std::isnan(cache.miss_ratio()));
	}

	SECTION("max_size() is OK")
	{
		CHECK(cache.max_size() == MAX_SIZE);
	}
}

TEMPLATE_TEST_CASE("Cache API: insert()", "[cache][insert]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	SECTION("Size grows after each insert()")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.size() == i);
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

	SECTION("Cache evicts items if size() == max_size()")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.size() == cache.max_size());
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = MAX_SIZE + 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.evicted_count() == i - MAX_SIZE);
		}
	}
}

TEMPLATE_TEST_CASE("Cache API: Size", "[cache][clear]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

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

TEMPLATE_TEST_CASE("Cache API: Hits & misses", "[cache][stats][hit][miss]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	SECTION("Every insert() is not a miss nor a hit")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.insert(std::to_string(i), (int)i);
			CHECK(cache.hit_count () == 0);
			CHECK(cache.miss_count() == 0);
		}
	}

	SECTION("Every emplace() is not a miss nor a hit")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.emplace(std::to_string(i), (int)i);
			CHECK(cache.hit_count () == 0);
			CHECK(cache.miss_count() == 0);
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
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.contains(std::to_string(i));
			CHECK(cache.hit_count() == i);
			CHECK(cache.miss_count() == 0);
		}

		CHECK(cache.hit_count() == MAX_SIZE);
		CHECK(cache.miss_count() == 0);
	}

	SECTION("Every access to non-existing items counts as a miss (using contains)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			cache.contains(std::to_string(i));
			CHECK(cache.hit_count() == 0);
			CHECK(cache.miss_count() == i - MAX_SIZE);
		}

		CHECK(cache.hit_count() == 0);
		CHECK(cache.miss_count() == MAX_SIZE);
	}

	SECTION("Every access to non-existing items counts as a miss (using find)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		REQUIRE(cache.hit_count () == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			cache.find(std::to_string(i));
			CHECK(cache.hit_count() == 0);
			CHECK(cache.miss_count() == i - MAX_SIZE);
		}

		CHECK(cache.hit_count() == 0);
		CHECK(cache.miss_count() == MAX_SIZE);
	}
}

TEMPLATE_TEST_CASE("Cache API: In-place construction", "[cache][emplace]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	class TestClass
	{
	public:
		TestClass(char) { Constructions++; }
		TestClass(const TestClass& other)
		{
			Constructions     += other.Constructions;
			CopyConstructions += other.CopyConstructions + 1;
			CopyAssignment    += other.CopyAssignment;
			MoveConstruction  += other.MoveConstruction;
			MoveAssignment    += other.MoveAssignment;
		}

		TestClass& operator=(const TestClass& other)
		{
			Constructions     += other.Constructions;
			CopyConstructions += other.CopyConstructions;
			CopyAssignment    += other.CopyAssignment + 1;
			MoveConstruction  += other.MoveConstruction;
			MoveAssignment    += other.MoveAssignment;

			return *this;
		}

		TestClass(TestClass&& other)
		{
			Constructions     += other.Constructions;
			CopyConstructions += other.CopyConstructions;
			CopyAssignment    += other.CopyAssignment;
			MoveConstruction  += other.MoveConstruction + 1;
			MoveAssignment    += other.MoveAssignment;
		}

		TestClass& operator=(TestClass&& other)
		{
			Constructions     += other.Constructions;
			CopyConstructions += other.CopyConstructions;
			CopyAssignment    += other.CopyAssignment;
			MoveConstruction  += other.MoveConstruction;
			MoveAssignment    += other.MoveAssignment + 1;

			return *this;
		}

		int Constructions{};
		int CopyConstructions{};
		int CopyAssignment{};
		int MoveConstruction{};
		int MoveAssignment{};
	};

	SECTION("emplace() constructs item in-place")
	{
		Cache<std::string, TestClass, TestType::template apply> emplace_cache(10);
		emplace_cache.emplace("key", 'v');

		CHECK(emplace_cache.at("key").Constructions     == 1);
		CHECK(emplace_cache.at("key").CopyConstructions == 0);
		CHECK(emplace_cache.at("key").CopyAssignment    == 0);
		CHECK(emplace_cache.at("key").MoveConstruction  == 0);
		CHECK(emplace_cache.at("key").MoveAssignment    == 0);
	}

	SECTION("Size grows after each emplace()")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache.emplace(std::to_string(i), (int)i);
			CHECK(cache.size() == i);
		}
	}

	SECTION("Size stops growing after size() == max_size()")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.emplace(std::to_string(i), (int)i);

		REQUIRE(cache.size() == MAX_SIZE);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.emplace(std::to_string(i), (int)i);
			CHECK(cache.size() == MAX_SIZE);
			CHECK(cache.max_size() == MAX_SIZE);
		}
	}

	SECTION("Cache evicts items if size() == max_size()")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.emplace(std::to_string(i), (int)i);

		REQUIRE(cache.size() == MAX_SIZE);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = MAX_SIZE + 1; i <= 10 * MAX_SIZE; i++)
		{
			cache.emplace(std::to_string(i), (int)i);
			CHECK(cache.evicted_count() == i - MAX_SIZE);
		}
	}

	SECTION("emplace() of an existing item does not change the size")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.emplace(std::to_string(i), (int)i);

		REQUIRE(cache.size() == MAX_SIZE);
		REQUIRE(cache.max_size() == MAX_SIZE);

		cache.emplace("1", 5);

		CHECK(cache.size() == MAX_SIZE);
		CHECK(cache.max_size() == MAX_SIZE);
	}
}

TEMPLATE_TEST_CASE("Cache API: find()", "[cache][find]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

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

TEMPLATE_TEST_CASE("Cache API: contains()", "[cache][contains]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	SECTION("contains() for an existing item returns true")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			bool contains = cache.contains(std::to_string(i));
			CHECK(contains == true);
		}
	}

	SECTION("contains() for a non-existing item returns false")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			bool contains = cache.contains(std::to_string(i));
			CHECK(contains == false);
		}
	}
}

TEMPLATE_TEST_CASE("Cache API: count()", "[cache][count]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	SECTION("count() for an existing item returns 1")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			size_t count = cache.count(std::to_string(i));
			CHECK(count == 1);
		}
	}

	SECTION("count() for a non-existing item returns 0")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
		{
			size_t count = cache.count(std::to_string(i));
			CHECK(count == 0);
		}
	}
}

TEMPLATE_TEST_CASE("Cache API: flush()", "[cache][flush]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

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
		bool contains = cache.contains(key);
		CHECK(contains == false);
		CHECK(cache.miss_count() == miss_count + 1);
	}

	SECTION("flush() of a non-existing key is a noop")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.evicted_count() == 0);
		REQUIRE(cache.hit_count() == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t size = cache.size();

		REQUIRE(cache.size() == size);
		REQUIRE(cache.evicted_count() == 0);
		REQUIRE(cache.hit_count() == 0);
		REQUIRE(cache.miss_count() == 0);

		for(size_t i = MAX_SIZE + 1; i <= 10 * MAX_SIZE; i++)
			cache.flush(std::to_string(i));

		CHECK(cache.size() == size);
		CHECK(cache.evicted_count() == 0);
		REQUIRE(cache.hit_count() == 0);
		REQUIRE(cache.miss_count() == 10 * MAX_SIZE - MAX_SIZE);
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

TEMPLATE_TEST_CASE("Cache API: erase()", "[cache][erase]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

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

	SECTION("Erased key is not cached (1/2)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss_count = cache.miss_count();

		REQUIRE(cache.miss_count() == miss_count);
		std::string key = std::to_string(((size_t)rand() % MAX_SIZE) + 1);
		cache.erase(key);
		bool contains = cache.contains(key);
		CHECK(contains == false);
		CHECK(cache.miss_count() == miss_count + 1);
	}

	SECTION("Erased key is not cached (2/2)")
	{
		REQUIRE(cache.size() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t miss_count = cache.miss_count();

		REQUIRE(cache.miss_count() == miss_count);
		std::string key = std::to_string(((size_t)rand() % MAX_SIZE) + 1);
		auto it = cache.find(key);
		cache.erase(it);
		bool contains = cache.contains(key);
		CHECK(contains == false);
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

	SECTION("erase() of an existing key is a hit")
	{
		REQUIRE(cache.hit_count() == 0);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		size_t hit = cache.hit_count();

		REQUIRE(cache.hit_count() == hit);
		cache.erase("1");
		CHECK(cache.hit_count() == hit + 1);
	}

	SECTION("erase(begin(), end()) clears the cache")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		cache.erase(cache.begin(), cache.end());

		CHECK(cache.max_size() == MAX_SIZE);
		CHECK(cache.size() == 0);

		size_t item_count = 0;

		for(auto it = cache.begin(); it != cache.end(); it++)
			item_count++;

		CHECK(item_count == 0);
	}
}

TEMPLATE_TEST_CASE("Cache API: at()", "[cache][at]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

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

TEMPLATE_TEST_CASE("Cache API: lookup()", "[cache][lookup]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	SECTION("lookup() return value for existing keys")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			auto item = cache.lookup(std::to_string(i));
			CHECK(item == (int)i);
		}
	}

	SECTION("lookup() throws for non-exisiting keys")
	{
		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache.insert(std::to_string(i), (int)i);

		for(size_t i = MAX_SIZE + 1; i <= 2 * MAX_SIZE; i++)
			CHECK_THROWS(cache.lookup(std::to_string(i)));
	}
}

TEMPLATE_TEST_CASE("Cache API: operator[]", "[cache][operator]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	SECTION("operator[] inserts items if the key is not present")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache[std::to_string(i)] = (int)i;
			CHECK(cache.size() == i);
			CHECK(cache.at(std::to_string(i)) == (int)i);
		}

		CHECK(cache.size() == MAX_SIZE);
		CHECK(cache.max_size() == MAX_SIZE);
	}

	SECTION("operator[] default constructs a new item")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= MAX_SIZE; i++)
		{
			cache[std::to_string(i)];
			CHECK(cache[std::to_string(i)] == int());
		}

		CHECK(cache.size() == MAX_SIZE);
		CHECK(cache.max_size() == MAX_SIZE);
	}

	SECTION("operator[] returns the value if the key contains (1/3)")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			cache[std::to_string(i)] = (int)i;

		REQUIRE(cache.size() == MAX_SIZE);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= MAX_SIZE; i++)
			CHECK(cache[std::to_string(i)] == (int)i);
	}

	SECTION("operator[] returns the value if the key contains (2/3)")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= 2 * MAX_SIZE; i++)
			cache[std::to_string(i)] = (int)i;

		CHECK(cache.size() == MAX_SIZE);
		CHECK(cache.max_size() == MAX_SIZE);

		for(auto it = cache.begin(); it != cache.end(); it++)
			CHECK(cache[std::to_string(it->second)] == (int)it->second);
	}

	SECTION("operator[] returns the value if the key contains (3/3)")
	{
		REQUIRE(cache.size() == 0);
		REQUIRE(cache.max_size() == MAX_SIZE);

		for(size_t i = 1; i <= 10 * MAX_SIZE; i++)
		{
			cache[std::to_string(i)] = (int)i;
			CHECK(cache[std::to_string(i)] == (int)i);
		}

		CHECK(cache.size() == MAX_SIZE);
		CHECK(cache.max_size() == MAX_SIZE);
	}
}
