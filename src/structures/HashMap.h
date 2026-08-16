#ifndef HASHMAP_H
#define HASHMAP_H

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename K, typename V>
class HashMap {
public:
    struct Entry {
        K key;
        V value;
        bool used = false;
    };

    HashMap(std::size_t bucketCount = 16)
        : buckets_(bucketCount) {}

    void insert(const K& key, const V& value) {
        auto index = hash(key) % buckets_.size();
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            std::size_t pos = (index + i) % buckets_.size();
            if (!buckets_[pos].used || buckets_[pos].key == key) {
                buckets_[pos].key = key;
                buckets_[pos].value = value;
                buckets_[pos].used = true;
                return;
            }
        }
        throw std::runtime_error("HashMap is full");
    }

    bool contains(const K& key) const {
        return get(key) != nullptr;
    }

    V* get(const K& key) {
        auto index = hash(key) % buckets_.size();
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            std::size_t pos = (index + i) % buckets_.size();
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
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            std::size_t pos = (index + i) % buckets_.size();
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
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            std::size_t pos = (index + i) % buckets_.size();
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

    std::size_t size() const {
        std::size_t count = 0;
        for (const auto& entry : buckets_) {
            if (entry.used) {
                ++count;
            }
        }
        return count;
    }

private:
    std::size_t hash(const K& key) const {
        std::hash<K> hasher;
        return hasher(key);
    }

    std::vector<Entry> buckets_;
};

#endif
