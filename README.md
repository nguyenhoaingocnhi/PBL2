# PBL2 - TutorFlow: Quản lý và tìm kiếm gia sư

Ứng dụng desktop **C++17 + Qt 6 Widgets + CMake** giúp trung tâm quản lý học sinh, gia sư, lớp học, hợp đồng,
phiếu đăng ký và **tự động ghép gia sư phù hợp** cho từng học sinh. Dữ liệu lưu bằng file TXT.

## Tính năng
- Đăng nhập admin (mật khẩu lưu dạng băm SHA-256 + muối).
- Quản lý (thêm / sửa / xóa / tìm kiếm / lọc) học sinh, gia sư, lớp học, hợp đồng, phiếu đăng ký.
- Bảng điều khiển và thống kê: số học sinh, gia sư, lớp đang học, tỉ lệ ghép.
- **Ghép gia sư thông minh**: chấm điểm 0-100, hiển thị điểm thành phần và lý do ghép; chấp nhận ghép sẽ tự tạo
  lớp học + hợp đồng và cập nhật số học sinh của gia sư.
- Kiểm tra dữ liệu nhập (số điện thoại, email, ngày sinh dd/MM/yyyy, học phí) và chặn trùng mã.

## Cấu trúc
```
src/entities       Person, Student, Tutor, TutoringClass, Contract, Registration
src/structures     Vector, HashMap (open addressing + rehash), Algorithms (merge sort)
src/services       MatchingService (thuật toán ghép)
src/repositories   FileManager (đọc/ghi TXT an toàn, đăng nhập admin)
src/utils          TextUtils (chuẩn hóa tiếng Việt), Validator, IdGenerator, Sha256
src/ui             Qt Widgets: LoginWindow, MainWindow
data               Dữ liệu mẫu .txt
tests              backend_test.cpp (kiểm thử có assert, chạy bằng ctest)
```

## Build và chạy
Yêu cầu: CMake >= 3.16, trình biên dịch C++17, Qt 6 (Widgets).
```bash
cmake -S . -B build
cmake --build build
```
Trên Windows chạy `run_app.bat` (hoặc mở `build/PBL2App.exe`). Ứng dụng tự tìm thư mục `data`.
Nếu CMake không thấy Qt, thêm `-DCMAKE_PREFIX_PATH=<đường dẫn Qt>` (ví dụ `C:/Qt/6.7.0/mingw_64`).

Chạy kiểm thử backend (không cần Qt):
```bash
ctest --test-dir build --output-on-failure
```

**Tài khoản mặc định:** `admin` / `123456` (hãy đổi sau lần đăng nhập đầu; file `data/admin.txt` chỉ lưu bản băm).

## Thuật toán ghép gia sư
Với mỗi gia sư đang nhận học sinh (chưa đủ 8 học sinh) và dạy được ít nhất một môn học sinh cần:

| Thành phần | Tối đa | Cách tính |
|---|---|---|
| Học phí | 45 | tỉ lệ học phí / ngân sách (<= 0.8 được điểm cao nhất); thiếu dữ liệu -> điểm trung tính |
| Kinh nghiệm | 25 | theo số năm (>= 10, >= 5, >= 3, >= 1) |
| Lịch dạy | 20 | 20 x (số buổi trùng / số buổi học sinh rảnh) |
| Khu vực | 10 | trùng khu vực = 10, chứa nhau = 8, khác = 3 |

`điểm = tổng x (0.85 + 0.15 x tỉ lệ môn phủ) + điều chỉnh giới tính (+3 / -8) - số học sinh hiện có`, giới hạn 0-100.
So sánh khu vực, môn, lịch **không phân biệt hoa/thường và dấu** ("Hải Châu" = "hai chau").
Kết quả sắp xếp bằng **merge sort** tự cài (O(n log n), ổn định); cùng điểm thì học phí thấp hơn đứng trước.

## Định dạng dữ liệu (cột phân cách bằng `|`)
- `student.txt` (15 cột): mã, họ tên, SĐT, email, khu vực, giới tính, ngày sinh, khối, môn cần (phẩy), lịch rảnh,
  học lực, giới tính gia sư mong muốn, ngân sách/buổi, ghi chú, đã ghép (true/false)
- `tutor.txt` (16 cột): mã, họ tên, SĐT, email, địa chỉ, giới tính, ngày sinh, môn dạy (phẩy), trình độ, số năm kinh
  nghiệm, mô tả kinh nghiệm, học phí/buổi, lịch rảnh, khu vực dạy (phẩy), đang nhận (true/false), số học sinh hiện có
- `class.txt` (11 cột), `contract.txt` (9 cột), `registration.txt` (7 cột), `admin.txt` (`tên|muối|băm`)

Ngày dùng định dạng `dd/MM/yyyy`. Ký tự `|` và xuống dòng trong nội dung nhập sẽ tự được thay thế khi lưu.
