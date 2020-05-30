#pragma once

namespace Stats
{
	template<typename Key, typename Value>
	struct None
	{
		constexpr void clear()                         const noexcept {}
		constexpr void hit  (const Key&, const Value&) const noexcept {}
		constexpr void miss (const Key&)               const noexcept {}
		constexpr void erase(const Key&, const Value&) const noexcept {}
		constexpr void evict(const Key&, const Value&) const noexcept {}

		constexpr size_t hit_count() const noexcept { return 0ULL; }
		constexpr size_t miss_count() const noexcept { return 0ULL; }
		constexpr size_t entry_invalidation_count() const noexcept { return 0ULL; }
		constexpr size_t cache_invalidation_count() const noexcept { return 0ULL; }
		constexpr size_t evicted_count() const noexcept { return 0ULL; }
	};
}
