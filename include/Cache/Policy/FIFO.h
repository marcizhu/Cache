#pragma once

#include <list>

namespace Policy
{
	template<typename Key>
	class FIFO
	{
	private:
		std::list<Key> fifo_queue;

	public:
		FIFO() = default;
		~FIFO() = default;

		void clear() { fifo_queue.clear(); }
		void insert(const Key& key) { fifo_queue.emplace_front(key); }
		void touch (const Key& key) { (void)key; }
		void erase (const Key& key) { key == fifo_queue.back() ? fifo_queue.pop_back() : fifo_queue.remove(key); }

		const Key& replace_candidate() const { return fifo_queue.back(); }
	};
}
