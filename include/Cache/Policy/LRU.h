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
