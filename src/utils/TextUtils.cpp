#include "TextUtils.h"

#include <cctype>
#include <cstdint>
#include <exception>

namespace TextUtils {

namespace {

struct AccentGroup {
    const char32_t* lower;
    const char32_t* upper;
    char base;
};

// Mỗi nhóm: các dạng có dấu (thường / hoa) của một chữ cái gốc.
const AccentGroup kGroups[] = {
    {U"àáạảãâầấậẩẫăằắặẳẵ", U"ÀÁẠẢÃÂẦẤẬẨẪĂẰẮẶẲẴ", 'a'},
    {U"èéẹẻẽêềếệểễ",       U"ÈÉẸẺẼÊỀẾỆỂỄ",       'e'},
    {U"ìíịỉĩ",             U"ÌÍỊỈĨ",             'i'},
    {U"òóọỏõôồốộổỗơờớợởỡ", U"ÒÓỌỎÕÔỒỐỘỔỖƠỜỚỢỞỠ", 'o'},
    {U"ùúụủũưừứựửữ",       U"ÙÚỤỦŨƯỪỨỰỬỮ",       'u'},
    {U"ỳýỵỷỹ",             U"ỲÝỴỶỸ",             'y'},
    {U"đ",                 U"Đ",                 'd'},
};

// Giải mã 1 ký tự UTF-8 tại vị trí i; trả về số byte đã đọc (>=1).
size_t decodeUtf8(const string& s, size_t i, char32_t& cp) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    size_t len = 1;
    if (c < 0x80) {
        cp = c;
    } else if ((c >> 5) == 0x6) {
        len = 2;
        cp = c & 0x1F;
    } else if ((c >> 4) == 0xE) {
        len = 3;
        cp = c & 0x0F;
    } else if ((c >> 3) == 0x1E) {
        len = 4;
        cp = c & 0x07;
    } else {
        cp = c;
        return 1;
    }
    if (i + len > s.size()) {
        cp = c;
        return 1;
    }
    for (size_t k = 1; k < len; ++k) {
        unsigned char cc = static_cast<unsigned char>(s[i + k]);
        if ((cc >> 6) != 0x2) {
            cp = c;
            return 1;
        }
        cp = (cp << 6) | (cc & 0x3F);
    }
    return len;
}

// Trả về chữ cái gốc ASCII nếu cp là chữ có dấu tiếng Việt, ngược lại 0.
char baseLetter(char32_t cp) {
    for (const auto& g : kGroups) {
        for (const char32_t* p = g.lower; *p; ++p) {
            if (*p == cp) return g.base;
        }
        for (const char32_t* p = g.upper; *p; ++p) {
            if (*p == cp) return g.base;
        }
    }
    return 0;
}

}  // namespace

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \r\n\t");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \r\n\t");
    return s.substr(start, end - start + 1);
}

string normalize(const string& s) {
    string out;
    out.reserve(s.size());
    bool pendingSpace = false;
    size_t i = 0;
    while (i < s.size()) {
        char32_t cp = 0;
        size_t len = decodeUtf8(s, i, cp);

        // Dấu kết hợp (dạng NFD) -> bỏ qua.
        if (cp >= 0x0300 && cp <= 0x036F) {
            i += len;
            continue;
        }

        char ch = 0;
        if (cp < 0x80) {
            unsigned char u = static_cast<unsigned char>(cp);
            if (isspace(u)) {
                pendingSpace = !out.empty();
                i += len;
                continue;
            }
            ch = static_cast<char>(tolower(u));
        } else {
            ch = baseLetter(cp);
        }

        if (pendingSpace) {
            out.push_back(' ');
            pendingSpace = false;
        }
        if (ch != 0) {
            out.push_back(ch);
        } else {
            out.append(s, i, len);  // ký tự khác: giữ nguyên
        }
        i += len;
    }
    return out;
}

Vector<string> splitTrimmed(const string& text, char delimiter) {
    Vector<string> result;
    string current;
    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == delimiter) {
            string t = trim(current);
            if (!t.empty()) result.push_back(t);
            current.clear();
        } else {
            current.push_back(text[i]);
        }
    }
    return result;
}

Vector<string> splitNormalized(const string& text, char delimiter) {
    Vector<string> result;
    Vector<string> parts = splitTrimmed(text, delimiter);
    for (size_t i = 0; i < parts.size(); ++i) {
        string n = normalize(parts[i]);
        if (!n.empty()) result.push_back(n);
    }
    return result;
}

string cleanField(const string& s) {
    string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i] == '|') out[i] = '/';
        else if (out[i] == '\n' || out[i] == '\r') out[i] = ' ';
    }
    return out;
}

long long toLongLong(const string& s, long long defaultValue) {
    try {
        size_t pos = 0;
        string t = trim(s);
        if (t.empty()) return defaultValue;
        long long v = stoll(t, &pos);
        return pos == t.size() ? v : defaultValue;
    } catch (const exception&) {
        return defaultValue;
    }
}

int toInt(const string& s, int defaultValue) {
    long long v = toLongLong(s, defaultValue);
    if (v > 2147483647LL || v < -2147483647LL) return defaultValue;
    return static_cast<int>(v);
}

}  // namespace TextUtils
