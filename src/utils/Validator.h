#pragma once

#include <cctype>
#include <string>

using namespace std;

// Các hàm kiểm tra dữ liệu nhập (thuần C++, dùng được cả trong test).
namespace Validator {

inline bool isLeapYear(int y) {
    return (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0);
}

// Ngày dạng dd/MM/yyyy có hợp lệ không (kiểm tra cả số ngày trong tháng, năm nhuận).
inline bool isValidDate(const string& s) {
    if (s.size() != 10 || s[2] != '/' || s[5] != '/') return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    int d = stoi(s.substr(0, 2));
    int m = stoi(s.substr(3, 2));
    int y = stoi(s.substr(6, 4));
    if (y < 1900 || y > 2100 || m < 1 || m > 12 || d < 1) return false;
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = days[m - 1] + ((m == 2 && isLeapYear(y)) ? 1 : 0);
    return d <= maxDay;
}

// So sánh 2 ngày hợp lệ: <0 nếu a trước b, 0 nếu bằng, >0 nếu a sau b.
inline int compareDates(const string& a, const string& b) {
    auto key = [](const string& s) {
        return stoi(s.substr(6, 4)) * 10000 + stoi(s.substr(3, 2)) * 100 + stoi(s.substr(0, 2));
    };
    return key(a) - key(b);
}

// Số điện thoại Việt Nam: 9-11 chữ số, bắt đầu bằng 0 (cho phép khoảng trắng, dấu chấm, gạch).
inline bool isValidPhone(const string& s) {
    string digits;
    for (char c : s) {
        if (isdigit(static_cast<unsigned char>(c))) digits.push_back(c);
        else if (c != ' ' && c != '.' && c != '-') return false;
    }
    return digits.size() >= 9 && digits.size() <= 11 && digits[0] == '0';
}

// Email đơn giản: có đúng 1 '@', phần trước và sau không rỗng, phần sau có dấu '.'.
inline bool isValidEmail(const string& s) {
    if (s.empty()) return true;  // email không bắt buộc
    size_t at = s.find('@');
    if (at == string::npos || at == 0 || s.find('@', at + 1) != string::npos) return false;
    size_t dot = s.find('.', at + 1);
    if (dot == string::npos || dot == at + 1 || dot + 1 >= s.size()) return false;
    return s.find(' ') == string::npos;
}

// Số nguyên không âm (học phí, ngân sách...). Rỗng không hợp lệ.
inline bool isNonNegativeNumber(const string& s) {
    if (s.empty() || s.size() > 12) return false;
    for (char c : s) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

}  // namespace Validator
