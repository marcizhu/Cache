#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "Cache/Cache.h"
#include "Cache/Stats/None.h"
#include "Cache/detail/utility.h"
#include "Cache/detail/tuple_hash.h"

template<
	template<typename> class CachePolicy, // Cache replacement policy
	typename CacheLock = NullLock,        // Cache lock. Set to `std::mutex` if function is concurrent
	typename Function,                    // Original function type
	typename... Args                      // Variadic cache arguments. Forwarded to Cache constructor
>
auto wrap(Function fn, Args&&... args)
{
	return
		[fn, cache_args = std::make_tuple(args...)]
		(auto&&... arguments) mutable
		{
			using Arguments = std::tuple<std::decay_t<decltype(arguments)>...>;
			using ReturnType = decltype(fn(std::forward<decltype(arguments)>(arguments)...));
			using FunctionCache = Cache<Arguments, ReturnType, CachePolicy, CacheLock, Stats::None>;

			static_assert(!std::is_void<ReturnType>::value, "Return type of wrapped function must not be void");

			static auto cache = detail::construct_from_tuple<FunctionCache>(cache_args);

			auto key = std::make_tuple(arguments...);
			auto it = cache.find(key);

			if(it != cache.end())
				return it->second;

			auto value = fn(std::forward<decltype(arguments)>(arguments)...);
			cache[key] = value;
			return value;
		};
}
