#pragma once

#include "Vector.h"

#include <string>

using namespace std;

// Các hàm xử lý chuỗi dùng chung (không phụ thuộc Qt).
namespace TextUtils {

// Xóa khoảng trắng đầu/cuối.
string trim(const string& s);

// Chuẩn hóa để so sánh: chữ thường, bỏ dấu tiếng Việt, gộp khoảng trắng.
// Ví dụ: "  Hải  Châu " -> "hai chau", "ĐÀ NẴNG" -> "da nang".
string normalize(const string& s);

// Tách chuỗi theo ký tự phân cách; mỗi phần tử được trim, bỏ phần tử rỗng.
Vector<string> splitTrimmed(const string& text, char delimiter);

// Tách rồi normalize từng phần tử (dùng cho môn học, khu vực, lịch).
Vector<string> splitNormalized(const string& text, char delimiter);

// Làm sạch một trường trước khi ghi vào file TXT:
// ký tự '|' và xuống dòng sẽ phá vỡ định dạng nên bị thay thế.
string cleanField(const string& s);

// Chuyển chuỗi sang số, trả về giá trị mặc định nếu lỗi (không ném exception).
long long toLongLong(const string& s, long long defaultValue = 0);
int toInt(const string& s, int defaultValue = 0);

}  // namespace TextUtils
