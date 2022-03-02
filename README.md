# Cache
![Build & Test](https://github.com/marcizhu/Cache/workflows/Build%20&%20Test/badge.svg)
[![License](https://img.shields.io/github/license/marcizhu/Cache)](https://github.com/marcizhu/Cache/blob/master/LICENSE)
[![Stability: Sustained](https://masterminds.github.io/stability/sustained.svg)](https://masterminds.github.io/stability/sustained.html)
[![codecov](https://codecov.io/gh/marcizhu/Cache/branch/master/graph/badge.svg)](https://codecov.io/gh/marcizhu/Cache)

A small, lightweight, thread-safe, easy to use, header-only, fast and simple cache with selectable replacement algorithms!

This is a small library aimed to provide a simple and intuitive cache with selectable replacement policies. The cache has the
same API as a `std::map`/`std::unordered_map` (with some nice extra features :D).

### Features
- Small, lightweight, fast cross-platform library.
- Written in C++14, and is C++14/17/20 compatible.
- Selectable cache replacement algorithms at compile time.
- Caches provide statistics about hits, misses, entry invalidations, cache invalidations, and entry evictions.
- 100% Thread-safe for concurrent applications where different threads will have to access the same cache.
- Ability to use custom replacement algorithms.
- Ability to use custom statistic measurements (for example to track some keys only, or to log information to log files).
- Ability to have custom callbacks on hit/miss/clear/erase/evicted entry.
- 100% STL-compatible API.
- Ability to 'wrap' functions in a cache to speed up expensive function calls.
- Heavily tested: over 90 test cases, more than 110.000 assertions and >99% code coverage

## Table of contents
- [Background](#background)
- [Install](#install)
- [Usage](#usage)
  - [Creating a cache](#creating-a-cache)
  - [Thread-safe cache](#thread-safe-cache)
  - [Function wrapping](#function-wrapping)
  - [Statistics](#statistics)
  - [Callbacks](#callbacks)
  - [Replacement policies](#replacement-policies)
  - [Other examples](#other-examples)
- [Benchmarks](#benchmarks)
- [Alternatives](#alternatives)
- [Maintainer](#maintainer)
- [Credits](#credits)
- [Contributing](#contributing)
- [License](#license)

## Background
When access to resources is limited or expensive, programmers need a way to optimize access to said resources. One way of 
doing that is to store all data in memory, but more often than not this solution is impractical. A more optimal approach would
be to store a small subset of the most used (or most expensive to access) data in a small data structure.

That is the intent of this library: to provide a small and simple data structure with a fixed size (but it can be left 
unbounded too) for caching function call parameters, recursive calls or any type of resources, like game assets, files or
other types of resources that might be too frequently accessed and take too much time to read or write to.

The cache replacement algorithm can be chosen at compile time (you can even write your own and use it) so that your 
application has the best performance possible in any scenario. And speaking of performance, by default the cache tracks some
statistics (number of hits, number of misses, number of entries erased by the user, number of cache clears and number
of entries evicted). Those statistics can be disabled at compile time for better performance or even make your own custom 
stats to track specific keys, implement callbacks and much more!

## Install
The provided `CMakeLists.txt` file exports a `Cache` library, so if your build system is based on CMake, you can use
that. If that is not the case, copy the `include/Cache` folder to your `include`/`vendor`/`deps` folder and you're ready to 
go!

The CMake file also provides two options:
- `CACHE_BUILD_TESTS` (default: `OFF`): Builds tests. This will require the [Catch2]
library, which will automatically be downloaded if needed.
- `CACHE_BUILD_EXAMPLES` (default: `OFF`): Builds the provided examples. 

## Usage
This header-only library provides the following files:
- `Cache/Cache.h`: Where the main `Cache` class is. This is the file you have to include to create caches.
- `Cache/Wrapper.h`: Contains the function `wrap()`, which wraps a function in a cache.
- `Cache/Policy/*.h`: Contains multiple replacement policies for caches. See [Replacement policies](#replacement-policies) for more.

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
	std::cout << "fib( 0) = " << cached_fibonacci( 0) << std::endl; // prints 0
	std::cout << "fib( 2) = " << cached_fibonacci( 2) << std::endl; // prints 1
	std::cout << "fib(40) = " << cached_fibonacci(40) << std::endl; // prints 102334155
	std::cout << "fib(45) = " << cached_fibonacci(45) << std::endl; // prints 1134903170
	std::cout << "fib(50) = " << cached_fibonacci(50) << std::endl; // prints 12586269025
	std::cout << "fib(90) = " << cached_fibonacci(90) << std::endl; // prints 2880067194370816120
}
```
For more information and an in-depth explanation on how it works, please see [examples/dynamic_programming.cpp]

The class `Cache` is 100% compatible with a `std::map`/`std::unordered_map`, and can even have an unlimited size (just pass 0
to the constructor and the size will be unlimited), but it also has some useful "extra" functions, like:
- `bool contains(key)`: Returns `true` if the cache contains the provided key, `false` otherwise
- `value& lookup(key)`: Alias for `at(key)`
- `void flush(key)`: Alias for `erase(key)`
- `void flush()`: Alias for `clear()`
- `size_t hit_count()`/`miss_count()`/`access_count()`/`entry_invalidation_count()`/`cache_invalidation_count()`/`evicted_count()`: Returns statistics about hits, misses, accesses, etc...
- `float hit_ratio()`/`float miss_ratio()`/`float utilization()`: Returns statistics about the hit/miss ratio and utilization (that is, size / capacity)

### Thread-safe cache
Caches are **NOT** thread-safe by default. This is done to prevent the continuous atomic locking and unlocking of mutex 
objects in single-threaded scenarios, which would reduce performance. If you need to use a thread safe cache, just pass 
`std::mutex` (or any other mutex-like type) as the fourth template parameter of the `Cache` class:

```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h"

Cache<std::string, int, Policy::LRU, std::mutex> cache(100);
```
If the fourth template parameter is not provided, it defaults to `NullLock`, which is a small dummy mutex class defined in
Cache/Cache.h that provides the `std::mutex` interface but does nothing.

For more examples on multithreading, please see [examples/multithread_cache.cpp] and  [examples/multithread_function_wrapping.cpp]

### Function wrapping
This library provides the utility template function `wrap()` that takes in a function and returns a new function that automatically
caches input-output pairs of data.

```cpp
#include "Cache/Cache.h"      // class Cache
#include "Cache/Wrapper.h"    // function wrap(...)
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// An example of a slow function: the Ackermann function
std::size_t ackermann(std::size_t m, std::size_t n)
{
	if(m == 0) return n + 1;
	if(n == 0) return ackermann(m - 1, 1);

	return ackermann(m - 1, ackermann(m, n - 1));
}

// Wrap 'ackermann' in a 16-entry cache with LRU replacement policy
auto cached_ackermann = wrap<Policy::LRU>(ackermann, 16);

// after this, just call your function like usual:
cached_ackermann(2, 5);
```

The first template parameter is the replacement policy (LRU replacement in this example) and the second
(optional) parameter is a mutex type. By default, it is set to `NullLock`, which is a null mutex object. This improves speed
but makes the cache not thread-safe. Pass `std::mutex` (or any other mutex object) as the second template parameter to make
this cached function thread safe:

```cpp
// Wrap 'ackermann' in a thread-safe 16-entry cache with LRU replacement policy
auto cached_thread_safe = wrap<Policy::LRU, std::mutex>(ackermann, 16);

// after this, just call your function like usual:
cached_thread_safe(2, 5);
```

Some examples on this topic are [examples/function_wrapping.cpp] for some extended examples on how to wrap a function in a
cache and [examples/multithread_function_wrapping.cpp] for a thread-safe function wrapper accessed simultaneously from
multiple threads.

### Statistics
By default, any cache object registers the number of hits, number of misses, number of entry invalidations (that is, number of 
`erase()`'d keys), number of cache invalidations (the number of `clear()` calls) and the number of evicted entries (that is, 
the number of entries deleted to make room for newer entries). If you wish to disable them, just use the fifth template
parameter like so:

```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h" // LRU replacement policy
#include "Cache/Stats/None.h" // for Stats::None (disabled/null statistics)

Cache<std::string, int, Policy::LRU, NullLock, Stats::None> cache(100);
```

Please note that the parameter `NullLock` refers to the lock type and could be `std::mutex` if we wanted the cache to be
thread-safe. Both parameters are totally independent. Please, refer to the section for [thread-safe caches](#thread-safe-cache) for more information about `NullLock`, `std::mutex` and thread-safety for this library.

That last template parameter can also be used to provide our own statistics, to have callbacks on hit/miss/evict/clear/erase
events and much more! Please check [examples/statistics.cpp], [examples/disable_statistics.cpp] and 
[examples/custom_statistics.cpp] to learn more about reading statistics from caches, how to disable statistics for a cache and 
how to roll your own custom statistics; or check the next section to learn more about callbacks, their uses and how to
implement them.

### Callbacks
Some applications will require having callbacks on certain events. With this library, it is possible to use a custom
statistics (as shown in the previous section) in order to implement callbacks on certain events. For more information,
please see  [examples/custom_statistics.cpp], [examples/custom_callbacks.cpp] and [examples/advanced_custom_callbacks.cpp].

### Replacement policies
Replacement policies are the algorithms that determine which elements get erase in order to make room for new items once
the cache is full. This library provides the following policies, although you could make your own algorithms for your specific
application:

| Replacement Policy | Defined in file           | Description                                                                |
| :----------------- | :------------------------ | :------------------------------------------------------------------------- |
| `Policy::FIFO`     | [`Cache/Policy/FIFO.h`]   | Works like a queue: the first element in is the first element out.         |
| `Policy::LFU`      | [`Cache/Policy/LFU.h`]    | Replaces the Least Frequently Used entry.                                  |
| `Policy::LIFO`     | [`Cache/Policy/LIFO.h`]   | Works like a stack: the last element in is the  first element out.         |
| `Policy::LRU`      | [`Cache/Policy/LRU.h`]    | Replaces the Least Recently Used entry.                                    |
| `Policy::MRU`      | [`Cache/Policy/MRU.h`]    | Replaces the Most Recently Used entry.                                     |
| `Policy::Random`   | [`Cache/Policy/Random.h`] | Replaces a random key. This is the default if you don't specify otherwise. |

As stated earlier, if none of the previous algorithms suit your needs, you can easily make your own and pass it to the cache.
Check out [examples/custom_replacement_policy.cpp] for an in-depth example on writing your own algorithms and creating caches
that use them.

### Other examples
For more examples or details on doing some specific task, please take a look at the [examples/] folder, which is packed with
examples and explanations for the different features of this library.

## Benchmarks
Coming soon

## Alternatives
This library aims to be as feature-complete as possible and to cover most use cases. However, we know that sometimes you
need something more specific. For those cases, you might check out one of the following:

- [goldsborough/lru-cache](https://github.com/goldsborough/lru-cache): A small library providing LRU and TLRU caches, with a similar feature set to this library.
- [vpetrigo/caches](https://github.com/vpetrigo/caches): LRU, LFU, FIFO cache C++ implementations

## Maintainer
This library was created and is currently maintained by [marcizhu](https://github.com/marcizhu).
If you find any issue with the library, you can either [open an issue](https://github.com/marcizhu/Cache/issues) or e-mail me at marcizhu@gmail.com.

## Credits
This library uses [Catch2] for unit tests. Many, many thanks to the authors and collaborators of such a great library!

## Contributing
Does this library miss any feature you'd like to have? Have you spotted any bug? PRs are always welcome! Alternatively, you
can open an issue [here](https://github.com/marcizhu/Cache/issues) or e-mail me at marcizhu@gmail.com and I'll try to
respond and/or fix it as soon as possible :D

## License
Copyright (c) 2020 Marc Izquierdo  
This library is licensed under the [MIT License](https://choosealicense.com/licenses/mit/). See
[LICENSE](https://github.com/marcizhu/Cache/blob/master/LICENSE) for more details.


[Catch2]: https://github.com/catchorg/Catch2

[`Cache/Policy/FIFO.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/FIFO.h
[`Cache/Policy/LFU.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LFU.h
[`Cache/Policy/LIFO.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LIFO.h
[`Cache/Policy/LRU.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LRU.h
[`Cache/Policy/MRU.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/MRU.h
[`Cache/Policy/Random.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/Random.h

[examples/]: https://github.com/marcizhu/Cache/blob/master/examples/
[examples/dynamic_programming.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/dynamic_programming.cpp
[examples/multithread_cache.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/multithread_cache.cpp
[examples/function_wrapping.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/function_wrapping.cpp
[examples/multithread_function_wrapping.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/multithread_function_wrapping.cpp
[examples/statistics.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/statistics.cpp
[examples/disable_statistics.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/disable_statistics.cpp
[examples/custom_statistics.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/custom_statistics.cpp
[examples/custom_callbacks.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/custom_callbacks.cpp
[examples/advanced_custom_callbacks.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/advanced_custom_callbacks.cpp
[examples/custom_replacement_policy.cpp]: https://github.com/marcizhu/Cache/blob/master/examples/custom_replacement_policy.cpp
