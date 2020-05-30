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
