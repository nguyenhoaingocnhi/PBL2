#pragma once

#include "TextUtils.h"
#include "Vector.h"
#include <string>

// Existing input is day-based (T2..T7, CN), not a clock-time calendar.
// Prevent double booking on the same DAY conservatively. A future time-slot
// feature must add explicit start/end times rather than infer them.
namespace ScheduleUtils {
inline std::string canonical(const std::string& slot) {
    std::string s = TextUtils::normalize(TextUtils::trim(slot));
    if (s == "chu nhat" || s == "t8") return "cn";
    return s;
}
inline bool validDay(const std::string& slot) {
    const std::string s = canonical(slot);
    return s == "t2" || s == "t3" || s == "t4" || s == "t5" ||
           s == "t6" || s == "t7" || s == "cn";
}
inline bool contains(const Vector<std::string>& days, const std::string& slot) {
    for (size_t i = 0; i < days.size(); ++i)
        if (days[i] == canonical(slot)) return true;
    return false;
}
inline Vector<std::string> parse(const std::string& text) {
    Vector<std::string> result;
    auto parts = TextUtils::splitTrimmed(text, ',');
    for (size_t i = 0; i < parts.size(); ++i) {
        auto s = canonical(parts[i]);
        if (!s.empty() && !contains(result, s)) result.push_back(s);
    }
    return result;
}
inline bool valid(const std::string& value, bool mayBeEmpty = true) {
    if (TextUtils::trim(value).empty()) return mayBeEmpty;
    auto days = parse(value);
    if (days.empty()) return false;
    for (size_t i = 0; i < days.size(); ++i) if (!validDay(days[i])) return false;
    // Empty segments such as "T2,,T3" must not silently pass.
    auto raw = TextUtils::splitTrimmed(value, ',');
    size_t commaCount = 0;
    for (char ch : value) if (ch == ',') ++commaCount;
    return raw.size() == commaCount + 1 && days.size() == raw.size();
}
inline bool intersects(const std::string& left, const std::string& right) {
    const auto a = parse(left);
    const auto b = parse(right);
    for (size_t i = 0; i < a.size(); ++i) if (contains(b, a[i])) return true;
    return false;
}
inline bool isSubset(const std::string& requested, const std::string& availability) {
    const auto required = parse(requested);
    const auto allowed = parse(availability);
    if (required.empty() || allowed.empty()) return false;
    for (size_t i = 0; i < required.size(); ++i)
        if (!contains(allowed, required[i])) return false;
    return true;
}
}
