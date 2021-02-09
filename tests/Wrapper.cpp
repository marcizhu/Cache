#include "Cache/Cache.h"
#include "Cache/Wrapper.h"
#include "Cache/Policy/LRU.h"

#include "catch2/catch.hpp"

TEST_CASE("Function Wrapper", "[cache][wrapper]")
{
	int call_count = 0;
	auto my_fn = [&call_count](int a, int b) { call_count++; return a + b; };

	auto my_fn_cached = wrap<Policy::LRU>(my_fn, 10U);

	SECTION("The original function is called if the data is not cached")
	{
		REQUIRE(call_count == 0);
		int ret = my_fn_cached(1, 2);

		REQUIRE(ret == 3);
		CHECK(call_count == 1);
	}

	SECTION("The original function is NOT called if the data is cached")
	{
		REQUIRE(my_fn_cached(2, 2) == 4);
		REQUIRE(call_count == 1);
		int ret = my_fn_cached(2, 2);

		REQUIRE(ret == 4);
		CHECK(call_count == 1);
	}
}
