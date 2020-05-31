# Cache
[![Build Status](https://travis-ci.com/marcizhu/Cache.svg?branch=master)](https://travis-ci.com/marcizhu/Cache)
[![License](https://img.shields.io/github/license/marcizhu/Cache)](https://github.com/marcizhu/Cache/blob/master/LICENSE)
[![Stability: Experimental](https://masterminds.github.io/stability/experimental.svg)](https://masterminds.github.io/stability/experimental.html)

A small, lightweight, thread-safe, easy to use, header-only, fast and simple cache with selectable replacement algorithms!

This is a small library aimed to provide a simple and intuitive cache with selectable replacement policies. The cache has the
same API as a `std::map`/`std::unordered_map` (with some nice extra features :D). Please note that this library is still in
development. Any kind of input/feedback is always welcome!

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
doing that is to store all data in memory, but most often than not that is impractical. A more optimal approach would be to
store a small subset of the most used (or most expensive to access) data in a small data structure.

That is the intent of this library: to provide a small and simple data structure with a fixed size (but it can be left 
unbounded too) for caching function call parameters, recursive calls or any type of resources, like game assets, files or
other types of resources that might be too frequently accessed and take too much time to read or write to.

The cache replacement algorithm can be chosen at compile time (you can even write your own and use it) so that your 
application has the best performance possible is any scenario. And speaking of performance, by default the cache tracks some
statistics (that is, number of hits, number of misses, number of entries erased by the user, number of cache clears and number
of entries evicted). Those statistics can be disabled at compile time for better performance or even make your own custom 
stats to track specific keys, implement callbacks and much more!

## Install
The provided `CMakeLists.txt` file exports a `Cache` library, so if your build system is based on CMake, you can use
that. If that is not the case, copy the `include/Cache` folder to your `include`/`vendor`/`deps` folder and you're ready to 
go!

The CMake file also provides two options:
- `CACHE_BUILD_TESTS` (default: `OFF`): Builds tests. This will require the [Catch2]
library, which is already included in `deps/Catch2`
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
For more information and an in-depth explaination on how it works, please see [examples/dynamic_programming.cpp]

The class `Cache` is 100% compatible with a `std::map`/`std::unordered_map`, and can even have an unlimited size (just pass 0
to the constructor and the size will be unlimited), but it also has some useful "extra" functions, like:
- `bool contains(key)`: Returns true if the cache contains the provided key. `false` otherwise
- `value& lookup(key)`: Alias for `at(key)`
- `void flush(key)`: Alias for `erase(key)`
- `void flush()`: Alias for `clear()`
- `size_t hit_count()`/`miss_count()`/`access_count()`/`entry_invalidation_count()`/`cache_invalidation_count()`/`evicted_count()`: Returns statistics about hits, misses, accesses, etc...
- `float hit_ratio()`/`float miss_ratio()`/`float utilization()`: Returns statistics about the hit/miss ratio and utilization (that is, size / capacity)

### Thread-safe cache
Caches are **NOT** thread-safe by default. This is done to prevent the continuous atomic locking and unlocking of mutex 
objects in single-threaded scenarios, which would reduce performance. If you need to use a thread safe cache, just pass 
`std::mutex` (or any other mutex-like type) as the third template parameter of the `Cache` class:

```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h"

Cache<std::string, int, Policy::LRU, std::mutex> cache(100);
```

As stated earlier, by default the cache is not thread-safe. That is becausee the third template parameter defaults to 
`NullLock` (defined in Cache/Cache.h), which is a dummy lock that does nothing. You can supply that as the third parameter if
you want to explicitly disable mutithread synchronization.

For more examples on multithreading, please see [examples/multithread_cache.cpp] and  [examples/multithread_function_wrapping.cpp]

### Function wrapping
Sometimes our applications have expensive functions, that take too long to compute some data. Thus, we want to prevent
recalculating the same data over and over. This is the perfect job for a cache!

We could manually add a wrapper function what has a cache, checks if a given set of parameters is in the cache and if so,
returns the result immediately. If not, calls the function with the original parameters and stores the result in the cache.
But why do it manually when this library already provides this awesome feature?

It is important to note that this type of cache **will not** store the returned values from recursive calls (since the cache
has no access to that data). Instead, it will only cache input-output data pairs. Thus, this is more like a "shallow" cache.

The following snippet will show how to wrap any non-void function with a cache:
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

As you can see, wrapping a function in a cache is a matter of including `Cache/Wrapper.h` and creating an object by calling 
the `wrap()` function. The first template parameter is the replacement policy (LRU replacement in this example) and the second
(optional) parameter is a mutex type. By default, it is set to `NullLock`, which is a null mutex object. This improves speed
but makes the cache not thread-safe. Pass `std::mutex` (or any other mutex object) as the second template parameter to make
this cached function thread safe

Some examples on this topic are [examples/function_wrapping.cpp] for some extended examples on how to wrap a function in a
cache and [examples/multithread_function_wrapping.cpp] for a thread-safe function wrapper accessed simultaneously from
multiple threads.

### Statistics
Sometimes we want to know how well is doing our cache. Or maybe not! Maybe we know it is doing a fantastic job and want to
disable statistical measurement to squeeze out every single bit of performance.

By default, any cache object registers the number of hits, number of misses, number of entry invalidations (that is, number of 
`erase()`'d keys), number of cache invalidations (the number of `clear()` calls) and the number of evicted entries (that is, 
the number of entries deleted to make room for newer entries). If you wish to disable them, just use the fourth template
parameter like so:

```cpp
#include "Cache/Cache.h"
#include "Cache/Policy/LRU.h" // LRU replacement policy
#include "Cache/Stats/None.h" // for Stats::None (disabled/null statistics)

Cache<std::string, int, Policy::LRU, NullLock, Stats::None> cache(100);
```

Please note that the parameter `NullLock` refers to the lock type and could be `std::mutex` if we wanted the cache to be
thread-safe. Both parameters are totally independent.

That last template parameter can also be used to provide our own statistics, to have callbacks on hit/miss/evict/clear/erase
events and much more! Please check [examples/statistics.cpp], [examples/disable_statistics.cpp] and 
[examples/custom_statistics.cpp] to learn more about reading statistics from caches, how to disable statistics for a cache and 
how to roll your own custom statistics; or check the next section to learn more about callbacks, their uses and how to
implement them.

### Callbacks
Some applications will require having callbacks on certain events. For example, say we are developing a text editor, and we
cache file data to prevent continuously writing changes to files. In that scenario, when an entry is evicted or erased from 
cache, we would need to write it down to the actual file.

With this library, this isn't an issue. Using the custom statistics (as shown in the previous section) we can implement custom
callbacks on certain events. For more information, please see  [examples/custom_statistics.cpp], 
[examples/custom_callbacks.cpp] and [examples/advanced_custom_callbacks.cpp].

### Replacement policies
Replacement policies are in charge of determining what elements get erase to make room for new items once the cache is full.
This library provides the following policies, although you could make your own algorithms for your specific application:

- `Policy::FIFO`: (defined inside [`Cache/Policy/FIFO.h`]). Works like a queue: the first element in is the first element out.
- `Policy::LFU`: (defined inside [`Cache/Policy/LFU.h`]). Replaces the Least Frequently Used entry.
- `Policy::LIFO`: (defined inside [`Cache/Policy/LIFO.h`]). Works like a stack: the last element in is the  first element out.
- `Policy::LRU`: (defined inside [`Cache/Policy/LRU.h`]). Replaces the Least Recently Used entry.
- `Policy::MRU`: (defined inside [`Cache/Policy/MRU.h`]). Replaces the Most Recently Used entry.
- `Policy::None`: (defined inside [`Cache/Policy/None.h`]). Replaces the smallest key (lexicographically).
- `Policy::Random`: (defined inside [`Cache/Policy/Random.h`]). Replaces a random key.

Depending on your application, you might choose different algorithms for your caches. For example, if your data is more likely
to be accessed the older it is, you might want to use the MRU replacement algorithm. In contrast, if your data is more likely
to be used the newer it is, LRU is a good candidate here. When in doubt, LRU or LFU are good candidates to start with. 
Remember, changing the algorithm is so simple as changing the include file and the template parameter!

And, of course, if any of the previous algorithms suite your needs, you can also make your own and pass it to the cache.
Check out [examples/custom_replacement_policy.cpp] for an in-depth example on writing your own algorithms and creating caches
that use them.

### Other examples
For more examples or details on doing some specific task, please take a look at the [examples/] folder, which is packed with
examples and explainations for the different features of this library.

## Benchmarks
Comming soon

## Alternatives
This library aims to be as feature-complete as possible, and to cover most use cases. However, we know that sometimes you
need something more specific. For those cases, you might check out one of the following:

- [goldsborough/lru-cache](https://github.com/goldsborough/lru-cache): A small library providing LRU and TLRU caches, with a similar feature set to this library.
- [vpetrigo/caches](https://github.com/vpetrigo/caches): LRU, LFU, FIFO cache C++ implementations

## Maintainer
This library was created and is currently maintained by [marcizhu](https://github.com/marcizhu).
If you find any issue with the library, you can either [open an issue](https://github.com/marcizhu/Units/issues) or e-mail me at marcizhu@gmail.com.

## Credits
This library uses [Catch2] for unit tests. Thanks to the authors and collaborators of such a great library!

## Contributing
Does this library miss any feature you'd like to have? Have you spotted any bug? PRs are always welcome! Alternatively, you
can open an issue [here](https://github.com/marcizhu/Units/issues) or e-mail me at marcizhu@gmail.com and I'll try to
respond and/or fix it as soon as possible :D

## License
Copyright (c) 2020 Marc Izquierdo  
This library is licensed under the [MIT License](https://choosealicense.com/licenses/mit/). See
[LICENSE](https://github.com/marcizhu/Units/blob/master/LICENSE) for more details.


[Catch2]: https://github.com/catchorg/Catch2

[`Cache/Policy/FIFO.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/FIFO.h
[`Cache/Policy/LFU.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LFU.h
[`Cache/Policy/LIFO.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LIFO.h
[`Cache/Policy/LRU.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/LRU.h
[`Cache/Policy/MRU.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/MRU.h
[`Cache/Policy/None.h`]: https://github.com/marcizhu/Cache/blob/master/include/Cache/Policy/None.h
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
