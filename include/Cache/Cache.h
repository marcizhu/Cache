#pragma once

#include <limits>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

#include "Policy/None.h"
#include "Stats/Basic.h"

// missing policies: RandomReplacementPolicy, TLRU (Time-aware LRU), and ARC (Adaptive Replacement Cache)

struct NullLock
{
	void lock() {}
	bool try_lock() { return true; }
	void unlock() {}
};

template<
	typename Key,                             // Key type
	typename Value,                           // Value type
	typename CachePolicy = Policy::None<Key>, // Cache policy
	typename Lock = NullLock,                 // Lock type (for multithreading)
	typename StatsProvider = Stats::Basic     // Statistics measurement object
>
class Cache
{
private:
	const size_t m_MaxSize;
	std::unordered_map<Key, Value> m_Cache;
	mutable CachePolicy m_CachePolicy;
	mutable StatsProvider m_Stats;
	mutable Lock m_Lock;

public:
	using iterator               = typename std::unordered_map<Key, Value>::iterator;
	using const_iterator         = typename std::unordered_map<Key, Value>::const_iterator;
	using key_type               = typename std::unordered_map<Key, Value>::key_type;
	using value_type             = typename std::unordered_map<Key, Value>::value_type;
	using mapped_type            = typename std::unordered_map<Key, Value>::mapped_type;
	using reference              = typename std::unordered_map<Key, Value>::reference;
	using const_reference        = typename std::unordered_map<Key, Value>::const_reference;
//	using reverse_iterator       = typename std::unordered_map<Key, Value>::reverse_iterator;
//	using const_reverse_iterator = typename std::unordered_map<Key, Value>::const_reverse_iterator;
	using difference_type        = typename std::unordered_map<Key, Value>::difference_type;
	using size_type              = typename std::unordered_map<Key, Value>::size_type;

	Cache(const size_t max_size,
		const CachePolicy& policy = CachePolicy(),
		const Lock& lock = Lock(),
		const StatsProvider& stats = StatsProvider())
		: m_MaxSize(max_size == 0 ? std::numeric_limits<size_t>::max() : max_size), m_CachePolicy(policy), m_Stats(stats), m_Lock(lock)
	{}

	~Cache() { clear(); }

	iterator       begin()       noexcept { return m_Cache.begin(); }
	const_iterator begin() const noexcept { return m_Cache.begin(); }
	iterator       end  ()       noexcept { return m_Cache.end  (); }
	const_iterator end  () const noexcept { return m_Cache.end  (); }

//	reverse_iterator       rbegin()       noexcept { return m_Cache.rbegin(); }
//	const_reverse_iterator rbegin() const noexcept { return m_Cache.rbegin(); }
//	reverse_iterator       rend  ()       noexcept { return m_Cache.rend  (); }
//	const_reverse_iterator rend  () const noexcept { return m_Cache.rend  (); }

	const_iterator cbegin() const noexcept { return m_Cache.cbegin(); }
	const_iterator cend  () const noexcept { return m_Cache.cend  (); }

//	const_reverse_iterator crbegin() const noexcept { return m_Cache.crbegin(); }
//	const_reverse_iterator crend  () const noexcept { return m_Cache.crend  (); }

	bool empty() { return m_Cache.empty(); }

	size_type size() { return m_Cache.size(); }
	size_type max_size() { return m_MaxSize; }

	      mapped_type& at(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); m_Stats.hit(); return m_Cache.at(key); }
	const mapped_type& at(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); m_Stats.hit(); return m_Cache.at(std::forward(key)); }

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

	void insert(const key_type& key, const mapped_type& value)
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto it = find_key(key);

		if(it == end())
		{
			if(size() + 1 > m_MaxSize)
			{
				auto replaced_key = m_CachePolicy.replace_candidate();
				it = find_key(replaced_key);

				m_CachePolicy.erase(replaced_key);
				m_Cache.erase(it);
				m_Stats.evict();
			}

			m_CachePolicy.insert(key);
			m_Cache.emplace(std::make_pair(key, value));
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

		for(auto it = begin(); it != end(); it++)
			m_CachePolicy.erase(it->first);

		m_Cache.clear();
		m_Stats.clear();
	}

	void flush() noexcept { clear(); }
	void flush(const key_type& key) noexcept { erase(key); }

	iterator       find(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); return find_key(key); }
	const_iterator find(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key); }

	bool exists(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key) != end(); }

	size_type count(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return m_Cache.count(key); }

	size_type hit_count() const noexcept { return m_Stats.hit_count(); }
	size_type miss_count() const noexcept { return m_Stats.miss_count(); }
	size_type access_count() const noexcept { return m_Stats.hit_count() + m_Stats.miss_count(); }
	size_type entry_invalidation_count() const noexcept { return m_Stats.entry_invalidation_count(); }
	size_type cache_invalidation_count() const noexcept { return m_Stats.cache_invalidation_count(); }
	size_type evicted_count() const noexcept { return m_Stats.evicted_count(); }

	float hit_ratio  () const noexcept { return static_cast<float>(hit_count ())   / (static_cast<float>(hit_count() + miss_count())); }
	float miss_ratio () const noexcept { return static_cast<float>(miss_count())   / (static_cast<float>(hit_count() + miss_count())); }
	float utilization() const noexcept { return static_cast<float>(m_Cache.size()) /  static_cast<float>(m_MaxSize); }

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
