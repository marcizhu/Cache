# Cache
[![Build Status](https://travis-ci.com/marcizhu/Cache.svg?branch=master)](https://travis-ci.com/marcizhu/Cache)
[![License](https://img.shields.io/github/license/marcizhu/Cache)](https://github.com/marcizhu/Cache/blob/master/LICENSE)
[![Stability: Experimental](https://masterminds.github.io/stability/experimental.svg)](https://masterminds.github.io/stability/experimental.html)

A small, lightweight, thread-safe, easy to use, header-only, fast and simple cache with selectable replacement algorithms!

This is a small library aimed to provide a simple and intuitive cache with selectable replacement policies. The cache has the same API as a `std::map`/`std::unordered_map` (with some nice additions :D)
Any kind of input/feedback is always welcome!

### Features
- Small, lightweight, fast cross-platform library.
- The library is written in C++14, and is C++14/17/20 compatible.
- Selectable cache replacement algorithms at compile time
- Caches provide statistics about hits, misses, entry invalidations, cache invalidations, and entry evictions
- 100% Thread-safe for concurrent applications where different threads will have to access the same cache
- Ability to use custom replacement algorithms
- Ability to use custom statistic measurements (for example to track some keys only, or to log information to log files)
- Ability to have custom callbacks on hit/miss/clear/erase/evicted entry.
- 100% STL-compatible API
- Ability to 'wrap' functions in a cache to speed up expensive function calls.

## Table of contents
- [Background](#background)
- [Install](#install)
- [Usage](#usage)
- [Limitations](#limitations)
- [Benchmarks](#benchmarks)
- [Alternatives](#alternatives)
- [Maintainer](#maintainer)
- [Credits](#credits)
- [Contributing](#contributing)
- [License](#license)

## Background
TBD

## Install
The provided `CMakeLists.txt` file exports a `Cache` library, so if your build system is based on CMake, you can use
that. If not, copy the `include/Cache` folder to your `include`/`vendor`/`deps` folder and you're ready to go!

The CMake file also provides two options:
- `CACHE_BUILD_TESTS` (default: `OFF`): Builds tests. This will require the [Catch2]() library, which is already included in `deps/Catch2`
- `CACHE_BUILD_EXAMPLES` (default: `OFF`): Builds the provided examples. This is a great way of learning how to use this library and start experimenting right away!

## Usage
This header only library provides the following files:
- `Cache/Cache.h`: Where the main `Cache` class is. This is the file you have to include to create caches.
- `Cache/Wrapper.h`: Contains the function `wrap()`, which wraps a function in a cache. More on that later.

### Creating a cache
To create a cache, just `#include "Cache/Cache.h"`, include your desired replacement algorithm (`Policy::LRU` is a good one to
start with), and you're ready to go! Just do something like:
```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h"

Cache<std::string, int, Policy::LRU> cache(100);
```

The above fragment would create a cache called `cache`, with 100 entries of string-int pairs, and will use the LRU
(Least Recently Used) replacement algorithm.

A more advanced example would be to use this cache to store previous recursive calls like so:
```cpp
#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// Optimized, dynamic-programming style fibonacci function using a cache
template<typename Cache>
std::uint64_t cached_fibonacci(std::uint64_t n, Cache& cache)
{
	if(n < 2) return n;

	// If the cache contains the pair (n, result), return the result directly
	if(cache.contains(n)) return cache[n];

	// If not, calculate it using recursive calls and store it in cache
	auto value = cached_fibonacci(n - 1, cache) + cached_fibonacci(n - 2, cache);
	cache.insert(n, value);

	// Finally, return the calculated value
	return value;
}

std::uint64_t cached_fibonacci(std::uint64_t n)
{
	// We create a cache with 100 entries and LRU replacement policy
	static Cache<std::uint64_t, std::uint64_t, Policy::LRU> cache(100);
	return cached_fibonacci(n, cache);
}

int main()
{
	std::cout << "cached_fibonacci( 0) = " << cached_fibonacci( 0) << std::endl; // should be 0
	std::cout << "cached_fibonacci( 2) = " << cached_fibonacci( 2) << std::endl; // should be 1
	std::cout << "cached_fibonacci(40) = " << cached_fibonacci(40) << std::endl; // should be 102334155
	std::cout << "cached_fibonacci(45) = " << cached_fibonacci(45) << std::endl; // should be 1134903170
	std::cout << "cached_fibonacci(50) = " << cached_fibonacci(50) << std::endl; // should be 12586269025
	std::cout << "cached_fibonacci(90) = " << cached_fibonacci(90) << std::endl; // should be 2880067194370816120
}
```
For more information and an in-depth explaination on how it works, please see [examples/dynamic_programming.cpp](https://github.com/marcizhu/Cache/blob/master/examples/dynamic_programming.cpp)

The class `Cache` is 100% compatible with a `std::map`/`std::unordered_map`, and can even have an unlimited size (just pass 0
to the constructor and the size will be unlimited), but it also has some useful "extra" functions, like:
- `bool contains(key)`: Returns true if the cache contains the provided key. `false` otherwise
- `value& lookup(key)`: Alias for `at(key)`
- `void flush(key)`: Alias for `erase(key)`
- `void flush()`: Alias for `clear()`
- `size_t hit_count()`/`miss_count()`/`access_count()`/`entry_invalidation_count()`/`cache_invalidation_count()`/`evicted_count()`: Returns statistics about hits, misses, accesses, etc...
- `float hit_ratio()`/`float miss_ratio()`/`float utilization()`: Returns statistics about the hit/miss ratio and utilization (that is, size / capacity)

### Thread-safe cache
Any cache, by default is **NOT** thread-safe. This is done on purpose to make single-threaded caches work faster. But don't
worry, making a thread-safe cache is as simple as adding a `std::mutex` parameter to the end of the template parameter list:
```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h"

Cache<std::string, int, Policy::LRU, std::mutex> cache(100);
```
Of course, any other mutex-like class can be passed to this class. So if you're not using the STL or you want to roll your own
mutex, just replace `std::mutex` with the type of that object and it will just work! Isn't that cool?

To make a cache explicitly not thread-safe, replace `std::mutex` in the previous snippet with `NullLock` (defined in Cache/Cache.h)

For more examples on multithreading, please see [examples/multithread_cache.cpp](https://github.com/marcizhu/Cache/blob/master/examples/multithread_cache.cpp)
and [examples/multithread_function_wrapping.cpp](https://github.com/marcizhu/Cache/blob/master/examples/multithread_function_wrapping.cpp)

### Function wrapping
This is a quick way of adding a small cache to existing codebases. However, take into account that this cache will only store
information about input-output pairs, and **NOT** about recursive calls. Thus, this is more like a "shallow" cache (but useful
nonetheless!). The following snippet will show how to wrap any non-void function with a cache:
```cpp
#include "Cache/Cache.h"      // class Cache
#include "Cache/Wrapper.h"    // function wrap(...)
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

std::size_t ackermann(std::size_t m, std::size_t n)
{
	// we add a small delay to make this function much slower
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(50ms);

	if(m == 0) return n + 1;
	if(n == 0) return ackermann(m - 1, 1);

	return ackermann(m - 1, ackermann(m, n - 1));
}

// Wrap 'ackermann' in a 16-entry cache with LRU replacement policy
auto cached_ackermann = wrap<Policy::LRU>(ackermann, 16);

// after this, just call your function like usual:
cached_ackermann(2, 5);
```
Feels like magic, huh?

Some examples on this topic are [examples/function_wrapping.cpp](https://github.com/marcizhu/Cache/blob/master/examples/function_wrapping.cpp)
and [examples/multithread_function_wrapping.cpp](https://github.com/marcizhu/Cache/blob/master/examples/multithread_function_wrapping.cpp)
for single-threaded and thread-safe function wrappers (respectively).

### Statistics
Statistics are an interesting topic. Any cache by default registers the number of hits, number of misses, number of entry
invalidations (aka number of `erase()`'d keys), number of cache invalidations (aka the number of `clear()` calls) and the
number of evicted entries (that is, the number of entries deleted to make room for newer entries). However, all of this
measurements can be disabled for high-performance applications using an extra template parameter:
```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h" // LRU replacement policy
#include "Cache/Stats/None.h" // for Stats::None (disabled/null statistics)

Cache<std::string, int, Policy::LRU, NullLock, Stats::None> cache(100);
```
Please note that the parameter `NullLock` refers to the lock type and could be `std::mutex` if we wanted the cache to be
thread-safe. Both parameters are totally independent.

That last template parameter can also be used to provide our own statistics, to have callbacks on hit/miss/evict/clear/erase
events and much more! Please check
[examples/statistics.cpp](https://github.com/marcizhu/Cache/blob/master/examples/statistics.cpp), 
[examples/disable_statistics.cpp](https://github.com/marcizhu/Cache/blob/master/examples/disable_statistics.cpp), 
[examples/custom_statistics.cpp](https://github.com/marcizhu/Cache/blob/master/examples/custom_statistics.cpp), 
[examples/custom_callbacks.cpp](https://github.com/marcizhu/Cache/blob/master/examples/custom_callbacks.cpp) and 
[examples/advanced_custom_callbacks.cpp](https://github.com/marcizhu/Cache/blob/master/examples/advanced_custom_callbacks.cpp)
to learn more about reading statistics from caches, how to disable statistics for a cache, how to roll your own custom
statistics, implementig (simple) callbacks and implementing more advanced callbacks using statistics.

### Replacement algorithms
When caches become full and we want to insert a new entry, what do we do? We evict one entry. But... which one? That's the task
of the replacement algorithm (or replacement policy).

This library provides the following replacement policies:
- `Policy::FIFO`: (defined inside [`Cache/Policy/FIFO.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/FIFO.h)). Works like a queue: First In, First Out.
- `Policy::LFU`: (defined inside [`Cache/Policy/LFU.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LFU.h)). Replaces the Least Frequently Used entry.
- `Policy::LIFO`: (defined inside [`Cache/Policy/LIFO.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LIFO.h)). Works like a stack: Last In, First Out.
- `Policy::LRU`: (defined inside [`Cache/Policy/LRU.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LRU.h)). Replaces the Least Recently Used entry.
- `Policy::MRU`: (defined inside [`Cache/Policy/MRU.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/MRU.h)). Replaces the Most Recently Used entry.
- `Policy::None`: (defined inside [`Cache/Policy/None.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/None.h)). Replaces the smallest key (lexicographically).
- `Policy::Random`: (defined inside [`Cache/Policy/Random.h`](https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/Random.h)). Replaces a random key.

Depending on your application, you might choose different algorithms for your caches, but if in doubt, use either `Policy::LRU`
or `Policy::LFU`.

And, of course, you can also make your own custom algorithms and pass them to the cache! Check out
[examples/custom_replacement_policy.cpp](https://github.com/marcizhu/Cache/blob/master/examples/custom_replacement_policy.cpp)
for an in-depth example on writing your own algorithms and creating caches that use them.

## Benchmarks
Comming soon

## Alternatives
This library aims to be as much feature-complete as possible, and to cover most use cases. However, we know that sometimes you
need something more specific. For those cases, you might check out one of the following:
- [goldsborough/lru-cache](https://github.com/goldsborough/lru-cache): 💫 A feature complete LRU cache implementation in C++
- [vpetrigo/caches](https://github.com/vpetrigo/caches): LRU, LFU, FIFO cache C++ implementations

## Maintainer
This library was created and is currently maintained by [marcizhu](https://github.com/marcizhu).
If you find any issue with the library, you can either [open an issue](https://github.com/marcizhu/Units/issues) or e-mail me at marcizhu@gmail.com.

## Contributing
Does this library miss any feature you'd like to have? Have you spotted any bug? PRs are always welcome! Alternatively, you
can open an issue [here](https://github.com/marcizhu/Units/issues) or e-mail me at marcizhu@gmail.com and I'll try to
respond and/or fix it as soon as possible :D

## License
Copyright (c) 2020 Marc Izquierdo  
This library is licensed under the [MIT License](https://choosealicense.com/licenses/mit/). See
[LICENSE](https://github.com/marcizhu/Units/blob/master/LICENSE) for more details.
