/*
 *  This file is part of the Cache library (https://github.com/marcizhu/Cache)
 *
 *  Copyright (C) 2020-2021 Marc Izquierdo
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

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
