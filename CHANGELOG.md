# Các thay đổi so với bản gốc

## Lỗi nghiêm trọng đã sửa
- **Mất dữ liệu khi lưu:** `Student::toString()` thiếu cột `grade`, `Tutor::toString()` thiếu cột `experienceDetails`
  nên file ghi ra thiếu cột và lần đọc sau bỏ qua toàn bộ dòng (lưu 1 lần -> mở lại không còn học sinh/gia sư nào).
- **HashMap:** thêm quá 16 phần tử là ném "HashMap is full"; xóa phần tử làm đứt chuỗi dò (`get` không tìm thấy
  phần tử khác). Viết lại: tự rehash, ô "đã xóa" (tombstone), có phiên bản const.
- **Ghép gia sư:** ngân sách = 0 được 45/45 điểm giá; so sánh chuỗi chính xác nên khác dấu/hoa thường là lệch;
  sắp xếp O(n^2); lý do ghép trên giao diện là câu cố định, luôn "khớp" kể cả khi không khớp; điểm bị ép tối thiểu 20%.
- **Gia sư mới tạo** bị gán sẵn 5 học sinh (`5.0`) thay vì 0.
- **Sinh mã:** `"SV00" + (size+1)` trùng mã sau khi xóa và sai định dạng từ phần tử 10; mã lớp/hợp đồng khi ghép
  cũng vậy. Dùng `IdGenerator` (số lớn nhất + 1).
- **Mật khẩu admin** lưu thô -> lưu băm SHA-256 + muối (5000 vòng); tài khoản cũ tự nâng cấp khi đăng nhập thành công.
- **FileManager:** `stoi` ném exception với dữ liệu hỏng -> dùng chuyển đổi an toàn; ghi file không an toàn ->
  ghi file tạm rồi đổi tên; hỗ trợ file có BOM; đếm số dòng bị bỏ qua.

## Cải tiến
- Ghép gia sư: thêm phủ môn, giới tính mong muốn, giới hạn/phạt số học sinh hiện có, danh sách lý do, `matchTopN`.
- Giao diện: kiểm tra SĐT / email / ngày sinh / học phí / ngân sách, chặn trùng mã, hiển thị lý do ghép thật,
  gia sư nhận nhiều học sinh (chỉ đóng nhận khi đủ 8).
- `main.cpp` tự tìm thư mục `data`; `run_app.bat` báo lỗi rõ khi chưa build.
- Kiểm thử có assert (114 kiểm tra), chạy qua `ctest`; đã chạy sạch với AddressSanitizer + UBSan.

## Chưa làm
- Tách `MainWindow.cpp` (3.300+ dòng) thành các lớp trang riêng.

## UI refresh
- Cột bảng tự co vừa nội dung (hàm `fitColumns`), tên không còn bị cắt chữ; hàng cao hơn, header gọn hơn.
- Thêm biểu đồ vẽ bằng QPainter: donut (ghép học sinh, trạng thái lớp), cột (khối lớp, gia sư theo môn), đường (lớp mới theo tháng).
- Bổ sung định nghĩa 5 hàm refresh* bị thiếu (số liệu dashboard/thống kê/tóm tắt trước đây luôn là 0 và gây lỗi link).
- Giao diện 5 trang quản lý (học sinh, gia sư, lớp học, hợp đồng, đăng ký): banner gradient riêng từng trang, ô tóm tắt có màu nhấn, bảng có hàng xen kẽ, hiệu ứng hover/chọn và viền tiêu đề cột theo màu trang.
- Thêm ô tóm tắt cho trang Học sinh và Đăng ký môn.
- Giao diện theo mẫu mới: sidebar xanh navy có thẻ quảng bá + minh họa sách/mũ tốt nghiệp, thanh tìm kiếm trắng bo tròn, thanh chào mừng kèm 4 thẻ thống kê, banner trang sáng có minh họa.
- 5 trang quản lý: tab trạng thái có số lượng (đồng bộ với bộ lọc), tag môn học nhiều màu, lịch rảnh dạng chip xanh, avatar màu theo tên kèm SĐT, nút thao tác tròn, phân trang thật (5/10/20/50 dòng mỗi trang).
