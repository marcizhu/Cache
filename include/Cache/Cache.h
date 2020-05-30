#pragma once

#include <limits>
#include <mutex>
#include <unordered_map>

#include "Policy/None.h"
#include "Stats/Basic.h"
#include "detail/utility.h"

// missing policies: TLRU (Time-aware LRU), and ARC (Adaptive Replacement Cache)

struct NullLock
{
	void lock() const noexcept {}
	bool try_lock() const noexcept { return true; }
	void unlock() const noexcept {}
};

template<
	typename Key,                                            // Key type
	typename Value,                                          // Value type
	template<typename> class CachePolicy = Policy::None,     // Cache policy
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

	Cache(const size_t max_size, const CachePolicy<Key>& policy = CachePolicy<Key>(), const StatsProvider<Key, Value>& stats = StatsProvider<Key, Value>())
		: m_MaxSize(max_size == 0 ? std::numeric_limits<size_t>::max() : max_size), m_CachePolicy(policy), m_Stats(stats), m_Lock()
	{
		static constexpr size_t MAX_RESERVE_SIZE = 1024;
		m_Cache.reserve(m_MaxSize < MAX_RESERVE_SIZE ? m_MaxSize : MAX_RESERVE_SIZE);
	}

	~Cache() = default;

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

	      iterator begin()       noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.begin(); }
	const_iterator begin() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.begin(); }
	      iterator end  ()       noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.end  (); }
	const_iterator end  () const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.end  (); }

	const_iterator cbegin() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.cbegin(); }
	const_iterator cend  () const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.cend  (); }

	bool empty() { std::lock_guard<Lock> lock(m_Lock); return m_Cache.empty(); }

	size_type size() const noexcept { std::lock_guard<Lock> lock(m_Lock); return m_Cache.size(); }
	constexpr size_type max_size() const noexcept { return m_MaxSize; }

	// TODO: Optimize this!
	      mapped_type& at(const key_type& key)       { std::lock_guard<Lock> lock(m_Lock); return m_Cache.at(key); }
	const mapped_type& at(const key_type& key) const { std::lock_guard<Lock> lock(m_Lock); return m_Cache.at(key); }

	      mapped_type& lookup(const key_type& key)       { return at(key); }
	const mapped_type& lookup(const key_type& key) const { return at(key); }

	      mapped_type& operator[](const Key& key)       { return (insert(key, mapped_type()).first)->second; }
	const mapped_type& operator[](const Key& key) const { return (insert(key, mapped_type()).first)->second; }

	void erase(iterator pos) { std::lock_guard<Lock> lock(m_Lock); m_CachePolicy.erase(pos->first); m_Cache.erase(pos); m_Stats.erase(pos->first, pos->second); }

	template<typename Iterator>
	void erase(Iterator begin, Iterator end)
	{
		for(; begin != end; ++begin)
			erase(begin);
	}

	size_type erase(const key_type& key)
	{
		std::lock_guard<Lock> lock(m_Lock);
		auto it = find_key(key);

		if(it == m_Cache.end()) return 0;

		m_CachePolicy.erase(key);
		m_Cache.erase(it);
		m_Stats.erase(it->first, it->second);

		return 1;
	}

  	template<typename... Ks, typename... Vs>
	std::pair<iterator, bool> emplace(std::piecewise_construct_t, const std::tuple<Ks...>& key_arguments, const std::tuple<Vs...>& value_arguments)
	{
		std::lock_guard<Lock> lock(m_Lock);

		auto key = detail::construct_from_tuple<key_type>(key_arguments);
		auto it = m_Cache.find(key);

		if(it == m_Cache.end())
		{
			if(m_Cache.size() + 1 > m_MaxSize)
			{
				auto replaced_key = m_CachePolicy.replace_candidate();
				it = m_Cache.find(replaced_key);

				m_CachePolicy.erase(replaced_key);
				m_Cache.erase(it);
				m_Stats.evict(it->first, it->second);
			}

			auto val = detail::construct_from_tuple<mapped_type>(value_arguments);
			m_CachePolicy.insert(key);
			return m_Cache.emplace(std::move(key), std::move(val));
		}
		else
		{
			m_CachePolicy.touch(key);
			return { it, false };
		}
	}

	template<typename K, typename V>
	std::pair<iterator, bool> emplace(K&& key_args, V&& val_args)
	{
		auto key_tuple = std::forward_as_tuple(std::forward<K>(key_args));
		auto val_tuple = std::forward_as_tuple(std::forward<V>(val_args));
		return emplace(std::piecewise_construct, key_tuple, val_tuple);
	}

	template<typename Iterator>
	size_t insert(Iterator begin, Iterator end)
	{
		size_t newly_inserted = 0;
		for(; begin != end; ++begin)
		{
			const auto result = insert(begin->first, begin->second);
			newly_inserted += result.second;
		}

		return newly_inserted;
	}

	template<typename Range>
	size_t insert(Range&& range)
	{
		size_t newly_inserted = 0;
		for (auto& pair : range)
		{
			const auto result = emplace(std::move(pair.first), std::move(pair.second));
			newly_inserted += result.second;
		}

		return newly_inserted;
	}

	std::pair<iterator,bool> insert(const value_type& val) { return insert(val.first, val.second); }
	size_t insert(std::initializer_list<mapped_type> list) { return insert(list.begin(), list.end()); }

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
				m_Cache.erase(it);
				m_Stats.evict(it->first, it->second);
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
