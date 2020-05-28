#pragma once

#include <cstdint>

namespace Stats
{
	class Basic
	{
	private:
		uint32_t m_HitCount{};
		uint32_t m_MissCount{};
		uint32_t m_EvctCount{};
		uint32_t m_FlushCount{};
		uint32_t m_InvalCount{};

	public:
		void hit    (size_t hit   = 1) { m_HitCount += hit; }
		void miss   (size_t miss  = 1) { m_MissCount += miss; }
		void flush  (size_t flush = 1) { flush == 1 ? m_FlushCount++ : m_InvalCount++; }
		void evicted(size_t evct  = 1) { m_EvctCount += evct; }

		size_t hit_count() const noexcept { return m_HitCount; }
		size_t miss_count() const noexcept { return m_MissCount; }
		size_t entry_invalidation_count() const noexcept { return m_FlushCount; }
		size_t cache_invalidation_count() const noexcept { return m_InvalCount; }
		size_t evicted_count() const noexcept { return m_EvctCount; }
	};
}
