# TutorFlow — Cập nhật PBL2

## Những phần đã sửa
1. `src/ui/LoginWindow.cpp`, `LoginWindow.h`: thiết kế cửa sổ đăng nhập hai cột theo ảnh tham khảo; báo lỗi đỏ trực tiếp trong form; xác thực theo `FileManager`.
2. `src/ui/MainWindow.cpp`: bổ sung thông báo lỗi màu đỏ dưới ô nhập trong các biểu mẫu có dùng `FormCheck` và `attachLive`; khi thêm mới **học sinh** hoặc **gia sư**, lưu thành công sẽ hỏi "Bạn có muốn nhập tiếp không?"; trả lời Không sẽ chuyển tới danh sách và chọn bản ghi mới nhất.
3. `src/main.cpp`: điều chỉnh kích thước cửa sổ đăng nhập.

## Biên dịch/chạy (Windows)
Sử dụng Qt6 Widgets và CMake, từ thư mục `PBL2`:
```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build build --config Release
```
Đổi `CMAKE_PREFIX_PATH` thành đường dẫn Qt6 đã cài trên máy; nếu dùng MSVC chọn kit tương ứng. Có thể dùng `run_app.bat` theo cấu hình sẵn của dự án.

## Kiểm tra thủ công cần thực hiện
- Đăng nhập bỏ trống tài khoản / mật khẩu, kiểm tra dòng lỗi đỏ dưới đúng trường.
- Đăng nhập sai mật khẩu, kiểm tra dòng lỗi trong form; đăng nhập tài khoản hợp lệ kiểm tra chuyển trang.
- Nhập sai số điện thoại, ngày sinh, mã trùng: kiểm tra cảnh báo ngay dưới ô nhập và không lưu bản ghi.
- Thêm học sinh, chọn Có và thêm tiếp; sau lần cuối chọn Không, kiểm tra bảng chọn bản ghi vừa thêm; thử tương tự với gia sư.
- Kiểm tra thêm/cập nhật lớp học trên máy có Qt6; đối với lớp học chưa triển khai hộp thoại hỏi nhập tiếp.

## Giới hạn kiểm thử
Môi trường thực hiện không cài Qt6: đã biên dịch và chạy thành công các backend tests, **chưa** xác nhận được biên dịch và hoạt động của màn hình desktop. Cần kiểm tra bằng Qt Creator trên Windows trước khi nộp báo cáo.

## Gợi ý cải thiện tiếp
- Tách `MainWindow.cpp` (~4.900 dòng) thành các màn hình/quản lý riêng, tránh trùng code form.
- Tách phần kiểm tra dữ liệu thành class `FormValidator` để kiểm thử đơn vị.
- Thêm nhật ký hoạt động, kiểm tra lưu file thất bại, backup dữ liệu và kiểm tra toàn vẹn các bản ghi.
- Thêm chế độ bật/tắt hiển thị mật khẩu; tài khoản phân quyền và cơ chế đặt lại mật khẩu thật.
- Cải thiện phần hỗ trợ bàn phím, tự động focus lỗi đầu tiên và khả năng hiển thị trên màn hình nhỏ.
