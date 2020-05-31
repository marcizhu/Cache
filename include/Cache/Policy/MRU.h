#pragma once

#include <list>
#include <unordered_map>

namespace Policy
{
	template<typename Key>
	class MRU
	{
	private:
		std::list<Key> mru_queue;
		std::unordered_map<Key, typename std::list<Key>::iterator> key_finder;

	public:
		MRU() = default;
		~MRU() = default;

		void clear() { mru_queue.clear(); key_finder.clear(); }

		void insert(const Key& key)
		{
			mru_queue.emplace_front(key);
			key_finder[key] = mru_queue.begin();
		}

		void touch(const Key& key) { mru_queue.splice(mru_queue.begin(), mru_queue, key_finder[key]); }

		void erase(const Key& key)
		{
			mru_queue.erase(key_finder[key]);
			key_finder.erase(key);
		}

		const Key& replace_candidate() const { return mru_queue.front(); }
	};
}
