## Cập nhật 09/11/2026 — Danh sách chọn gia sư và ngày sinh bắt buộc

- Trang ghép hiển thị tất cả gia sư đủ điều kiện trong bảng xếp hạng điểm giảm dần; có thể chọn TOP 2, TOP 3... thay vì bị buộc chọn TOP 1.
- Chọn dòng trong bảng tự cập nhật thông tin gia sư, đồ thị độ phù hợp, các điểm thành phần và lý do đề xuất. Không tự động tạo lớp trước khi xác nhận.
- Xóa danh sách/lựa chọn cũ khi đổi học sinh, đổi môn, tìm lại hoặc refresh dữ liệu.
- Ô ngày sinh trong form thêm học sinh/gia sư không nhập sẵn dữ liệu ví dụ; chỉ để placeholder `Vd: 25/12/2008 (dd/MM/yyyy)`.
- Form thêm/sửa dùng chung bộ nhập có `QRegularExpressionValidator` kiểm soát định dạng, `Validator::isValidDate` kiểm tra ngày thực và `dobError` kiểm tra tuổi.
- Mở rộng kiểm thử backend: xếp hạng 3 gia sư, chọn không phải TOP 1, tạo lớp đúng gia sư đã chọn; bổ sung trường hợp ngày tháng sai/năm nhuận. 213/213 assertions backend đạt.
- Xem `UPDATE_2026_11_09_MATCHING_DOB.md` để biết chi tiết, cách thử UI và giới hạn kiểm thử.

## 09/11/2026 — Thống nhất nghiệp vụ ghép gia sư, đăng ký môn, lớp học

- Bổ sung `ClassService` làm điểm đi qua thống nhất cho toàn bộ thao tác lớp; trạng thái ghép và tải gia sư được dựng lại từ lớp hiệu lực, không cập nhật tay.
- Chặn ghép trùng cùng gia sư hoặc cùng môn; hỗ trợ tìm gia sư theo từng môn; kiểm tra lại điều kiện, lịch, học phí và sức chứa khi xác nhận.
- Chặn xóa học sinh/gia sư còn lớp liên quan (kể cả lớp đã hoàn thành); phát hiện dữ liệu cũ tham chiếu sai và ghép trùng.
- Bổ sung ghi ba file kèm backup/rollback khi lỗi I/O và cảnh báo không crash-atomic.
- Bộ backend được mở rộng lên **192 phép kiểm tra**, kiểm thử thành công; Qt UI cần kiểm thử trên máy có Qt6.
- Chi tiết: `LOGIC_FIXES.md`.

# Các thay đổi so với bản gốc

## Giao diện đăng nhập (thiết kế lại theo ảnh mẫu 1578x996)
- `LoginWindow` được viết lại: nền minh hoạ, logo TutorFlow, 3 tính năng bên trái; thẻ đăng nhập bên phải với
  ô tài khoản/mật khẩu (có nút hiện/ẩn mật khẩu), "Nhớ đăng nhập", "Quên mật khẩu?", nút gradient, "Tài khoản giáo dục", "Đăng ký ngay".
- Bố cục đặt theo toạ độ ảnh thiết kế và co giãn theo cửa sổ nên luôn đúng tỉ lệ. Logic đăng nhập giữ nguyên.
- Tài nguyên ảnh nằm trong `assets/login/` (tạo bằng `tools/make_login_assets.py`). "Quên mật khẩu", "Tài khoản giáo dục",
  "Đăng ký" hiện chỉ hiện thông báo "chưa hỗ trợ" vì chưa có chức năng tương ứng.

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
- Giao diện: kiểm tra SĐT / ngày sinh / học phí / ngân sách, chặn trùng mã, hiển thị lý do ghép thật,
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

## Giao diện TutorHub (bảng màu xanh da trời (#E2F3FB, #B1E0F4, #80CCEE, #53B7E8, #1E9EDB))
- Sidebar xanh đen, mục đang chọn dùng gradient pastel teal → xanh da trời, thêm nhóm menu và nút "Đăng xuất".
- Header bảng xanh đen chữ trắng; nút chính dùng gradient pastel; hộp thoại thêm/sửa có thanh tiêu đề gradient.
- Xanh lá đậm được thay bằng bảng màu xanh da trời (#E2F3FB, #B1E0F4, #80CCEE, #53B7E8, #1E9EDB); logo được đổi tông pastel (bản gốc: assets/logo_original.png).
- Đổi màn hình đăng nhập, biểu đồ, QSS toàn cục. Không thay đổi chức năng nghiệp vụ.
- Thay các mảng màu đen (sidebar, header bảng, chữ) bằng xanh dương đậm (#0343ED → #0228C7 → #0024B7) theo ảnh mẫu.
- Menu (sidebar) và header bảng đổi sang tông xanh cat-eye (#0E2233 → #154C69 → #1070A2 → #2A9FC9).
- Phối lại toàn bộ màu theo một hệ thống "Ocean" duy nhất (xanh dương 200–205°): sidebar/header bảng navy-xanh, nút gradient xanh da trời chữ trắng, chữ navy #0F2B46, nền #F2F7FB.
- Giao diện hiện đại 'glass/cosmic': nền gradient + vầng sáng + hành tinh, thẻ kính có bóng, nút phát sáng cyan→xanh / xanh→tím, biểu đồ gradient xanh–tím, sidebar xanh sapphire; áp dụng cho mọi trang, hộp thoại và đăng nhập.
- Thêm hiệu ứng lấp lánh/công nghệ: sao 4 cánh, tinh vân, dải sáng, hành tinh có viền sáng, sidebar sapphire có sao và viền sáng, mục menu neon, thẻ kính trong suốt.

## Giao diện và quy trình nhập liệu (29/10/2026)
- Thiết kế lại LoginWindow theo bố cục hai cột, tông màu xanh tím pastel, thẻ đăng nhập bo góc.
- Hiển thị lỗi đăng nhập ngay dưới trường liên quan; sai thông tin xác thực hiện lỗi đỏ trong form.
- Loại bỏ thông tin đăng nhập hard-code `admin/123456`; chỉ dùng `FileManager::validateAdminLogin`.
- FormCheck hiển thị lỗi kiểm tra trực tiếp dưới ô nhập; `attachLive` thông báo lỗi định dạng trong lúc gõ.
- Sau khi lưu học sinh/gia sư, hỏi có nhập tiếp không; chọn Có mở biểu mẫu mới, Không chọn bản ghi mới trên bảng.
- Lưu ý: Chưa thử biên dịch giao diện Qt6 trong môi trường hiện tại do thiếu Qt6; backend CTest đã chạy đạt.

## 2026-11-09 — Giao diện lọc theo cột và chuẩn hóa dữ liệu (v3)
- Đăng nhập: bỏ phân đoạn "Hoặc đăng nhập bằng", nút "Tài khoản giáo dục", link "Đăng ký ngay"; thu gọn thẻ đăng nhập.
- Bỏ ô tìm kiếm hệ thống tại đầu trang. Tạo khung tìm kiếm chung cho Học sinh, Gia sư, Lớp, mỗi trường có nhãn riêng, nút Tìm kiếm / Xóa và hỗ trợ Enter.
- Lọc không phân biệt dấu tiếng Việt, phân biệt các trường và áp dụng kết hợp điều kiện.
- Lưu bản gốc `data/archive/class_original_before_cleanup_2026-11-09.txt`; chuyển 5 bản ghi ghép trùng sang archive; dữ liệu `data/class.txt` còn 22 lớp không trùng.
- Gỡ thuộc tính ngày kết thúc từ mô hình lớp, hộp thoại, nghiệp vụ và lưu TXT; đọc được class.txt cũ 11 trường và mới 10 trường.
- Thêm kiểm thử di trú format dữ liệu lớp, dữ liệu mẫu sạch; backend đạt 226/226 kiểm tra, gồm AddressSanitizer/UBSan.
- Chưa có Qt 6 trên môi trường kiểm tra nên chưa thể build/chạy phần GUI tại đây.
