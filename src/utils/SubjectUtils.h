#pragma once

#include "TextUtils.h"
#include "Vector.h"

#include <regex>
#include <string>

// Business-facing subject identifiers are independent of display labels/grade.
// Legacy titles such as "Toan Lớp 10" and "10 - Toan, Ly" are parsed at
// delimiters; substring matching is deliberately forbidden (Anh != Anh van 2).
namespace SubjectUtils {
inline std::string canonical(std::string value) {
    value = TextUtils::normalize(TextUtils::trim(value));
    if (value == "toan hoc") return "toan";
    if (value == "vat ly" || value == "vat li" || value == "li") return "ly";
    if (value == "hoa hoc") return "hoa";
    if (value == "ngu van") return "van";
    if (value == "tieng anh" || value == "anh van") return "anh";
    if (value == "sinh hoc") return "sinh";
    if (value == "lich su") return "su";
    if (value == "dia ly" || value == "dia li") return "dia";
    return value;
}

inline bool equals(const std::string& a, const std::string& b) {
    return !canonical(a).empty() && canonical(a) == canonical(b);
}

inline bool contains(const Vector<std::string>& subjects, const std::string& requested) {
    for (size_t i = 0; i < subjects.size(); ++i)
        if (equals(subjects[i], requested)) return true;
    return false;
}

inline Vector<std::string> parseClassSubjects(const std::string& title) {
    std::string text = TextUtils::normalize(title);
    // Support the exact grade prefixes/suffixes already stored in legacy files.
    text = std::regex_replace(text, std::regex(R"(^[0-9]{1,2}\s*-\s*)"), "");
    text = std::regex_replace(text, std::regex(R"(\s+(lop|khoi)\s+[0-9]{1,2}$)"), "");
    Vector<std::string> parts = TextUtils::splitTrimmed(text, ',');
    Vector<std::string> normalized;
    for (size_t i = 0; i < parts.size(); ++i) {
        std::string subject = canonical(parts[i]);
        if (!subject.empty() && !contains(normalized, subject)) normalized.push_back(subject);
    }
    return normalized;
}
}
