#include <cstdint>  // std::uint64_t
#include <iostream> // std::cout
#include <mutex>    // std::mutex
#include <string>   // std::string
#include <thread>   // std::thread
#include <chrono>   // std::chrono_literals

#include "Cache/Cache.h"      // class Cache
#include "Cache/Wrapper.h"    // wrap(...)
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

// This is the target function that we will be wrapping around a cache. This
// function could be anything, really. As long as it takes some parameters and
// is non-void, it will work!

// For our small example, we will use a trivial implementation of the fibonacci
// function.

using namespace std::chrono_literals;

std::uint64_t fibonacci(std::uint64_t n)
{
	if(n < 2) return n;
	return fibonacci(n - 1) + fibonacci(n - 2);
}

// We continue by creating the wrapper as is 'function_wrapping.cpp'. But now, we
// will pass thee extra std::mutex template parameter (see multithread_cache.cpp
// for more information)

// Our cache will bee a thread-safe, 16-entry cache with LRU replacement algorithm

auto cached_fibonacci = wrap<Policy::LRU, std::mutex>(fibonacci, 16);

// This is the first thread. It will call the cached_fibonacci() function at the
// same time the thread 2 is calling it.
void thread1()
{
	// Inside this function we will call cached_fibonacci 6 times at the same
	// time the second thread is calling them.

	auto r1 = cached_fibonacci(45); std::cout << "T1: fib(45) = " << r1 << std::endl;
	auto r2 = cached_fibonacci(12); std::cout << "T1: fib(12) = " << r2 << std::endl;
	auto r3 = cached_fibonacci(15); std::cout << "T1: fib(15) = " << r3 << std::endl;
	auto r4 = cached_fibonacci(20); std::cout << "T1: fib(20) = " << r4 << std::endl;
	auto r5 = cached_fibonacci(47); std::cout << "T1: fib(47) = " << r5 << std::endl;
	auto r6 = cached_fibonacci(47); std::cout << "T1: fib(47) = " << r6 << std::endl;
}

// This is our second thread. It will too call cached_fibonacci() concurrently
// with thread 1.
void thread2()
{
	// For thread two, we will see an interesting pattern: while thread 2 is
	// executing cached_fibonacci(47), thread 1 will be executing cached_fibonacci(45).
	// Since the thread 1 will finish first, we will see how later calls to
	// cached_fibonacci(45) from this thread take no time at all, and how the
	// last calls to cached_fibonacci(47) on thread 1 will return immediately.

	auto r1 = cached_fibonacci(45); std::cout << "T2: fib(47) = " << r1 << std::endl;
	auto r2 = cached_fibonacci(45); std::cout << "T2: fib(45) = " << r2 << std::endl;
	auto r3 = cached_fibonacci(30); std::cout << "T2: fib(30) = " << r3 << std::endl;
	auto r4 = cached_fibonacci(15); std::cout << "T2: fib(15) = " << r4 << std::endl;
	auto r5 = cached_fibonacci(45); std::cout << "T2: fib(45) = " << r5 << std::endl;
	auto r6 = cached_fibonacci(47); std::cout << "T2: fib(47) = " << r6 << std::endl;
}

int main()
{
	// Spawn both threads
	std::thread t1(thread1);
	std::thread t2(thread2);

	// Wait for them to finish
	t1.join();
	t2.join();
}
