#include <chrono>   // std::chrono_literals
#include <iostream> // std::cout
#include <mutex>    // std::mutex
#include <string>   // std::string
#include <thread>   // std::thread, std::this_thread::sleep_for()

#include "Cache/Cache.h"      // class Cache
#include "Cache/Policy/LRU.h" // Policy::LRU (LRU replacement policy)

using namespace std::chrono_literals;

// We begin by creating a global cache that both threads will be able to access.
// This could be a private class member, but for sake of simplicity we will keep
// it as a global variable.

// For this example, we will be creating a 128-entry string-int cache with LRU
// replacement policy and std::mutex as the mutex to be used by this class. This
// last template parameter is what makes it thread-safe. By deafult, this parameter
// is NullLock, which is a dummy class that does nothing but provide the std::mutex
// interface. This makes it really quick in single-thread scenarios but totally
// unsafe for multithreaded environments.

Cache<std::string, int, Policy::LRU, std::mutex> cache(128);

// This is the first thread. It will only be checking a value from cache, but in
// a real application both read, modify and write operations are supported
// simultaneously in any arbitrary number of threads (provided the caches are
// thread-safe as explained above).
void thread1()
{
	for(int i = 0; i < 10; i++)
	{
		std::this_thread::sleep_for(100ms);
		std::cout << "key 'asd' = " << cache["asd"] << std::endl;
	}
}

// This is our second thread. It will be modifying the cache after half a second,
// but as explained before, any operation can be performed as long as the cache
// is thread-safe.
void thread2()
{
	std::this_thread::sleep_for(500ms);
	cache["asd"] = 24;
}

int main()
{
	// To start with, we will insert "asd" -> 42 into the cache
	cache["asd"] = 42;

	// Spawn both threads
	std::thread t1(thread1);
	std::thread t2(thread2);

	// Wait for them to finish
	t1.join();
	t2.join();
}
