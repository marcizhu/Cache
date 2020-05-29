#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "Cache/Cache.h"
#include "Cache/Stats/None.h"

namespace detail
{
	template<
		typename Key,
		typename Value,
		template<typename> class CachePolicy,
		typename Lock,
		typename... Args,
		std::size_t... Indices
	>
	inline auto make_cache(const std::tuple<Args...>& args, std::index_sequence<Indices...>)
	{
		return Cache<Key, Value, CachePolicy, Lock, Stats::None>(std::forward<Args>(std::get<Indices>(args))...);
	}

	template<
		typename Key,
		typename Value,
		template<typename> class CachePolicy,
		typename Lock,
		typename... Args
	>
	inline auto make_cache(const std::tuple<Args...>& args)
	{
		return make_cache<Key, Value, CachePolicy, Lock, Args...>(args, std::make_index_sequence<sizeof...(Args)>());
	}
}

template<
	template<typename> class CachePolicy, // Cache replacement policy
	typename CacheLock = NullLock,        // Cache lock. Set to `std::mutex` if function is concurrent
	typename Function,                    // Original function type
	typename... Args                      // Variadic cache arguments. Forwarded to Cache constructor
>
auto wrap(Function fn, Args&&... args)
{
	return
		[fn, cache_args = std::forward_as_tuple(std::forward<Args>(args)...)]
		(auto&&... arguments) mutable
		{
			using Arguments = std::tuple<std::decay_t<decltype(arguments)>...>;
			using ReturnType = decltype(fn(std::forward<decltype(arguments)>(arguments)...));

			static_assert(!std::is_void<ReturnType>::value, "Return type of wrapped function must not be void");

			static auto cache = detail::make_cache<Arguments, ReturnType, CachePolicy, CacheLock>(cache_args);

			auto key = std::make_tuple(arguments...);
			auto it = cache.find(key);

			if(it != cache.end())
				return it->second;

			auto value = fn(std::forward<decltype(arguments)>(arguments)...);
			cache.insert(key, value);
			return value;
		};
}

namespace std
{
	template<typename... Ts>
	struct hash<std::tuple<Ts...>>
	{
		using argument_type = std::tuple<Ts...>;
		using result_type = std::size_t;

		result_type operator()(const argument_type& argument) const { return hash_tuple(argument, std::make_index_sequence<sizeof...(Ts)>()); }

	private:
		template<std::size_t I, std::size_t... Is>
		result_type hash_tuple(const argument_type& tuple, std::index_sequence<I, Is...>) const
		{
			auto value = std::get<I>(tuple);
			auto current = std::hash<decltype(value)>{}(value);
			auto seed = hash_tuple(tuple, std::index_sequence<Is...>());

			// http://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html
			return current + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		result_type hash_tuple(const argument_type&, std::index_sequence<>) const { return 0; }
	};
}
