#pragma once

#include <list>
#include <unordered_map>

template<typename Key>
class LRUCachePolicy
{
private:
	std::list<Key> lru_queue;
	std::unordered_map<Key, typename std::list<Key>::iterator> key_finder;

public:
	LRUCachePolicy() = default;
	~LRUCachePolicy() = default;

	void insert(const Key& key)
	{
		lru_queue.emplace_front(key);
		key_finder[key] = lru_queue.begin();
	}

	void touch(const Key& key) { if(lru_queue.size() > 1) lru_queue.splice(lru_queue.begin(), lru_queue, key_finder[key]); }

	void erase(const Key& key)
	{
		lru_queue.erase(key_finder[key]);
		key_finder.erase(key);
	}

	const Key& replace_candidate() const { return lru_queue.back(); }
};
