#pragma once

#include <set>

template<typename Key>
class NoCachePolicy
{
public:
	NoCachePolicy() = default;
	~NoCachePolicy() = default;

	void insert(const Key& key) { key_storage.emplace(key); }
	void touch (const Key& key) { (void)key; }
	void erase (const Key& key) { key_storage.erase(key); }

	const Key& replace_candidate() const { return *key_storage.cbegin(); }

private:
	std::set<Key> key_storage;
};
