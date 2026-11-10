# Tích hợp các chức năng còn thiếu từ PBL.rar vào TutorFlow

**Bản tích hợp ngày 10/11/2026.** Nền tảng chính là TutorFlow bản đã sửa logic/UI; chỉ chuyển các tính năng hữu ích chưa có, KHÔNG thay thế kiến trúc lõi hoặc dữ liệu mẫu bằng file của PBL cũ.

## Đối chiếu tính năng

| Nhóm tính năng của PBL.rar | Trạng thái trong TutorFlow tích hợp | Vị trí chính |
|---|---|---|
| Đăng nhập, quản lý học sinh/gia sư/lớp, ghép gia sư | Đã có từ bản TutorFlow; tiếp tục dùng bản đã sửa | `src/ui/MainWindow.cpp`, `ClassService`, `MatchingService` |
| Quản lý hợp đồng lớp học | **Đã bổ sung** tạo, sửa, xem, lọc, hủy, xóa có điều kiện | `ContractPage`, `ContractService` |
| Quản lý hợp đồng học viên | **Đã bổ sung** | `ContractPage`, `ContractService` |
| Quản lý hợp đồng gia sư | **Đã bổ sung** | `ContractPage`, `ContractService` |
| Xuất hợp đồng PDF | **Đã bổ sung** qua `QPdfWriter` của Qt 6 | `ContractPage::exportPdf()` |
| Theo dõi khoản phí/hoa hồng và thanh toán | **Đã bổ sung** ghi nhận từng khoản thanh toán, còn phải thu, lịch sử | `ContractService` |
| Thống kê hợp đồng và doanh thu theo tháng | **Đã bổ sung**, doanh thu = khoản đã nhận, không cộng nhầm tiền học phí | `MainWindow::refreshStats()`, `ContractService` |
| Thống kê học sinh theo môn | **Đã bổ sung** biểu đồ môn đăng ký | `MainWindow::createStatsPage()` |
| Bật/tắt trạng thái nhận lớp của gia sư | **Đã bổ sung** thao tác ở bảng gia sư | `MainWindow.cpp`, `ClassService::updateTutor()` |

## Chủ đích không chép nguyên

- PBL cũ có liên kết 'Liên hệ' dạng thông báo placeholder; không tạo nút giả không hoạt động.
- Không khôi phục **ngày kết thúc** vì người dùng đã yêu cầu bỏ trước đó; hợp đồng có ngày lập và trạng thái hiệu lực, lớp vẫn có ngày bắt đầu.
- Không chép dữ liệu có mã tham chiếu khác từ `PBL.rar` vào thư mục `data/` đang hoạt động, tránh lỗi khóa ngoại. Dữ liệu hợp đồng mới được ghi vào `data/contracts.txt` khi lập hợp đồng đầu tiên.
- Hợp đồng PDF là bản mẫu thông tin để xem/đối chiếu, cần rà điều khoản pháp lý trước khi ký chính thức.
- 'Nhớ đăng nhập' chỉ ghi nhớ tên tài khoản, không lưu mật khẩu.

## Quy trình sử dụng

1. Đăng nhập bằng admin; kiểm tra danh sách học sinh/gia sư/lớp.
2. Mở **Hợp đồng** ở menu trái, chọn một trong ba tab.
3. Nhấn **Tạo hợp đồng**, chọn đúng mã đối tượng, ngày lập và phí dịch vụ/hoa hồng. Học phí lớp được lấy từ dữ liệu lớp.
4. Chọn hợp đồng để **Chi tiết**, **Ghi nhận thanh toán**, **Sửa**, **Xuất PDF** hoặc **Hủy hiệu lực**.
5. Xem doanh thu tháng (chỉ tiền đã thu) và biểu đồ trong **Thống kê**.
6. Hợp đồng có khoản thanh toán đã ghi nhận không được xóa để bảo vệ sổ thu; có thể hủy hiệu lực mà vẫn giữ lịch sử.

## Mô hình lưu trữ

`data/contracts.txt` được ghi nguyên tử trong một file (mỗi dòng phân cách `|`):

- `C|CTC1|CLASS|LH101|10/11/2026|150000|300000|1|ghi chú` — hợp đồng. Ba tiền tố ID: CTC (lớp), CTS (học viên), CTT (gia sư).
- `P|PAY1|CTC1|10/11/2026|100000|chuyển khoản` — khoản thu.

`student.txt` dùng 14 cột, `tutor.txt` dùng 15 cột. Ứng dụng cũng đọc bản ghi có thêm một cột cuối từ định dạng lưu trữ trước đây, nhưng bỏ cột đó khi ghi lại. `class.txt` tiếp tục 10 cột và vẫn đọc bản cũ 11 cột.

Trước khi nâng cấp bộ dữ liệu hiện có, sao lưu cả thư mục `data/`. Lưu hợp đồng và lưu ba tập tin hồ sơ/lớp là **hai giao dịch riêng**: không tuyên bố bảo đảm ACID xuyên 4 tập tin nếu mất điện.

## Xây dựng và kiểm thử

**Windows, Qt 6 Widgets** (cần thêm Qt vào `CMAKE_PREFIX_PATH` nếu CMake không tự tìm):

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.7.0/mingw_64
cmake --build build
ctest --test-dir build --output-on-failure
```

Backend (không cần Qt):

```
cmake -S . -B build-backend -DBUILD_QT_APP=OFF
cmake --build build-backend
ctest --test-dir build-backend --output-on-failure
```

**Giới hạn kiểm chứng:** Kiểm thử và build backend được chạy trong môi trường hiện tại. Chưa có Qt 6 nên giao diện và xuất PDF phải được biên dịch, kiểm tra trên máy Windows cài Qt; chưa khẳng định ứng dụng desktop đã chạy trực quan. Xem các tình huống kiểm thử trong `tests/backend_test.cpp`.
