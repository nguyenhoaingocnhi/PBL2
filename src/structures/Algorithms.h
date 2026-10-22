#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Vector.h"

#include <cstddef>
#include <utility>

using namespace std;

// Sắp xếp trộn (merge sort) ổn định, độ phức tạp O(n log n).
// comesBefore(a, b) trả về true nếu a phải đứng trước b.
//   Ví dụ giảm dần theo điểm: [](const X& a, const X& b){ return a.score > b.score; }
template <typename T, typename Compare>
void mergeSortRange(Vector<T>& data, Vector<T>& buffer, size_t left, size_t right, Compare& comesBefore) {
    if (right - left < 2) return;
    size_t mid = left + (right - left) / 2;
    mergeSortRange(data, buffer, left, mid, comesBefore);
    mergeSortRange(data, buffer, mid, right, comesBefore);

    size_t i = left, j = mid, k = left;
    while (i < mid && j < right) {
        // Chỉ lấy phần tử bên phải khi nó "đứng trước" thật sự -> giữ tính ổn định.
        if (comesBefore(data[j], data[i])) {
            buffer[k++] = data[j++];
        } else {
            buffer[k++] = data[i++];
        }
    }
    while (i < mid) buffer[k++] = data[i++];
    while (j < right) buffer[k++] = data[j++];
    for (size_t t = left; t < right; ++t) data[t] = buffer[t];
}

template <typename T, typename Compare>
void mergeSort(Vector<T>& data, Compare comesBefore) {
    if (data.size() < 2) return;
    Vector<T> buffer(data.size());
    mergeSortRange(data, buffer, 0, data.size(), comesBefore);
}

#endif
