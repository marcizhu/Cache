#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <unordered_map>

namespace Policy
{
	template<typename Key>
	class LFU
	{
	private:
		std::multimap<std::size_t, Key> frequency_storage;
		std::unordered_map<Key, typename std::multimap<std::size_t, Key>::iterator> lfu_storage;

	public:
		LFU() = default;
		~LFU() = default;

		void clear() { frequency_storage.clear(); lfu_storage.clear(); }

		void insert(const Key& key)
		{
			constexpr size_t INITIAL_VALUE = 1;
			lfu_storage[key] = frequency_storage.emplace_hint(frequency_storage.begin(), INITIAL_VALUE, key);
		}

		void touch(const Key& key)
		{
			auto elem_for_update = lfu_storage[key];
			auto updated_elem = std::make_pair(elem_for_update->first + 1, elem_for_update->second);

			frequency_storage.erase(elem_for_update);
			lfu_storage[key] = frequency_storage.emplace_hint(frequency_storage.end(), std::move(updated_elem));
		}

		void erase(const Key& key)
		{
			frequency_storage.erase(lfu_storage[key]);
			lfu_storage.erase(key);
		}

		const Key& replace_candidate() const { return frequency_storage.begin()->second; }
	};
}
