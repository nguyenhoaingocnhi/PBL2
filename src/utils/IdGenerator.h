#pragma once

#include "Vector.h"

#include <cctype>
#include <cstddef>
#include <string>

using namespace std;

// Sinh mã mới dạng <PREFIX><số 3 chữ số>, ví dụ SV004.
// Lấy số lớn nhất đang có rồi cộng 1 nên không bao giờ trùng, kể cả khi đã xóa bớt phần tử.
namespace IdGenerator {

inline string next(const string& prefix, const Vector<string>& existingIds, size_t width = 3) {
    long long maxNumber = 0;
    for (size_t i = 0; i < existingIds.size(); ++i) {
        const string& id = existingIds[i];
        if (id.size() <= prefix.size() || id.compare(0, prefix.size(), prefix) != 0) continue;
        string digits = id.substr(prefix.size());
        bool allDigits = !digits.empty() && digits.size() <= 9;
        for (char c : digits) {
            if (!isdigit(static_cast<unsigned char>(c))) {
                allDigits = false;
                break;
            }
        }
        if (allDigits) {
            long long n = stoll(digits);
            if (n > maxNumber) maxNumber = n;
        }
    }
    string number = to_string(maxNumber + 1);
    while (number.size() < width) number = "0" + number;
    return prefix + number;
}

// Tiện ích cho Vector<T*>: getId là hàm trả về mã của từng phần tử.
template <typename T, typename Getter>
string nextFor(const string& prefix, Vector<T*>& items, Getter getId, size_t width = 3) {
    Vector<string> ids;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i] != nullptr) ids.push_back(getId(items[i]));
    }
    return next(prefix, ids, width);
}

}  // namespace IdGenerator
