#pragma once

namespace Stats
{
	struct None
	{
		constexpr void hit  () const noexcept {}
		constexpr void miss () const noexcept {}
		constexpr void erase() const noexcept {}
		constexpr void clear() const noexcept {}
		constexpr void evict() const noexcept {}

		constexpr size_t hit_count() const noexcept { return 0ULL; }
		constexpr size_t miss_count() const noexcept { return 0ULL; }
		constexpr size_t entry_invalidation_count() const noexcept { return 0ULL; }
		constexpr size_t cache_invalidation_count() const noexcept { return 0ULL; }
		constexpr size_t evicted_count() const noexcept { return 0ULL; }
	};
}