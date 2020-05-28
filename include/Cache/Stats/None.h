#pragma once

namespace Stats
{
	struct None
	{
		void hit    (size_t = 1) {}
		void miss   (size_t = 1) {}
		void flush  (size_t = 1) {}
		void evicted(size_t = 1) {}

		size_t hit_count() const noexcept { return 0ULL; }
		size_t miss_count() const noexcept { return 0ULL; }
		size_t entry_invalidation_count() const noexcept { return 0ULL; }
		size_t cache_invalidation_count() const noexcept { return 0ULL; }
		size_t evicted_count() const noexcept { return 0ULL; }
	};
}
