#pragma once

#include <map>
#include <stdexcept>

#include "Policy/None.h"

template<typename Key, typename Value, typename CachePolicy = NoCachePolicy<Key>>
class Cache
{
private:
	std::map<Key, Value> m_Cache;
	mutable CachePolicy m_CachePolicy;
	const size_t m_MaxSize;
	mutable unsigned int m_HitCount;
	mutable unsigned int m_MissCount;

public:
	using iterator               = typename std::map<Key, Value>::iterator;
	using const_iterator         = typename std::map<Key, Value>::const_iterator;
	using key_type               = typename std::map<Key, Value>::key_type;
	using value_type             = typename std::map<Key, Value>::value_type;
	using mapped_type            = typename std::map<Key, Value>::mapped_type;
	using reference              = typename std::map<Key, Value>::reference;
	using const_reference        = typename std::map<Key, Value>::const_reference;
	using reverse_iterator       = typename std::map<Key, Value>::reverse_iterator;
	using const_reverse_iterator = typename std::map<Key, Value>::const_reverse_iterator;
	using difference_type        = typename std::map<Key, Value>::difference_type;
	using size_type              = typename std::map<Key, Value>::size_type;

	Cache(const size_t maxSize, const CachePolicy& policy = CachePolicy())
		: m_CachePolicy(policy), m_MaxSize(maxSize), m_HitCount(0), m_MissCount(0)
	{
		if(m_MaxSize == 0) throw std::invalid_argument("Invalid cache size!");
	}

	~Cache() { clear(); }

	iterator       begin()       noexcept { return m_Cache.begin(); }
	const_iterator begin() const noexcept { return m_Cache.begin(); }
	iterator       end  ()       noexcept { return m_Cache.end  (); }
	const_iterator end  () const noexcept { return m_Cache.end  (); }

	reverse_iterator       rbegin()       noexcept { return m_Cache.rbegin(); }
	const_reverse_iterator rbegin() const noexcept { return m_Cache.rbegin(); }
	reverse_iterator       rend  ()       noexcept { return m_Cache.rend  (); }
	const_reverse_iterator rend  () const noexcept { return m_Cache.rend  (); }

	const_iterator cbegin() const noexcept { return m_Cache.cbegin(); }
	const_iterator cend  () const noexcept { return m_Cache.cend  (); }

	const_reverse_iterator crbegin() const noexcept { return m_Cache.crbegin(); }
	const_reverse_iterator crend  () const noexcept { return m_Cache.crend  (); }

	bool empty() { return size() == static_cast<size_type>(0); }

	size_type size() { return m_Cache.size(); }
	size_type max_size() { return m_MaxSize; }

		  mapped_type& at(const key_type& key)       { m_HitCount++; return m_Cache.at(key); }
	const mapped_type& at(const key_type& key) const { m_HitCount++; return m_Cache.at(std::forward(key)); }

	size_type erase(const key_type& key)
	{
		auto it = find_key(key);

		//onErase(it->first, it->second);
		m_CachePolicy.erase(key);
		m_Cache.erase(it);

		return 1ULL;
	}

	void insert(const key_type& key, const mapped_type& value)
	{
		auto it = find_key(key);

		if(it == end())
		{
			if(size() + 1 > m_MaxSize)
			{
				auto replaced_key = m_CachePolicy.replace_candidate();
				it = find_key(replaced_key);

				//onErase(it->first, it->second);
				m_CachePolicy.erase(replaced_key);
				m_Cache.erase(it);
			}

			m_CachePolicy.insert(key);
			m_Cache.emplace(std::make_pair(key, value));
		}
		else
		{
			m_CachePolicy.touch(key);
			m_Cache[key] = value;
		}
	}

	void clear() noexcept
	{
		for(auto it = begin(); it != end(); it++)
		{
			//onErase(it->first, it->second);
			m_CachePolicy.erase(it->first);
		}

		m_Cache.clear();
	}

	void flush() noexcept { clear(); }
	void flush(const key_type& key) noexcept { erase(key); }

	iterator       find(const key_type& key)       { return find_key(key); }
	const_iterator find(const key_type& key) const { return find_key(key); }

	bool has(const key_type& key) const { return find_key(key) != end(); }

	size_type count(const key_type& key) const { return m_Cache.count(key); }

	size_type hit_count () const noexcept { return m_HitCount; }
	size_type miss_count() const noexcept { return m_MissCount; }

	float hit_ratio () const noexcept { return static_cast<float>(m_HitCount ) / (static_cast<float>(m_HitCount + m_MissCount)); }
	float miss_ratio() const noexcept { return static_cast<float>(m_MissCount) / (static_cast<float>(m_HitCount + m_MissCount)); }

private:
	iterator find_key(const key_type& key)
	{
		auto it = m_Cache.find(key);

		return (it != end() ?
			(m_HitCount++, m_CachePolicy.touch(key), it) :
			(m_MissCount++, it));
	}

	const_iterator find_key(const key_type& key) const
	{
		auto it = m_Cache.find(key);

		return (it != end() ?
			(m_HitCount++, m_CachePolicy.touch(key), it) :
			(m_MissCount++, it));
	}
};
