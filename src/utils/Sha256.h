#pragma once

#include <string>

using namespace std;

namespace Sha256 {

// Trả về chuỗi hex 64 ký tự của SHA-256(input).
string hashHex(const string& input);

// Băm mật khẩu có muối: lặp nhiều vòng để làm chậm dò mật khẩu.
string hashPassword(const string& salt, const string& password);

// Sinh muối ngẫu nhiên (32 ký tự hex).
string generateSalt();

}  // namespace Sha256
