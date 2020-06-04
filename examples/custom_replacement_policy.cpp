#include <iostream> // std::cout
#include <string>   // std::string

#include "Cache/Cache.h"      // class Cache

// In this example, we will develop our own custom replacement policy. This policy
// will work like a FIFO (First In, First Out), but evict preferently keys that
// contain "test" in them

// To create our own replacement policy, we only need to create a custom templated
// class with a single template argument (the key type) and implement a few
// methods:

template<typename Key>
class CustomPolicy
{
private:
	// Here we will add all the data structures we need to track keys and to
	// decide which key to evict when the cache needs to. In our case, we will
	// use a std::list of keys to track them. Every insertion, if a key contains
	// "test", we will push it to the front of the list. If not, we will push it
	// to the back of the list.

	std::list<Key> keys;

public:
	// A policy requires the following functions:
	// - void clear();
	// - void insert(const Key&);
	// - void touch(const Key&);
	// - void erase(const Key&);
	// - const Key& replace_candidate() const;

	// This class may contain more functions, but it MUST provide the five
	// mentioned methods for it to work properly. For information about each
	// method, please read the comments over the functions:

	// clear(): This function is called when the cache is cleared. Thus, all keys
	// stored by this policy should be freed.
	// In our case, this is as simple as a call to "keys.clear()"
	void clear() { keys.clear(); }

	// insert(): This function is called when an item is inserted to the cache.
	// The key is passed as an argument so that it can be stored into any structure
	// we have.
	// In our case, we will check if it contains "test". If so, we will push it
	// to the front of the list (that will behave like a queue), if not, we will
	// push it to the back.
	// Note that this is a bad example, as it will only work if the type of Key
	// is an std::string or std::string-compatible, but it will suffice for
	// illustration purposes :D
	void insert(const Key& key)
	{
		if(key.find("test") != Key::npos)
		{
			// key contains "test" -> push to front so it will be evicted first
			keys.push_front(key);
		}
		else
		{
			// key does NOT contain "test". Push it to back, just like we would
			// on a FIFO
			keys.push_back(key);
		}
	}

	// touch(): This function gets called everytime a cache entry is accessed or
	// modified. The objective is to help the policy know how many times and which
	// keys get accessed to for example implemente a LRU or LFU policy.
	// In our example and since we will be implementing a simple FIFO, we will do
	// nothing.
	void touch(const Key& key)
	{
		// The following line prevents the compiler from generating the classic
		// "warning: unused parameter" message. It serves no other purpose than
		// that.
		(void)key;
	}

	// erase(): This function is invoked either when a key is evicted by the
	// cache or when the user calls erase(key)/flush(key) on the cache. The key
	// is passed as an argument to this function, and the policy should remove
	// the key from any structure it has, since this key is no longer present
	// in the cache.
	// In our example, we will remove the provided key from the std::list.
	void erase(const Key& key)
	{
		keys.remove(key);
	}

	// replace_candidate(): This function should return a candidate key to evict.
	// This function is called by the cache when the user tries to insert a new
	// item and the cache is full. Thus, the key returned by this method is the
	// on evicted from cache.
	// For our case, we will return the front of the queue:
	const Key& replace_candidate() const
	{
		return keys.front();
	}
};

// Keep in mind that the performance of a policy is critical: a slow policy will
// negatively impact the performance of a cache. Try to use fast algorithms and
// data structures to achieve the best performance.

int main()
{
	// to test this cache policy, we will create a small 128-entry cache of
	// string int pair that uses our own policy:
	Cache<std::string, int, CustomPolicy> cache(128);

	// we will insert 128 items (the maximum) to prove that if keys do not
	// contain "test", the policy will just behave as a FIFO
	for(int i = 0; i < cache.max_size(); i++)
	{
		cache.insert(std::to_string(i), i);
	}

	std::cout << "Cache contains key '0': " << std::boolalpha << cache.contains("0") << std::endl;
	std::cout << "Cache contains key '1': " << std::boolalpha << cache.contains("1") << std::endl;

	// This insertion will evict key "0" and because it contains "test", it will
	// be the next key to be evicted from cache
	cache.insert("test key", 42);

	std::cout << "Cache contains key '0': " << std::boolalpha << cache.contains("0") << std::endl;
	std::cout << "Cache contains key '1': " << std::boolalpha << cache.contains("1") << std::endl;
	std::cout << "Cache contains key 'test key': " << std::boolalpha << cache.contains("test key") << std::endl;

	cache.insert("other key", 10); // This insertion will evict "test key" from cache

	std::cout << "Cache contains key '1': " << std::boolalpha << cache.contains("1") << std::endl;
	std::cout << "Cache contains key 'test key': " << std::boolalpha << cache.contains("test key") << std::endl;
}
