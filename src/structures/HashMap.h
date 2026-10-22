#ifndef HASHMAP_H
#define HASHMAP_H

#include "Vector.h"

#include <cstddef>
#include <functional>
#include <utility>

using namespace std;

template <typename T>
using Hash = hash<T>;

// Bảng băm địa chỉ mở (open addressing, dò tuyến tính).
//  - Tự mở rộng (rehash) khi hệ số tải > 70%, nên không bao giờ "đầy".
//  - remove() dùng ô "đã xóa" (tombstone) nên không làm đứt chuỗi dò.
//  - Độ phức tạp trung bình: insert/get/remove O(1).
template <typename K, typename V>
class HashMap {
public:
    enum class State : unsigned char { Empty, Used, Deleted };

    struct Entry {
        K key{};
        V value{};
        State state = State::Empty;
    };

    explicit HashMap(size_t bucketCount = 16)
        : buckets_(bucketCount < 4 ? 4 : bucketCount) {}

    // Thêm mới hoặc cập nhật giá trị của khóa đã có.
    void insert(const K& key, const V& value) {
        if ((used_ + deleted_ + 1) * 10 > buckets_.size() * 7) {
            // Nhiều phần tử thật -> nhân đôi; chủ yếu là ô đã xóa -> giữ nguyên kích thước.
            rehash(used_ * 2 >= buckets_.size() ? buckets_.size() * 2 : buckets_.size());
        }
        size_t cap = buckets_.size();
        size_t index = hash(key) % cap;
        bool hasTomb = false;
        size_t tomb = 0;
        for (size_t i = 0; i < cap; ++i) {
            size_t pos = (index + i) % cap;
            Entry& e = buckets_[pos];
            if (e.state == State::Empty) {
                size_t target = hasTomb ? tomb : pos;
                Entry& t = buckets_[target];
                if (t.state == State::Deleted) --deleted_;
                t.key = key;
                t.value = value;
                t.state = State::Used;
                ++used_;
                return;
            }
            if (e.state == State::Deleted) {
                if (!hasTomb) {
                    hasTomb = true;
                    tomb = pos;
                }
            } else if (e.key == key) {
                e.value = value;
                return;
            }
        }
        // Không gặp ô trống nào nhưng có ô đã xóa: dùng lại ô đó.
        if (hasTomb) {
            Entry& t = buckets_[tomb];
            t.key = key;
            t.value = value;
            t.state = State::Used;
            --deleted_;
            ++used_;
        }
    }

    bool contains(const K& key) const { return find(key) != nullptr; }

    V* get(const K& key) {
        return const_cast<V*>(find(key));
    }

    const V* get(const K& key) const { return find(key); }

    // Xóa khóa; trả về true nếu khóa tồn tại.
    bool remove(const K& key) {
        size_t cap = buckets_.size();
        size_t index = hash(key) % cap;
        for (size_t i = 0; i < cap; ++i) {
            size_t pos = (index + i) % cap;
            Entry& e = buckets_[pos];
            if (e.state == State::Empty) return false;
            if (e.state == State::Used && e.key == key) {
                e.state = State::Deleted;
                e.value = V{};
                --used_;
                ++deleted_;
                return true;
            }
        }
        return false;
    }

    void clear() {
        for (size_t i = 0; i < buckets_.size(); ++i) {
            buckets_[i].state = State::Empty;
            buckets_[i].value = V{};
        }
        used_ = 0;
        deleted_ = 0;
    }

    size_t size() const { return used_; }
    bool empty() const { return used_ == 0; }
    size_t bucketCount() const { return buckets_.size(); }

private:
    static size_t hash(const K& key) {
        Hash<K> hasher;
        return hasher(key);
    }

    const V* find(const K& key) const {
        size_t cap = buckets_.size();
        size_t index = hash(key) % cap;
        for (size_t i = 0; i < cap; ++i) {
            size_t pos = (index + i) % cap;
            const Entry& e = buckets_[pos];
            if (e.state == State::Empty) return nullptr;
            if (e.state == State::Used && e.key == key) return &e.value;
        }
        return nullptr;
    }

    void rehash(size_t newCapacity) {
        Vector<Entry> old = move(buckets_);
        buckets_ = Vector<Entry>(newCapacity);
        used_ = 0;
        deleted_ = 0;
        for (size_t i = 0; i < old.size(); ++i) {
            if (old[i].state == State::Used) {
                insert(old[i].key, old[i].value);
            }
        }
    }

    Vector<Entry> buckets_;
    size_t used_ = 0;
    size_t deleted_ = 0;
};

#endif
