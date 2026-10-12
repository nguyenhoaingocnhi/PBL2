#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <initializer_list>

using namespace std;

template <typename T>
class Vector {
public:
    Vector() = default;

    explicit Vector(size_t initialSize, const T& defaultValue = T()) {
        size_t cap = initialSize == 0 ? 4 : initialSize;
        data_ = new T[cap];
        capacity_ = cap;
        size_ = initialSize;
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = defaultValue;
        }
    }

    Vector(initializer_list<T> list) {
        size_t cap = list.size() == 0 ? 4 : list.size() * 2;
        data_ = new T[cap];
        capacity_ = cap;
        size_ = 0;
        for (const auto& item : list) {
            data_[size_++] = item;
        }
    }

    Vector(const Vector& other) {
        copyFrom(other);
    }

    Vector(Vector&& other) noexcept {
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~Vector() {
        clear();
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            clear();
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            resize(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            resize(capacity_ == 0 ? 4 : capacity_ * 2);
        }
        data_[size_++] = move(value);
    }

    void pop_back() {
        if (empty()) {
            return;
        }
        --size_;
    }

    void erase(size_t index) {
        if (index >= size_) {
            throw out_of_range("Vector index out of range");
        }
        for (size_t i = index; i + 1 < size_; ++i) {
            data_[i] = move(data_[i + 1]);
        }
        --size_;
    }

    T* erase(T* it) {
        if (it < begin() || it >= end()) {
            throw out_of_range("Vector iterator out of range");
        }
        size_t index = it - begin();
        erase(index);
        return begin() + index;
    }

    T& operator[](size_t index) {
        if (index >= size_) {
            throw out_of_range("Vector index out of range");
        }
        return data_[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw out_of_range("Vector index out of range");
        }
        return data_[index];
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear() {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    void resize(size_t newCapacity) {
        T* newData = new T[newCapacity];
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = move(data_[i]);
        }
        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

    void copyFrom(const Vector& other) {
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = nullptr;
        if (capacity_ > 0) {
            data_ = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
    }

    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

#endif
