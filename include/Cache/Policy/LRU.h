#pragma once

#include <list>
#include <unordered_map>

namespace Policy
{
	template<typename Key>
	class LRU
	{
	private:
		std::list<Key> lru_queue;
		std::unordered_map<Key, typename std::list<Key>::iterator> key_finder;

	public:
		LRU() = default;
		~LRU() = default;

		void clear() { lru_queue.clear(); key_finder.clear(); }

		void insert(const Key& key)
		{
			lru_queue.emplace_front(key);
			key_finder[key] = lru_queue.begin();
		}

		void touch(const Key& key) { lru_queue.splice(lru_queue.begin(), lru_queue, key_finder[key]); }

		void erase(const Key& key)
		{
			lru_queue.erase(key_finder[key]);
			key_finder.erase(key);
		}

		const Key& replace_candidate() const { return lru_queue.back(); }
	};
}
