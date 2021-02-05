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

		template<typename Iterator>
		Iterator select_randomly(Iterator begin, size_t size) const
		{
			static std::random_device rd;
			static std::mt19937 gen(rd());
			static std::uniform_int_distribution<long> distrib(0, static_cast<long>(size) - 1);

			std::advance(begin, distrib(gen));
			return begin;
		}

	public:
		Random() = default;
		~Random() = default;

		void clear() { key_storage.clear(); }
		void insert(const Key& key) { key_storage.emplace_back(key); }
		void touch (const Key& key) { (void)key; }
		void erase (const Key& key) { key_storage.remove(key); }

		const Key& replace_candidate() const { return *select_randomly(key_storage.begin(), key_storage.size()); }
	};
}
