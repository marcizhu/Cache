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

#include <cstdint>

namespace Stats
{
	template<typename Key, typename Value>
	class Basic
	{
	private:
		uint32_t m_HitCount{};
		uint32_t m_MissCount{};
		uint32_t m_EvctCount{};
		uint32_t m_EraseCount{};
		uint32_t m_InvalCount{};

	public:
		void clear()                         noexcept { m_InvalCount++; }
		void hit  (const Key&, const Value&) noexcept { m_HitCount++; }
		void miss (const Key&)               noexcept { m_MissCount++; }
		void erase(const Key&, const Value&) noexcept { m_EraseCount++; }
		void evict(const Key&, const Value&) noexcept { m_EvctCount++; }

		constexpr size_t hit_count() const noexcept { return m_HitCount; }
		constexpr size_t miss_count() const noexcept { return m_MissCount; }
		constexpr size_t entry_invalidation_count() const noexcept { return m_EraseCount; }
		constexpr size_t cache_invalidation_count() const noexcept { return m_InvalCount; }
		constexpr size_t evicted_count() const noexcept { return m_EvctCount; }
	};
}
