# TutorFlow PBL2 — Sửa logic nghiệp vụ (09/11/2026)

## 1. Luồng nghiệp vụ đã thống nhất

- **Học sinh đăng ký môn** bằng danh sách `Student::needSubjects` (không cho đăng ký trùng tên khác chữ hoa/dấu). Trang tìm gia sư cho phép tìm **tất cả môn** hoặc **từng môn**; ví dụ Toán và Anh có thể ghép với **hai gia sư khác nhau**.
- **Tìm gia sư** áp dụng điều kiện bắt buộc trước khi chấm điểm: gia sư còn nhận lớp và dưới 8 học sinh; dạy **tất cả các môn đang chọn**; có ít nhất một ca rảnh chung khi học sinh khai lịch; học phí không vượt ngân sách (nếu học sinh cung cấp ngân sách).
- **Xác nhận ghép** kiểm tra lại điều kiện tại thời điểm nhấn, chọn **lịch giao nhau**, tự tạo lớp qua `ClassService::create()`; không tăng/giảm số học sinh bằng cách cộng tay.
- **Tạo lớp thủ công, chỉnh sửa, xóa lớp** đều đi qua `ClassService`, xác thực tham chiếu, trạng thái, môn học, lịch học, học phí, ngày, chống ghép trùng, rồi đồng bộ trạng thái và lưu ba file.
- **Thống kê** `Student::isMatched` dựa trên việc học sinh có ít nhất một lớp hiệu lực đã gán gia sư. `Tutor::currentStudents` là số **học sinh khác nhau** trong các lớp hiệu lực, không tính lớp đã hoàn thành/cần gia sư. Một học sinh học hai môn cùng gia sư vẫn chỉ chiếm 1 chỗ.
- **Xóa học sinh/gia sư** không được phép nếu đối tượng có lớp đang học, chờ xếp lịch **hoặc đã hoàn thành**; giữ tính toàn vẹn lịch sử. Sau khi xóa hết lớp liên quan mới có thể xóa hồ sơ.

## 2. Ý nghĩa 4 trạng thái lớp

| Trạng thái | Gia sư | Tính đã ghép | Chiếm chỗ gia sư |
|---|---|---|---|
| `Cần gia sư` | Không có | Không | Không |
| `Chờ xếp lịch` | Có hoặc chưa có | Có nếu đã gán gia sư | Có nếu đã gán gia sư |
| `Đang học` | Bắt buộc có | Có | Có |
| `Hoàn thành` | Có, giữ lại để lưu lịch sử | Không | Không |

Không cho một học sinh ghép trùng với cùng gia sư trong hai lớp còn hiệu lực; đồng thời không cho ghép lại cùng môn đã có lớp hiệu lực với gia sư khác. Một lớp nhiều môn chỉ được phép khi gia sư dạy được mọi môn trong lớp đó.

## 3. Các tệp thay đổi chính

- `src/services/ClassService.h/.cpp`: **mới**, lớp dịch vụ nghiệp vụ thống nhất; xác thực CRUD lớp, xử lý rollback và tái dựng dữ liệu dẫn xuất.
- `src/services/MatchingService.h/.cpp`: bổ sung `meetsMandatoryRequirements()`; các ràng buộc bắt buộc không còn chỉ là điểm thưởng/phạt.
- `src/ui/MainWindow.h/.cpp`: kết nối cả ghép tự động/tạo lớp thủ công/sửa/xóa vào dịch vụ mới; nút xác nhận vô hiệu khi lựa chọn hết hiệu lực; tìm gia sư theo môn; thống kê gia sư khả dụng tính cả sức chứa; chặn xóa đối tượng đã có lớp.
- `src/repositories/FileManager.h/.cpp`: thêm `saveAll()` lưu nhất quán 3 file và **báo lỗi** để nghiệp vụ hoàn tác trong bộ nhớ; có bản sao dự phòng cho trường hợp lỗi I/O.
- `src/entities/Student.cpp`: tránh đăng ký trùng môn khác chữ hoa/dấu; hủy môn theo tên chuẩn hóa.
- `tests/backend_test.cpp`, `CMakeLists.txt`: kiểm thử và liên kết dịch vụ mới.

## 4. Lưu ý dữ liệu thực tế

1. **Sao lưu thư mục `data/` hiện tại trước khi chạy bản mới.** Khi ứng dụng khởi động, các cờ ghép và số học sinh được tính lại từ dữ liệu lớp đã có. Lần ghi tiếp theo sẽ lưu các giá trị đã tính lại.
2. Dữ liệu mẫu đi kèm có **những lớp ghép trùng sẵn**. Chương trình **không tự xóa** vì có thể là dữ liệu lịch sử quan trọng. Màn hình sẽ cảnh báo để admin tự kiểm tra các lớp đó và hoàn thành/xóa nếu không còn cần.
3. `FileManager::saveAll()` staging 3 file, sao lưu trước khi ghi và cố phục hồi nếu lỗi I/O. **Đây không phải transaction crash-atomic trên 3 file** (mất điện giữa chừng vẫn có thể tạo ra trạng thái không đồng bộ). Nếu thấy tệp `.backup` sau khi chương trình ngắt đột ngột, **giữ nguyên** rồi sao lưu để phục hồi, không xóa tùy tiện. Giải pháp sản xuất nên chuyển sang SQLite transaction để đảm bảo toàn vẹn khi mất điện.
4. Cơ chế đăng nhập hiện vẫn theo **vai trò admin** của bản gốc; dự án **chưa** có hệ thống tài khoản đăng ký/đăng nhập riêng cho học sinh/gia sư.

## 5. Kiểm thử và chạy ứng dụng

```bash
# Build/test backend trên Windows hoặc Linux (không cần Qt)
cmake -S . -B build-backend -DBUILD_QT_APP=OFF
cmake --build build-backend
ctest --test-dir build-backend --output-on-failure

# Build giao diện Qt6 (phải có Qt6 Widgets cùng compiler tương thích)
cmake -S . -B build -DBUILD_QT_APP=ON -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/mingw_64"
cmake --build build
```

- Backend đã thực thi **192/192 phép kiểm tra thành công**, gồm vòng đời lớp, ghép theo môn, sửa gia sư, kết thúc lớp, ngăn ghép trùng, giới hạn sức chứa, thử lỗi lưu dữ liệu, đọc lại file.
- Cũng chạy các bài backend với **AddressSanitizer / UndefinedBehaviorSanitizer**.
- Môi trường chỉnh sửa hiện tại **không có Qt6** nên chưa thể biên dịch/chạy bản giao diện; cần build và chạy thử UI trên máy đã cài Qt6 trước khi nghiệm thu.
