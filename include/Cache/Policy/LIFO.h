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
