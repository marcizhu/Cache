#include <functional>
#include <string>

#include "Cache/Cache.h"
#include "Cache/Policy/FIFO.h"
#include "Cache/Policy/LFU.h"
#include "Cache/Policy/LIFO.h"
#include "Cache/Policy/LRU.h"
#include "Cache/Policy/MRU.h"
#include "Cache/Policy/Random.h"

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
	wrapper<Policy::Random>

TEMPLATE_TEST_CASE("Cache API: std::erase_if()", "[cache][erase_if]", CACHE_REPLACEMENT_POLICIES)
{
	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply> cache(MAX_SIZE);

	cache["key 1"] = 4;
	cache["key 2"] = 5;

	using value_type = typename Cache<std::string, int, TestType::template apply>::value_type;

	SECTION("std::erase_if() deletes items if predicate is true (1/3)")
	{
		std::erase_if(cache, [](const value_type& p) { return p.second == 4; });
		CHECK(cache.size() == 1);
	}

	SECTION("std::erase_if() deletes items if predicate is true (2/3)")
	{
		std::erase_if(cache, [](const value_type& p) { return p.first == "key 1"; });
		CHECK(cache.size() == 1);
	}

	SECTION("std::erase_if() deletes items if predicate is true (3/3)")
	{
		std::erase_if(cache, [](const value_type&) { return true; });
		CHECK(cache.size() == 0);
	}

	SECTION("std::erase_if() does NOT erase items if predicate is false (1/2)")
	{
		std::erase_if(cache, [](const value_type& p) { return p.second == 7; });
		CHECK(cache.size() == 2);
	}

	SECTION("std::erase_if() does NOT erase items if predicate is false (2/2)")
	{
		std::erase_if(cache, [](const value_type&) { return false; });
		CHECK(cache.size() == 2);
	}
}

TEMPLATE_TEST_CASE("Cache API: Thread-safety", "[cache][thread]", CACHE_REPLACEMENT_POLICIES)
{
	struct TestLock
	{
		std::function<void()> lock_fn;
		std::function<void()> unlock_fn;
		std::function<bool()> try_lock_fn;

		void lock() { lock_fn(); }
		bool try_lock() { return try_lock_fn(); }
		void unlock() { unlock_fn(); }
	};

	int locks = 0;
	int unlocks = 0;

	TestLock test_lock;
	test_lock.lock_fn     = [&]() { if(locks >  unlocks) throw std::runtime_error("Deadlock!"); locks++; };
	test_lock.try_lock_fn = [&]() { if(locks == unlocks) return locks++, true; else return false; };
	test_lock.unlock_fn   = [&]() { unlocks++; };

	constexpr size_t MAX_SIZE = 128;
	Cache<std::string, int, TestType::template apply, TestLock> cache(MAX_SIZE, test_lock);

#define CHECK_THREAD_SAFETY_EX(fn, n1, n2) \
	CHECK(locks == n1);   \
	CHECK(unlocks == n1); \
	fn;                   \
	CHECK(locks == n2);   \
	CHECK(unlocks == n2);

#define CHECK_THREAD_SAFETY(fn) CHECK_THREAD_SAFETY_EX(fn, 1, 2)

	SECTION("Constructor is thread-safe")
	{
		CHECK(locks == 1);
		CHECK(unlocks == 1);
	}

	// Iterators
	SECTION("begin() is thread-safe")  { CHECK_THREAD_SAFETY(cache.begin ()); }
	SECTION("cbegin() is thread-safe") { CHECK_THREAD_SAFETY(cache.cbegin()); }
	SECTION("end() is thread-safe")    { CHECK_THREAD_SAFETY(cache.end ()  ); }
	SECTION("cend() is thread-safe")   { CHECK_THREAD_SAFETY(cache.cend()  ); }

	// Size getters
	SECTION("empty() is thread-safe") { CHECK_THREAD_SAFETY(cache.empty()); }
	SECTION("size() is thread-safe")  { CHECK_THREAD_SAFETY(cache.size() ); }

	// Lookup functions
	SECTION("at() is thread-safe")       { cache["key"] = 0; CHECK_THREAD_SAFETY_EX(cache.at    ("key"), 2, 3); }
	SECTION("lookup() is thread-safe")   { cache["key"] = 0; CHECK_THREAD_SAFETY_EX(cache.lookup("key"), 2, 3); }
	SECTION("operator[] is thread-safe") { CHECK_THREAD_SAFETY(cache["key"]); }
	SECTION("contains() is thread-safe") { CHECK_THREAD_SAFETY(cache.contains("key")); }
	SECTION("count() is thread-safe")    { CHECK_THREAD_SAFETY(cache.count("key")); }
	SECTION("find() is thread-safe")     { CHECK_THREAD_SAFETY(cache.find("key")); }

	// Erase functions
	SECTION("erase(it) is thread-safe")    { cache["key"] = 0; CHECK_THREAD_SAFETY_EX(cache.erase(cache.begin()             ), 2, 4); }
	SECTION("erase(range) is thread-safe") { cache["key"] = 0; CHECK_THREAD_SAFETY_EX(cache.erase(cache.begin(), cache.end()), 2, 5); }
	SECTION("erase(key) is thread-safe")   { cache["key"] = 0; CHECK_THREAD_SAFETY_EX(cache.erase("key"                     ), 2, 3); }

	// Insertion functions
	SECTION("emplace() is thread-safe")     { CHECK_THREAD_SAFETY(cache.emplace("test", 5)); }
	SECTION("insert() is thread-safe")      { CHECK_THREAD_SAFETY(cache.insert ("test", 5)); }
	SECTION("insert(range) is thread-safe") { CHECK_THREAD_SAFETY_EX(cache.insert({ { "a", 1 }, { "b", 2}, { "c", 3 } }), 1, 1+3); }
	SECTION("insert(pair) is thread-safe")  { CHECK_THREAD_SAFETY(cache.insert(std::make_pair("key", 9))); }

	// Clear functions
	SECTION("clear() is thread-safe") { CHECK_THREAD_SAFETY(cache.clear()); }
	SECTION("flush() is thread-safe") { CHECK_THREAD_SAFETY(cache.flush()); }
}

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

TEMPLATE_TEST_CASE("Cache API: clear()", "[cache][clear]", CACHE_REPLACEMENT_POLICIES)
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

TEMPLATE_TEST_CASE("Cache API: emplace()", "[cache][emplace]", CACHE_REPLACEMENT_POLICIES)
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

	SECTION("operator[] returns the value if the key exists (1/3)")
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

	SECTION("operator[] returns the value if the key exists (2/3)")
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

	SECTION("operator[] returns the value if the key exists (3/3)")
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
