# PBL2 - TutorFlow: Quản lý và tìm kiếm gia sư

Ứng dụng desktop **C++17 + Qt 6 Widgets + CMake** giúp trung tâm quản lý học sinh, gia sư, lớp học và
**tự động ghép gia sư phù hợp** cho từng học sinh. Dữ liệu lưu bằng file TXT.

## Tính năng
- Đăng nhập admin (mật khẩu lưu dạng băm SHA-256 + muối); bỏ các tùy chọn tài khoản giáo dục và đăng ký chưa triển khai.
- Quản lý (thêm / sửa / xóa / tìm kiếm / lọc) học sinh, gia sư và lớp học.
  Mỗi trang có bộ lọc riêng theo các cột và nút **Tìm kiếm** / **Xóa**, tìm không phân biệt dấu.
  Không còn ô "Tìm kiếm hệ thống" ở thanh tiêu đề.
- Bảng điều khiển và thống kê: số học sinh, gia sư, lớp đang học, tỉ lệ ghép.
- **Ghép gia sư thông minh**: xếp hạng **toàn bộ gia sư đủ điều kiện** theo điểm phù hợp giảm dần;
  **người dùng tự chọn gia sư trong danh sách** để xem chi tiết, điểm thành phần và lý do ghép;
  chỉ tạo lớp sau khi chọn và xác nhận, đồng bộ học sinh/gia sư.
- Kiểm tra dữ liệu nhập (số điện thoại, ngày sinh `dd/MM/yyyy`, học phí) và chặn trùng mã.
  Ngày sinh ở form **Thêm mới** để trống, có ví dụ mờ, bắt buộc nhập đúng và là ngày có thật.

## Cấu trúc
```
src/entities       Person, Student, Tutor, TutoringClass
src/structures     Vector, HashMap (open addressing + rehash), Algorithms (merge sort)
src/services       MatchingService (lọc/xếp hạng), ClassService (nghiệp vụ lớp)
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

**Tài khoản demo trên GitHub:** `demo` / `TutorFlow-Demo-Only`. Đây là thông tin công khai chỉ để chạy dữ liệu minh họa; hãy đổi mật khẩu trước khi sử dụng dữ liệu riêng. Không dùng tài khoản này cho môi trường thật.

## Thuật toán ghép gia sư
**Bước 1 — lọc bắt buộc:** gia sư phải còn nhận học sinh, đang dạy dưới 8 học sinh khác nhau, dạy **toàn bộ môn đang tìm**, có lịch rảnh trùng (khi học sinh đã khai lịch), và học phí không vượt ngân sách nếu có. Học sinh đăng ký nhiều môn có thể chọn tìm theo **từng môn** để ghép nhiều gia sư khác nhau.

**Bước 2 — xếp hạng:** chỉ chấm điểm các ứng viên đã thỏa mãn điều kiện bắt buộc:

| Thành phần | Tối đa | Cách tính |
|---|---|---|
| Học phí | 45 | tỉ lệ học phí / ngân sách (<= 0.8 được điểm cao nhất); thiếu dữ liệu -> điểm trung tính |
| Kinh nghiệm | 25 | theo số năm (>= 10, >= 5, >= 3, >= 1) |
| Lịch dạy | 20 | 20 x (số buổi trùng / số buổi học sinh rảnh) |
| Khu vực | 10 | trùng khu vực = 10, chứa nhau = 8, khác = 3 |

`điểm = tổng x (0.85 + 0.15 x tỉ lệ môn phủ) + điều chỉnh giới tính (+3 / -8) - số học sinh hiện có`, giới hạn 0-100.
So sánh khu vực, môn, lịch **không phân biệt hoa/thường và dấu** ("Hải Châu" = "hai chau").
Kết quả sắp xếp bằng **merge sort** tự cài (O(n log n), ổn định); cùng điểm thì học phí thấp hơn đứng trước.

## Tính nhất quán nghiệp vụ

- Mọi thao tác tạo/sửa/xóa lớp đi qua `ClassService`, đồng bộ lại `isMatched`, `currentStudents` và dashboard dựa trên dữ liệu lớp, chống ghép trùng.
- Chặn xóa học sinh/gia sư khi còn lớp liên quan; lớp `Hoàn thành` không chiếm sức chứa gia sư nhưng vẫn giữ lịch sử.
- `FileManager::saveAll()` dùng staging, backup và nhật ký giao dịch để **phục hồi đồng bộ ba file** khi lần chạy trước bị gián đoạn. Chưa đảm bảo độ bền ACID khi mất điện; xem [FINAL_FIXES_2026-11-10.md](FINAL_FIXES_2026-11-10.md).

## Định dạng dữ liệu (cột phân cách bằng `|`)
- `student.txt` (14 cột): mã, họ tên, SĐT, khu vực, giới tính, ngày sinh, khối, môn cần (phẩy), lịch rảnh,
  học lực, giới tính gia sư mong muốn, ngân sách/giờ, ghi chú, đã ghép (true/false)
- `tutor.txt` (15 cột): mã, họ tên, SĐT, địa chỉ, giới tính, ngày sinh, môn dạy (phẩy), trình độ, số năm kinh
  nghiệm, mô tả kinh nghiệm, học phí/giờ, lịch rảnh, khu vực dạy (phẩy), đang nhận (true/false), số học sinh hiện có
- `class.txt` (10 cột): mã lớp, mã học sinh, mã gia sư, môn, lịch, ngày bắt đầu, trạng thái,
  học phí/giờ, ngày tạo, ngày cập nhật. **Không còn trường ngày kết thúc.**
  Trình đọc vẫn hỗ trợ file cũ 11 cột (bỏ qua cột ngày kết thúc) để không mất dữ liệu.
- `admin.txt` (`tên|muối|băm`)

Ngày dùng định dạng `dd/MM/yyyy`. Ký tự `|` và xuống dòng trong nội dung nhập sẽ tự được thay thế khi lưu.

Chi tiết hai tính năng mới: [UPDATE_2026_11_09_MATCHING_DOB.md](UPDATE_2026_11_09_MATCHING_DOB.md).

## Thay đổi giao diện và dữ liệu mẫu (09/11/2026)

- Màn hình đăng nhập chỉ giữ đăng nhập truyền thống; phần "Hoặc đăng nhập bằng / Tài khoản giáo dục"
  và "Chưa có tài khoản? Đăng ký ngay" đã được bỏ.
- Dữ liệu mẫu ban đầu có 27 lớp, trong đó 5 lần ghép lặp theo cùng cặp học sinh–gia sư.
  Dữ liệu hiện có **22 lớp không lặp**. Giữ bản ghi đầu tiên của mỗi cặp đang hiệu lực;
  các bản ghi bỏ khỏi danh sách nằm trong `data/archive/duplicate_class_records_2026-11-09.txt`.
  Bản gốc nguyên vẹn ở `data/archive/class_original_before_cleanup_2026-11-09.txt`.
- `MainWindow` vẫn cảnh báo **nếu dữ liệu thực sự có lỗi** khi người dùng nhập dữ liệu khác;
  việc hết cảnh báo trên dữ liệu mẫu là do dữ liệu được chuẩn hóa, không phải giấu cảnh báo.
- Nếu bạn đã có dữ liệu tự nhập ở dự án khác, **sao lưu thư mục data trước khi thay**;
  không chép đè tự động file `student.txt`, `tutor.txt`, `class.txt` mà chưa kiểm tra.
- Qt 6 cần được cài ở máy Windows để biên dịch/chạy giao diện. Backend chạy test không cần Qt.

## Nâng cấp nghiệp vụ và UX ngày 10/11/2026

Xem **[FINAL_FIXES_2026-11-10.md](FINAL_FIXES_2026-11-10.md)** để có danh sách sửa đổi,
phục hồi dữ liệu, kiểm thử backend và checklist nghiệm thu giao diện.
**Quy ước lịch hiện tại chỉ có ngày trong tuần:** hai lớp cùng ngày của một học sinh/gia sư được coi là trùng cho đến khi dự án bổ sung giờ học cụ thể.
**Ngân sách/học phí dùng đơn vị đồng/giờ như tên trường dữ liệu**; dữ liệu do phiên bản cũ nhập cần được xác nhận lại đơn vị.

## Tính năng mới tích hợp từ PBL.rar (10/11/2026)

- **Quản lý hợp đồng**: 3 tab hợp đồng lớp/học viên/gia sư; tạo, sửa, tra cứu, xem chi tiết, hủy hiệu lực, xóa có ràng buộc, xuất PDF qua Qt 6.
- **Theo dõi khoản thu**: nhập từng khoản thanh toán, còn phải thu, lịch sử khoản thu; dashboard có số hợp đồng, doanh thu thực thu theo tháng, biểu đồ phân bố môn đăng ký.
- **Quản lý gia sư**: bật/tắt trạng thái nhận lớp ngay tại bảng.
- Định dạng TXT học sinh có 14 cột và gia sư có 15 cột; ứng dụng vẫn đọc các bản ghi cũ có thêm một cột cuối.
- Dữ liệu hợp đồng được lưu riêng ở `data/contracts.txt`. Không tự nhập dữ liệu từ file PBL cũ vì mã khóa ngoại khác nhau.
- Giữ nguyên yêu cầu **không có ngày kết thúc**.

Đọc **[INTEGRATION_PBL.md](INTEGRATION_PBL.md)** để xem bảng đối chiếu chức năng, sơ đồ lưu dữ liệu, hướng dẫn thao tác và giới hạn kiểm thử Qt 6.


## Rà soát và sửa lỗi bảo toàn dữ liệu (10/11/2026 — bản mới nhất)

Xem **[REVIEW_AND_FIXES_2026-11-10.md](REVIEW_AND_FIXES_2026-11-10.md)**.
Các phần đã sửa gồm: khóa ghi khi TXT sai định dạng/ID trùng/số không hợp lệ,
chọn thư mục dữ liệu thống nhất, giữ nguyên thứ tự khi rollback xóa, kiểm tra
hợp đồng hỏng theo hướng fail-closed và chặn form có môn/lịch sai.
Bản mới nhất đã chạy **389/389 kiểm tra backend thành công**, bao gồm bản
AddressSanitizer/UBSan; giao diện Qt 6 vẫn cần xác minh trên máy có Qt.
