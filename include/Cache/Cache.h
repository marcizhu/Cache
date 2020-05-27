#pragma once

#include <map>

#include "CachePolicy.h"

template<typename Key, typename Value, typename CachePolicy = NoCachePolicy()>
class Cache
{
public:
	using iterator               = typename std::map<Key, Value>::iterator;
	using const_iterator         = typename std::map<Key, Value>::const_iterator;
	using key_type               = typename std::map<Key, Value>::key_type;
	using value_type             = typename std::map<Key, Value>::value_type;
	using reference              = typename std::map<Key, Value>::reference;
	using const_reference        = typename std::map<Key, Value>::const_reference;
	using reverse_iterator       = typename std::map<Key, Value>::reverse_iterator;
	using const_reverse_iterator = typename std::map<Key, Value>::const_reverse_iterator;
	using difference_type        = typename std::map<Key, Value>::difference_type;
	using size_type              = typename std::map<Key, Value>::size_type;

	constexpr Cache(const size_type maxSize) : m_maxSize(maxSize) {}
	~Cache() = default;

	iterator       begin()       noexcept { return m_cache.begin(); }
	const_iterator begin() const noexcept { return m_cache.begin(); }
	iterator       end  ()       noexcept { return m_cache.end  (); }
	const_iterator end  () const noexcept { return m_cache.end  (); }

	reverse_iterator       rbegin()       noexcept { return m_cache.rbegin(); }
	const_reverse_iterator rbegin() const noexcept { return m_cache.rbegin(); }
	reverse_iterator       rend  ()       noexcept { return m_cache.rend  (); }
	const_reverse_iterator rend  () const noexcept { return m_cache.rend  (); }

	const_iterator cbegin() const noexcept { return m_cache.cbegin(); }
	const_iterator cend  () const noexcept { return m_cache.cend  (); }

	const_reverse_iterator crbegin() const noexcept { return m_cache.crbegin(); }
	const_reverse_iterator crend  () const noexcept { return m_cache.crend  (); }

	bool empty() { return size() == static_cast<size_type>(0); }

	size_type size() { return m_cache.size(); }
	size_type max_size() { return max_size; }

	// operator[]
	// at

	iterator  erase(const_iterator position);
	size_type erase(const key_type& k);
	iterator  erase(const_iterator first, const_iterator last);

	bool insert(const key_type& key, const value_type& val);

	void clear() noexcept { m_cache.clear(); }
	void flush() noexcept { m_cache.clear(); }

	iterator       find(const key_type& key)       { m_cache.find(key); }
	const_iterator find(const key_type& key) const { m_cache.find(key); }

	bool has(const key_type& key) const { return find(key) != end(); }

	size_type count (const key_type& key) const { return m_cache.count(key); }

private:
	std::map<Key, Value> m_cache;
	const size_type m_maxSize;
	unsigned int m_hitCount = 0;
	unsigned int m_missCount = 0;

};
