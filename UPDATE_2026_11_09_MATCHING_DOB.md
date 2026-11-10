# TutorFlow PBL2 — Cập nhật tìm gia sư và ngày sinh

## 1. Danh sách gia sư để người dùng lựa chọn

**Thay đổi:** `src/ui/MainWindow.h`, `src/ui/MainWindow.cpp`.

1. Mở **Ghép gia sư**, chọn học sinh và môn học cần tìm.
2. Nhấn **TÌM GIA SƯ PHÙ HỢP**.
3. Hệ thống hiển thị **tất cả gia sư đáp ứng điều kiện bắt buộc** trong bảng gồm: Thứ hạng, Gia sư, Độ phù hợp, Học phí/buổi, Kinh nghiệm, Khu vực dạy.
4. Danh sách được sắp xếp theo **độ phù hợp giảm dần** (`MatchingService::matchStudentsToTutors`); nếu bằng điểm, ưu tiên học phí thấp, sau cùng là mã gia sư.
5. Nhấn **bất kỳ dòng nào** để xem thẻ chi tiết, điểm thành phần, lý do gợi ý và nút **CHỌN GIA SƯ NÀY & TẠO LỚP**. Ví dụ: không thích gia sư TOP 1, chọn TOP 2.
6. Chỉ khi nhấn nút tạo lớp **và xác nhận** thì `ClassService::create()` mới thêm lớp mới.

**An toàn dữ liệu:** Đổi học sinh, đổi môn học, tìm lại, cập nhật dữ liệu hoặc không có kết quả sẽ xóa lựa chọn cũ. Trước khi ghép vẫn kiểm tra lại điều kiện, trùng ghép, giới hạn sức chứa. Không còn popup thông báo "gia sư TOP 1" làm gián đoạn thao tác chọn.

## 2. Nhập ngày sinh: để trống khi tạo mới, bắt buộc đúng định dạng

Áp dụng cho **Thêm học sinh**, **Thêm gia sư**, **Sửa học sinh**, **Sửa gia sư**:

- **Tạo mới:** ô ngày sinh trống hoàn toàn; chỉ hiển thị gợi ý `Vd: 25/12/2008 (dd/MM/yyyy)` bằng placeholder, không phải giá trị thật.
- **Sửa hồ sơ:** vẫn hiển thị **ngày sinh đang lưu của người đó** để tránh làm mất dữ liệu hiện có, nhưng cùng bộ quy tắc kiểm tra.
- Chỉ nhận cấu trúc `dd/MM/yyyy`: 2 chữ số ngày, `/`, 2 chữ số tháng, `/`, 4 chữ số năm.
- Regex cho nhập liệu: `(0[1-9]|[12][0-9]|3[01])/(0[1-9]|1[0-2])/[0-9]{4}`.
- Kiểm tra khi gõ và khi lưu bằng `dobError` + `Validator::isValidDate`: ngày có thực, năm nhuận, năm trong khoảng 1900–2100, không ở tương lai, tuổi học sinh trong 5–25, tuổi gia sư trong 16–70 (giữ quy tắc nghiệp vụ từ bản trước).
- Nếu trống, thiếu chữ số, sai định dạng, ngày không tồn tại hay ngoài độ tuổi: nút Lưu/Cập nhật **không thực hiện lưu**, lỗi xuất hiện dưới ô nhập.

Ví dụ: `25/12/2008` đúng định dạng; `25/12/08`, `1/1/2008`, `31/02/2008`, `29/02/2023` không hợp lệ. `29/02/2024` hợp lệ về mặt lịch (vẫn cần thỏa tuổi theo vai trò).

## 3. Kiểm thử

Trên hệ thống phát triển không có Qt 6:

```sh
cmake -S . -B build-backend -DBUILD_QT_APP=OFF
cmake --build build-backend
ctest --test-dir build-backend --output-on-failure
```

**Kết quả tại thời điểm bàn giao:** 213/213 assertions backend đạt, trong đó có kiểm tra xếp hạng nhiều ứng viên, chọn TOP 2, tạo lớp đúng gia sư đã chọn, chặn trùng môn và xác thực chuỗi ngày theo định dạng chính xác/năm nhuận.

**Giới hạn:** Chưa biên dịch/chạy giao diện Qt Widgets vì máy kiểm thử không có Qt 6. Trên máy có Qt 6, chạy `cmake -S . -B build && cmake --build build`, sau đó mở `PBL2App` và kiểm thử UI như các bước nêu trên. Không có thay đổi định dạng file TXT, dữ liệu mẫu được giữ nguyên.
