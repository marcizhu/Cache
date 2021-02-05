/*
 *  This file is part of the Cache library (https://github.com/marcizhu/Cache)
 *
 *  Copyright (C) 2020-2021 Marc Izquierdo
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

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
