#pragma once

#include <list>

namespace Policy
{
	template<typename Key>
	class LIFO
	{
	private:
		std::list<Key> lifo_queue;

	public:
		LIFO() = default;
		~LIFO() = default;

		void clear() { lifo_queue.clear(); }
		void insert(const Key& key) { lifo_queue.emplace_front(key); }
		void touch (const Key& key) { (void)key; }
		void erase (const Key& key) { key == lifo_queue.front() ? lifo_queue.pop_front() : lifo_queue.remove(key); }

		const Key& replace_candidate() const { return lifo_queue.front(); }
	};
}
