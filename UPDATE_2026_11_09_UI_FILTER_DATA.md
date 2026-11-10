# TutorFlow – Bản cập nhật bộ lọc / dữ liệu / màn hình đăng nhập

1. Đăng nhập: bỏ lựa chọn tài khoản giáo dục, bỏ link tạo tài khoản.
2. Dữ liệu: giữ 22 dòng lớp; lưu bản gốc 27 dòng và 5 dòng trùng ở `data/archive/`;
   không tắt cảnh báo lỗi nếu dữ liệu về sau lại không hợp lệ.
3. Header: bỏ ô "Tìm kiếm hệ thống".
4. Bộ lọc:
   - Học sinh: ID, họ và tên, SĐT, môn học, địa chỉ.
   - Gia sư: ID, họ và tên, SĐT, môn dạy, số năm kinh nghiệm, địa chỉ/khu vực nhận lớp.
   - Lớp học: ID, học sinh, gia sư, môn học, lịch học, trạng thái.
   - Dùng nút **Tìm kiếm** hoặc Enter để lọc; **Xóa** để khôi phục toàn bộ.
   - Mọi bộ lọc có thể kết hợp; tìm chữ không phân biệt dấu tiếng Việt.
5. Ngày kết thúc: đã gỡ khỏi form, entity, kiểm tra nghiệp vụ, định dạng file lớp.
   Class file mới 10 cột, phiên bản cũ 11 cột vẫn đọc được.

## Kiểm tra dữ liệu của bạn

Nếu chỉ muốn chạy bộ dữ liệu đã làm sạch, giải nén ZIP vào thư mục mới và build từ đầu.
Nếu cần giữ dữ liệu cá nhân từ lần chạy trước: sao lưu `data/` cũ rồi trộn theo từng bản ghi
sau khi xác minh, không ghi đè file mới một cách mù quáng.

Các file trong `data/archive` là bản sao phục vụ đối chiếu. Chúng không được phần mềm
nạp tự động và không được tính vào thống kê hiện tại.

## Kiểm thử

`cmake -S . -B build -DBUILD_QT_APP=OFF`
`cmake --build build`
`ctest --test-dir build --output-on-failure`

Để kiểm tra trực quan form và bố cục hãy build bằng Qt 6 Widgets.
