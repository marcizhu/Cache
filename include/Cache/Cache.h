#pragma once

#include <limits>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

#include "Policy/None.h"
#include "Stats/Basic.h"

// missing policies: TLRU (Time-aware LRU), and ARC (Adaptive Replacement Cache)

struct NullLock
{
	void lock() const noexcept {}
	void unlock() const noexcept {}
};

template<
	typename Key,                                        // Key type
	typename Value,                                      // Value type
	template<typename> class CachePolicy = Policy::None, // Cache policy
	typename Lock = NullLock,                            // Lock type (for multithreading)
	typename StatsProvider = Stats::Basic                // Statistics measurement object
>
class Cache
{
private:
	using underlying_storage = std::unordered_map<Key, Value>;

	const size_t m_MaxSize;
	underlying_storage m_Cache;
	mutable CachePolicy<Key> m_CachePolicy;
	mutable StatsProvider m_Stats;
	mutable Lock m_Lock;

public:
	using iterator        = typename underlying_storage::iterator;
	using const_iterator  = typename underlying_storage::const_iterator;
	using key_type        = typename underlying_storage::key_type;
	using value_type      = typename underlying_storage::value_type;
	using mapped_type     = typename underlying_storage::mapped_type;
	using reference       = typename underlying_storage::reference;
	using const_reference = typename underlying_storage::const_reference;
	using difference_type = typename underlying_storage::difference_type;
	using size_type       = typename underlying_storage::size_type;

	Cache(const size_t max_size, const CachePolicy<Key>& policy = CachePolicy<Key>(), const StatsProvider& stats = StatsProvider())
		: m_MaxSize(max_size == 0 ? std::numeric_limits<size_t>::max() : max_size), m_CachePolicy(policy), m_Stats(stats), m_Lock()
	{}

	~Cache() = default;

	Cache(const Cache& other)
		: m_MaxSize(other.m_MaxSize)
	{
		std::lock(m_Lock, other.m_Lock);
		std::lock_guard<std::mutex> lhs_lk(m_Lock, std::adopt_lock);
		std::lock_guard<std::mutex> rhs_lk(other.m_Lock, std::adopt_lock);

		m_Cache = other.m_Cache;
		m_Stats = other.m_Stats;
		m_CachePolicy = other.m_CachePolicy;
	}

	      iterator begin()       noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.begin(); }
	const_iterator begin() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.begin(); }
	      iterator end  ()       noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.end  (); }
	const_iterator end  () const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.end  (); }

	const_iterator cbegin() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.cbegin(); }
	const_iterator cend  () const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.cend  (); }

	bool empty() { std::lock_guard<Lock> lock(m_Lock); return m_Cache.empty(); }

	size_type size() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.size(); }
	constexpr size_type max_size() const noexcept { return m_MaxSize; }

	      mapped_type& at(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); m_Stats.hit(); return m_Cache.at(key); }
	const mapped_type& at(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); m_Stats.hit(); return m_Cache.at(key); }

	      mapped_type& lookup(const key_type& key)       { return at(key); }
	const mapped_type& lookup(const key_type& key) const { return at(key); }

	size_type erase(const key_type& key)
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto it = find_key(key);

		if(it == end()) return 0ULL;

		m_CachePolicy.erase(key);
		m_Cache.erase(it);
		m_Stats.erase();

		return 1ULL;
	}

	// TODO: emplace()

	void insert(const key_type& key, const mapped_type& value) noexcept
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto it = find_key(key);

		if(it == end())
		{
			if(size() + 1 > m_MaxSize)
			{
				auto replaced_key = m_CachePolicy.replace_candidate();
				it = m_Cache.find(replaced_key);

				m_CachePolicy.erase(replaced_key);
				m_Cache.erase(it);
				m_Stats.evict();
			}

			m_CachePolicy.insert(key);
			m_Cache.insert(std::make_pair(key, value));
		}
		else
		{
			m_CachePolicy.touch(key);
			it->second = value;
		}
	}

	void clear() noexcept
	{
		std::lock_guard<Lock> lock(m_Lock);

		m_CachePolicy.clear();
		m_Cache.clear();
		m_Stats.clear();
	}

	void flush() noexcept { clear(); }
	void flush(const key_type& key) noexcept { erase(key); }

	bool exists(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key) != end(); }

	size_type count(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return m_Cache.count(key); }

	      iterator find(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); return find_key(key); }
	const_iterator find(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key); }

	size_type hit_count() const noexcept { return m_Stats.hit_count(); }
	size_type miss_count() const noexcept { return m_Stats.miss_count(); }
	size_type access_count() const noexcept { return m_Stats.hit_count() + m_Stats.miss_count(); }
	size_type entry_invalidation_count() const noexcept { return m_Stats.entry_invalidation_count(); }
	size_type cache_invalidation_count() const noexcept { return m_Stats.cache_invalidation_count(); }
	size_type evicted_count() const noexcept { return m_Stats.evicted_count(); }

	constexpr float hit_ratio  () const noexcept { return static_cast<float>(hit_count ())   / (static_cast<float>(hit_count() + miss_count())); }
	constexpr float miss_ratio () const noexcept { return static_cast<float>(miss_count())   / (static_cast<float>(hit_count() + miss_count())); }
	constexpr float utilization() const noexcept { return static_cast<float>(m_Cache.size()) /  static_cast<float>(m_MaxSize); }

private:
	iterator find_key(const key_type& key)
	{
		auto it = m_Cache.find(key);

		return (it != end() ?
			(m_Stats.hit(), m_CachePolicy.touch(key), it) :
			(m_Stats.miss(), it));
	}

	const_iterator find_key(const key_type& key) const
	{
		auto it = m_Cache.find(key);

		return (it != end() ?
			(m_Stats.hit(), m_CachePolicy.touch(key), it) :
			(m_Stats.miss(), it));
	}
};
