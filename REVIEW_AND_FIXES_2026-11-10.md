# TutorFlow PBL2 — Rà soát và sửa lỗi bổ sung (10/11/2026)

Bản nền: `TutorFlow_PBL2_Merged_PBL_Functions.zip`. Đây là **lượt sửa bổ sung** trên đúng bản đã tích hợp hợp đồng, thanh toán, PDF và giao diện tìm kiếm. Không thay thế dữ liệu mẫu; giữ nguyên 22 lớp, file hợp đồng, ảnh và tài liệu cũ.

## Lỗi đã xác nhận và sửa

| Mức độ | Vấn đề trong bản nền | Cách xử lý mới |
|---|---|---|
| P0 | Dòng học sinh/gia sư/lớp có số cột sai bị bỏ qua, sau đó một lần lưu có thể ghi đè dữ liệu đã bị bỏ | Giữ nguyên file gốc; ghi nhận lỗi đọc dạng **sticky**, từ chối `saveAll`, các hàm lưu từng file và lưu hợp đồng cho đến khi sửa nguồn và khởi động lại |
| P0 | ID trùng trong file TXT được đọc thành hai đối tượng, có nguy cơ ghi sai quan hệ | Phát hiện ID trùng hoặc rỗng; không cho ghi tiếp |
| P0 | Các trường số như kinh nghiệm, lượng học sinh hoặc ngân sách có thể bị ép về 0 dù bản ghi chứa chuỗi không hợp lệ | Phát hiện lỗi số/ngày/trạng thái và khóa lưu; cho phép xem phần dữ liệu đọc được để chẩn đoán, nhưng không viết đè nguồn |
| P1 | `getDataPath` dò **từng tên file** và có thể lấy `student.txt` từ `./data` nhưng `tutor.txt` từ `../data` | Chỉ xác định **một thư mục dữ liệu** cho toàn bộ file |
| P1 | Xóa lớp thất bại khi ghi: phần tử được chèn lại **cuối danh sách**, đảo thứ tự bản ghi | Chụp thứ tự danh sách và khôi phục nguyên vị trí trước khi thử xóa |
| P1 | Tương tự, xóa hợp đồng/học sinh/gia sư thất bại có thể làm thay đổi thứ tự bản ghi trong bộ nhớ | Hoàn nguyên danh sách theo thứ tự cũ khi ghi thất bại |
| P1 | `hasSavedReference` chỉ kiểm tra số cột hợp đồng; một dòng đủ cột nhưng sai mã/kiểu/ngày/số có thể bị coi là không liên quan | Kiểm tra cấu trúc, ID, ngày, các khoản phí, trạng thái và dòng thanh toán; nếu có dòng hỏng, **từ chối xóa đối tượng liên quan** (fail closed) |
| P2 | Form thêm/sửa học sinh/gia sư chấp nhận danh sách môn chỉ gồm dấu phẩy hoặc lịch sai, khiến dữ liệu nguồn không hợp lệ | Kiểm tra mục môn rỗng/dấu phẩy thừa, ký tự phân tách file và chuẩn lịch T2…CN ngay trước khi lưu |
| P2 | Khi lỗi TXT xảy ra, giao diện không trình bày rõ nguyên nhân khiến thao tác lưu bị khóa | Thêm thông báo lỗi đọc dữ liệu và hướng dẫn khôi phục file từ bản sao |

## Tính tương thích và dữ liệu

- **Không đổi schema**: student.txt 14/15 cột, tutor.txt 15/16 cột, class.txt 10/11 cột (bản 11 cột bỏ qua ngày kết thúc), contracts.txt hiện hữu.
- Thao tác bị từ chối khi file nguồn lỗi; bản gốc trên đĩa vẫn được giữ nguyên. Không tự động xoá dòng sai hay tự động điền lại giá trị mất.
- Nếu ứng dụng hiện thông báo **“Lỗi đọc dữ liệu — đã khóa lưu”**, thoát ứng dụng, sao lưu thư mục `data`, chỉnh file bị nêu tên hoặc khôi phục từ bản backup, rồi mở lại. Không bỏ qua thông báo để nhập tiếp.
- Cơ chế phát hiện dữ liệu lỗi hoạt động trong tiến trình khi loader đọc file. Nếu chỉnh tay dữ liệu bên ngoài trong khi ứng dụng đang mở, cần khởi động lại để kiểm tra lại nguồn.

## Kết quả xác minh trong môi trường hiện tại

- Biên dịch C++17 core và CTest: **389/389 điều kiện kiểm thử đạt** (1 bài CTest chứa nhiều assertion).
- AddressSanitizer + UndefinedBehaviorSanitizer: CTest **đạt** ở bộ build instrumented.
- Test mới: loại dữ liệu thiếu cột, trùng ID, khóa ghi, phục hồi sau sửa file, rollback thứ tự lớp/hợp đồng, khóa xóa với hợp đồng hỏng dù đủ cột, thư mục dữ liệu thống nhất.
- Bộ dữ liệu mẫu 22 lớp vượt kiểm tra tham chiếu, ghép trùng, lớp không hợp lệ, và kiểm tra loader không báo lỗi.
- **Chưa biên dịch và kiểm thử UI Qt 6**: môi trường phát triển hiện không có Qt 6. Các chỉnh sửa trong `MainWindow.cpp` đã rà soát tĩnh, nhưng chưa xác nhận UI trên Windows.

## Cách build và kiểm thử

```bash
cmake -S . -B build -G Ninja -DBUILD_QT_APP=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

Trên Windows khi có Qt 6 Widgets, mở thư mục dự án trong **Qt Creator**, cấu hình CMake với `BUILD_QT_APP=ON`, build `PBL2App`, chạy qua `run_app.bat` hoặc launcher phù hợp.

### Checklist UI thực tế cần chạy trên máy có Qt 6

1. Đăng nhập → lần đầu mở, xác nhận không báo lỗi dữ liệu khi dùng bộ mẫu.
2. Tạo/sửa học sinh và gia sư: thử môn `,,`, `Toán,`, lịch `T9` (phải báo lỗi), ngày sinh không tồn tại (phải báo lỗi), thông tin đúng (lưu được).
3. Tìm nhiều gia sư: chọn TOP 2 thay TOP 1 → xác nhận → kiểm tra đúng gia sư/lớp được tạo, không trùng lịch.
4. Tạo/sửa/xóa/hoàn thành lớp; thử xóa lớp đã có hợp đồng (phải bị chặn).
5. Tạo hợp đồng 3 loại, ghi nhận thanh toán, xuất PDF, khởi động lại kiểm tra số liệu vẫn khớp.
6. Sao lưu dữ liệu rồi cố ý thêm một dòng hỏng vào TXT để kiểm tra ứng dụng cảnh báo và không ghi đè; khôi phục file trước khi tiếp tục dùng.

## Những giới hạn chưa thể loại bỏ bằng patch nhỏ

- Lịch học hiện chỉ có **ngày trong tuần**; hai lớp cùng ngày của một học sinh/gia sư bị xem là trùng dù giờ học có thể khác nhau. Muốn hỗ trợ nhiều ca/ngày cần mở rộng model thành khung giờ bắt đầu/kết thúc và migration.
- Mô hình ba file TXT và `contracts.txt` có cơ chế ghi tạm/khôi phục, nhưng chưa có giao dịch DB thực sự, khóa nhiều tiến trình hay bảo đảm bền vững sau mất điện ở cấp ổ đĩa. Nếu dự án chuyển sang sử dụng thực tế, nên sử dụng SQLite với khóa ngoại và transaction.
- Mã hợp đồng đang phát sinh dựa vào ID hiện có; nếu xóa bản ghi cuối, số ID đó có thể được cấp lại. Để vận hành tài chính thật nên bổ sung định danh không tái sử dụng và audit log không thể sửa.
- `admin.txt` dùng mật khẩu băm SHA-256 có muối để tương thích hệ thống cũ, chưa phải cơ chế lưu mật khẩu theo chuẩn production (Argon2id/scrypt); cần đổi mật khẩu mặc định trước khi triển khai.
- Chưa xác nhận render giao diện, Qt PDF và các thao tác người dùng trên Windows vì máy test không có Qt 6.

**Phạm vi nghiệm thu đã đạt: code backend/kiểm thử tự động. Phạm vi chưa nghiệm thu: bản Qt desktop thực tế.**
