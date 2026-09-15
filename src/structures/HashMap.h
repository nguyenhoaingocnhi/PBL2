#ifndef HASHMAP_H
#define HASHMAP_H

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
using Hash = hash<T>;

template <typename K, typename V>
class HashMap {
public:
    struct Entry {
        K key;
        V value;
        bool used = false;
    };

    HashMap(size_t bucketCount = 16)
        : buckets_(bucketCount) {}

    void insert(const K& key, const V& value) {
        auto index = hash(key) % buckets_.size();
        for (size_t i = 0; i < buckets_.size(); ++i) {
            size_t pos = (index + i) % buckets_.size();
            if (!buckets_[pos].used || buckets_[pos].key == key) {
                buckets_[pos].key = key;
                buckets_[pos].value = value;
                buckets_[pos].used = true;
                return;
            }
        }
        throw runtime_error("HashMap is full");
    }

    bool contains(const K& key) const {
        return get(key) != nullptr;
    }

    V* get(const K& key) {
        auto index = hash(key) % buckets_.size();
        for (size_t i = 0; i < buckets_.size(); ++i) {
            size_t pos = (index + i) % buckets_.size();
            if (!buckets_[pos].used) {
                return nullptr;
            }
            if (buckets_[pos].key == key) {
                return &buckets_[pos].value;
            }
        }
        return nullptr;
    }

    const V* get(const K& key) const {
        auto index = hash(key) % buckets_.size();
        for (size_t i = 0; i < buckets_.size(); ++i) {
            size_t pos = (index + i) % buckets_.size();
            if (!buckets_[pos].used) {
                return nullptr;
            }
            if (buckets_[pos].key == key) {
                return &buckets_[pos].value;
            }
        }
        return nullptr;
    }

    void remove(const K& key) {
        auto index = hash(key) % buckets_.size();
        for (size_t i = 0; i < buckets_.size(); ++i) {
            size_t pos = (index + i) % buckets_.size();
            if (!buckets_[pos].used) {
                return;
            }
            if (buckets_[pos].key == key) {
                buckets_[pos].used = false;
                return;
            }
        }
    }

    void clear() {
        for (auto& entry : buckets_) {
            entry.used = false;
        }
    }

    size_t size() const {
        size_t count = 0;
        for (const auto& entry : buckets_) {
            if (entry.used) {
                ++count;
            }
        }
        return count;
    }

private:
    size_t hash(const K& key) const {
        Hash<K> hasher;
        return hasher(key);
    }

    vector<Entry> buckets_;
};

#endif
