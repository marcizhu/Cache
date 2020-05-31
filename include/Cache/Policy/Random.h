#pragma once

#include <iterator>
#include <list>
#include <random>

namespace Policy
{
	template<typename Key>
	class Random
	{
	private:
		std::list<Key> key_storage;

		template<typename Iterator, typename RandomGenerator>
		Iterator select_randomly(Iterator start, Iterator end, RandomGenerator& g)
		{
			std::uniform_int_distribution<> distrib(0, std::distance(start, end) - 1);
			std::advance(start, distrib(g));
			return start;
		}

		template<typename Iterator>
		Iterator select_randomly(Iterator start, Iterator end)
		{
			static std::random_device rd;
			static std::mt19937 gen(rd());
			return select_randomly(start, end, gen);
		}

	public:
		Random() = default;
		~Random() = default;

		void clear() { key_storage.clear(); }
		void insert(const Key& key) { key_storage.emplace(key); }
		void touch (const Key& key) { (void)key; }
		void erase (const Key& key) { key_storage.remove(key); }

		const Key& replace_candidate() const { return *select_randomly(key_storage.begin(), key_storage.end()); }
	};
}
