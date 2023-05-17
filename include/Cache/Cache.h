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

#include <limits>
#include <mutex>
#include <unordered_map>

#include "Policy/Random.h"
#include "Stats/Basic.h"
#include "detail/utility.h"

struct NullLock
{
	void lock() const noexcept {}
	bool try_lock() const noexcept { return true; }
	void unlock() const noexcept {}
};

template<
	typename Key,                                            // Key type
	typename Value,                                          // Value type
	template<typename> class CachePolicy = Policy::Random,   // Cache policy
	typename Lock = NullLock,                                // Lock type (for multithreading)
	template<typename...> class StatsProvider = Stats::Basic // Statistics measurement object
>
class Cache
{
private:
	using underlying_storage = std::unordered_map<Key, Value>;

	const size_t m_MaxSize;
	underlying_storage m_Cache;
	mutable CachePolicy<Key> m_CachePolicy;
	mutable StatsProvider<Key, Value> m_Stats;
	mutable Lock m_Lock;

public:
	using key_type        = typename underlying_storage::key_type;
	using mapped_type     = typename underlying_storage::mapped_type;
	using value_type      = typename underlying_storage::value_type;
	using reference       = typename underlying_storage::reference;
	using const_reference = typename underlying_storage::const_reference;
	using pointer         = typename underlying_storage::pointer;
	using const_pointer   = typename underlying_storage::const_pointer;
	using iterator        = typename underlying_storage::iterator;
	using const_iterator  = typename underlying_storage::const_iterator;
	using size_type       = typename underlying_storage::size_type;
	using difference_type = typename underlying_storage::difference_type;

	Cache(
		const size_t max_size,
		const CachePolicy<Key>& policy = CachePolicy<Key>(),
		const StatsProvider<Key, Value>& stats = StatsProvider<Key, Value>())
		: m_MaxSize(max_size == 0 ? std::numeric_limits<size_t>::max() : max_size), m_CachePolicy(policy), m_Stats(stats), m_Lock()
	{
		std::lock_guard<Lock> lock(m_Lock);
		static constexpr size_t MAX_RESERVE_SIZE = 1024;
		m_Cache.reserve(m_MaxSize < MAX_RESERVE_SIZE ? m_MaxSize : MAX_RESERVE_SIZE);
	}

	Cache(
		const size_t max_size,
		const Lock& lock,
		const CachePolicy<Key>& policy = CachePolicy<Key>(),
		const StatsProvider<Key, Value>& stats = StatsProvider<Key, Value>())
		: m_MaxSize(max_size == 0 ? std::numeric_limits<size_t>::max() : max_size), m_CachePolicy(policy), m_Stats(stats), m_Lock(lock)
	{
		std::lock_guard<Lock> mlock(m_Lock);
		static constexpr size_t MAX_RESERVE_SIZE = 1024;
		m_Cache.reserve(m_MaxSize < MAX_RESERVE_SIZE ? m_MaxSize : MAX_RESERVE_SIZE);
	}

	Cache(const Cache& other)
		: m_MaxSize(other.m_MaxSize)
	{
		std::lock(m_Lock, other.m_Lock);
		std::lock_guard<Lock> lhs_lk(m_Lock, std::adopt_lock);
		std::lock_guard<Lock> rhs_lk(other.m_Lock, std::adopt_lock);

		m_Cache = other.m_Cache;
		m_Stats = other.m_Stats;
		m_CachePolicy = other.m_CachePolicy;
	}

	~Cache() = default;

	      iterator  begin()       noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.begin(); }
	const_iterator  begin() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.begin(); }
	const_iterator cbegin() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.cbegin(); }

	      iterator  end()       noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.end(); }
	const_iterator  end() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.end(); }
	const_iterator cend() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.cend(); }

	bool empty() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.empty(); }

	size_type size() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.size(); }
	constexpr size_type max_size() const noexcept { return m_MaxSize; }

	      mapped_type& at(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); return m_Cache.at(key); }
	const mapped_type& at(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return m_Cache.at(key); }

	      mapped_type& lookup(const key_type& key)       { return at(key); }
	const mapped_type& lookup(const key_type& key) const { return at(key); }

	mapped_type& operator[](const key_type&  key) { return insert(key, mapped_type()).first->second; }
	mapped_type& operator[](      key_type&& key) { return insert(key, mapped_type()).first->second; }

	iterator erase(const_iterator pos) { std::lock_guard<Lock> lock(m_Lock); m_CachePolicy.erase(pos->first); m_Stats.erase(pos->first, pos->second); return m_Cache.erase(pos); }

	iterator erase(const_iterator first, const_iterator last)
	{
		std::lock_guard<Lock> lock(m_Lock);
		for(auto it = first; it != last; ++it)
		{
			m_CachePolicy.erase(it->first);
			m_Stats.erase(it->first, it->second);
		}

		return m_Cache.erase(first, last);
	}

	size_type erase(const key_type& key)
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto it = find_key(key);

		if(it == m_Cache.end()) return 0;

		m_CachePolicy.erase(key);
		m_Stats.erase(it->first, it->second);
		m_Cache.erase(it);

		return 1;
	}

  	template<typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto pair = m_Cache.emplace(args...);

		if(pair.second == true)
		{
			if(m_Cache.size() > m_MaxSize)
			{
				auto replaced_key = m_CachePolicy.replace_candidate();
				auto it = m_Cache.find(replaced_key);

				m_CachePolicy.erase(replaced_key);
				m_Stats.evict(it->first, it->second);
				m_Cache.erase(it);
			}

			m_CachePolicy.insert(pair.first->first);
		}
		else
			m_CachePolicy.touch(pair.first->first);

		return pair;
	}

	template<typename InputIt>
	void insert(InputIt first, InputIt last)
	{
		for(; first != last; ++first)
			insert(first->first, first->second);
	}

	void insert(std::initializer_list<value_type> ilist) { insert(ilist.begin(), ilist.end()); }
	std::pair<iterator, bool> insert(const value_type& val) { return insert(val.first, val.second); }

	std::pair<iterator, bool> insert(const key_type& key, const mapped_type& value) noexcept
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto it = m_Cache.find(key);

		if(it == m_Cache.end())
		{
			if(m_Cache.size() + 1 > m_MaxSize)
			{
				auto replaced_key = m_CachePolicy.replace_candidate();
				it = m_Cache.find(replaced_key);

				m_CachePolicy.erase(replaced_key);
				m_Stats.evict(it->first, it->second);
				m_Cache.erase(it);
			}

			m_CachePolicy.insert(key);
			return m_Cache.insert(std::make_pair(key, value));
		}
		else
		{
			m_CachePolicy.touch(key);
			return { it, false };
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

	bool   contains(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key) != m_Cache.end(); }
	size_type count(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key) != m_Cache.end(); }

	      iterator find(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); return find_key(key); }
	const_iterator find(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return find_key(key); }

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

		return (it != m_Cache.end() ?
			(m_Stats.hit(it->first, it->second), m_CachePolicy.touch(key), it) :
			(m_Stats.miss(key), it));
	}

	const_iterator find_key(const key_type& key) const
	{
		auto it = m_Cache.find(key);

		return (it != m_Cache.end() ?
			(m_Stats.hit(it->first, it->second), m_CachePolicy.touch(key), it) :
			(m_Stats.miss(key), it));
	}
};

namespace std
{
	template<
		typename Key,
		typename Value,
		template<typename> class Policy,
		typename Lock,
		template<typename...> class Stats,
		typename Pred
	>
	typename Cache<Key, Value, Policy, Lock, Stats>::size_type erase_if(Cache<Key, Value, Policy, Lock, Stats>& c, Pred pred)
	{
		auto old_size = c.size();
		for(auto i = c.begin(), last = c.end(); i != last;)
		{
			if(pred(*i))
				i = c.erase(i);
			else
				++i;
		}

		return old_size - c.size();
	}
}
