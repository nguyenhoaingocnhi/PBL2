# TutorFlow PBL2 — Bản sửa nhất quán nghiệp vụ và giao diện (10/11/2026)

## Phạm vi

Bản này tiếp tục trực tiếp từ `TutorFlow_PBL2_UI_Data_Updated.zip` (không khởi tạo dự án mới). Giữ C++17, Qt 6 Widgets, CMake, các cấu trúc dữ liệu tự cài, tài sản giao diện và file TXT mẫu.

### 1. Chặn sửa hồ sơ gây hỏng lớp đang học

- `ClassService::updateStudent()` và `ClassService::updateTutor()` thay cho thao tác sửa rồi `FileManager::saveAll()` trực tiếp trong giao diện.
- Kiểm tra các lớp chịu tác động sau khi thay đổi môn, lịch rảnh và ngân sách; không cho xóa môn đang học, đổi lịch không bao trùm ngày học hoặc hạ ngân sách thấp hơn học phí lớp.
- Gia sư không được xóa môn đang dạy hoặc lịch rảnh chứa các lớp đã nhận. Chuyển trạng thái gia sư thành không nhận lớp mới vẫn được phép và **không hủy lớp đã có**.
- Kiểm tra đầu vào ở tầng dịch vụ; lỗi sẽ hoàn tác dữ liệu trong bộ nhớ và hiển thị lý do ở form.

### 2. Quy tắc lớp học và lịch học

- `SubjectUtils.h`: chuẩn hóa tên môn (các tên thông dụng như Toán/Toan, Tiếng Anh/Anh) và tách môn khỏi tên lớp kiểu cũ `Toan Lớp 10` hay `10 - Toan, Ly` theo **ranh giới rõ ràng**, không còn dùng phép `find()` dễ ghép nhầm.
- `ScheduleUtils.h`: lịch theo ngày `T2,T3,T4,T5,T6,T7,CN` (cũng nhận `T8`/`Chủ Nhật` làm CN); chặn ngày sai, ngày lặp và đoạn rỗng.
- `Đang học` bắt buộc có gia sư và có lịch; `Chờ xếp lịch` bắt buộc có gia sư nhưng có thể chưa có lịch; `Cần gia sư` không được gán gia sư; `Hoàn thành` giữ lịch sử và không thể tự chuyển lại thành lớp đang học.
- Khi có lịch, **mọi ngày lớp dạy phải có trong lịch rảnh của cả học sinh và gia sư** (trước đây chỉ cần trùng một ngày).
- Chặn lớp mới/sửa lớp gây trùng ngày với **bất kỳ lớp đang hiệu lực** của cùng học sinh hoặc cùng gia sư; đồng thời chặn trùng môn học, trùng cặp học sinh–gia sư và giới hạn 8 học sinh.
- Do dữ liệu chỉ có **ngày trong tuần, không có giờ bắt đầu/kết thúc**, cùng một ngày được coi là trùng lịch, dù ngoài đời có thể dạy hai ca khác giờ. Đây là quy tắc phòng tránh đặt trùng an toàn; muốn hỗ trợ nhiều ca/ngày phải bổ sung khung giờ thực tế vào mô hình.

### 3. Ghép thông minh dựa trên ngày *còn trống*

- Danh sách ứng viên vẫn cho phép chọn TOP 2, TOP 3,...; bổ sung loại gia sư không còn ngày trống **sau khi trừ lịch của cả học sinh và gia sư ở các lớp khác**.
- Điều chỉnh điểm lịch và sắp xếp lại toàn bộ danh sách theo **điểm phù hợp /100** thực tế; không gọi điểm thuật toán là tỷ lệ/xác suất thành công.
- Sau khi xác nhận chọn gia sư, tạo lớp với **một ngày trống khả thi**, không dùng toàn bộ lịch rảnh có nguy cơ xung đột. Kiểm tra lại tại thời điểm lưu; nếu có xung đột mới thì từ chối.
- Mã nguồn dùng `budgetPerHour` và `ratePerHour`; toàn bộ nhãn học phí/ngân sách trong giao diện ghép và quản lý được thống nhất thành **đồng/giờ**. **Không chuyển đổi giá trị cũ**; người dùng cần xác nhận dữ liệu lịch sử thực sự tính theo giờ trước khi đưa vào vận hành.

### 4. Lưu trữ, giao dịch, khôi phục

- `FileManager::saveAll()` dùng cơ chế staging + backup + nhật ký trạng thái `.tutorflow_pending`/`.tutorflow_committed`.
- Nếu ứng dụng dừng sau khi thay một/một số file nhưng chưa hoàn thành giao dịch, lần đọc kế tiếp sẽ phục hồi đồng bộ **cả ba** file `student.txt`, `tutor.txt`, `class.txt` từ backup.
- Nếu đã ghi dấu `COMMITTED` thì lần đọc kế tiếp giữ dữ liệu mới và xóa tệp tạm.
- Nếu thiếu backup hoặc nhật ký bị hỏng, chức năng ghi bị **từ chối**, giữ lại chứng cứ để người dùng khôi phục thủ công (không tiếp tục ghi đè).
- Việc cập nhật một file, ví dụ `admin.txt`, không còn xóa đích trước khi thay thành công trên Windows (dùng `MoveFileExW` thay thế).
- **Giới hạn:** đây là phục hồi khi ứng dụng bị ngắt, không phải transaction ACID hoàn chỉnh. Không bảo đảm bền vững tuyệt đối khi hệ điều hành/ổ đĩa mất điện, không hỗ trợ nhiều tiến trình cùng ghi vào một thư mục. Với hệ thống vận hành nhiều người, nên chuyển sang SQLite (transaction ACID) hoặc DBMS.

### 5. Giao diện

- Bộ lọc ở học sinh/gia sư/lớp tự sắp xếp 6/3/2 cột dựa vào bề ngang, vẫn giữ nhãn ngay trên trường lọc; không đổi nội dung, thứ tự hoặc nút Tìm kiếm/Xóa.
- Dùng thang `X/100` ở bảng ứng viên và biểu đồ vòng, không làm người dùng nhầm thành xác suất `%`.
- Lớp chưa sắp xếp lịch hiển thị **Chưa xếp lịch** thay vì để trống.
- `Ghi nhớ tài khoản`: chỉ lưu **tên tài khoản** qua `QSettings` nếu được chọn, không lưu mật khẩu và không tự đăng nhập.
- Bỏ liên kết **Quên mật khẩu** chỉ mở popup chưa hỗ trợ; không giả vờ có cơ chế khôi phục mật khẩu an toàn khi ứng dụng offline không có quy trình xác minh danh tính.
- Loại bỏ chỉ số tăng trưởng `+12%`, `+5%` cố định chưa có dữ liệu theo thời gian để chứng minh.

### 6. Dữ liệu mẫu

- Không xóa lớp: vẫn có **22 bản ghi**, nhưng sửa trạng thái/sắp xếp lịch các dòng cũ không đáp ứng điều kiện mới. Bản *nguyên vẹn* trước lần sửa này nằm ở `data/archive/class_before_final_consistency_repair_2026-11-10.txt`.
- `LH003`: thu lịch còn T2 (lịch chung thực tế).
- `LH007`, `LH008`, `LH011`, `LH016`: chưa tìm được ngày trùng với gia sư cũ → trạng thái `Cần gia sư`, bỏ gia sư gán trong dữ liệu đang hoạt động.
- `LH010`, `LH015`, `LH019`: phí lớp cũ cao hơn ngân sách học sinh → `Cần gia sư` thay vì giữ một phân công không khả thi.
- `LH022`: giữ gia sư, đổi phạm vi môn còn `Lý`, chuyển `Chờ xếp lịch` để tránh trùng lớp Toán và trùng ngày với lớp cũ.
- `LH026`: giữ gia sư, đổi phạm vi môn còn `Lý`, chuyển `Chờ xếp lịch` để tránh trùng môn Hóa và xung đột lịch.
- Không tự ý giảm phí, tăng ngân sách, thay lịch rảnh cá nhân hay xóa vĩnh viễn bản ghi lịch sử.

### 7. Kiểm thử và hạn chế xác minh

- `cmake -S . -B build -DBUILD_QT_APP=OFF && cmake --build build && ctest --test-dir build --output-on-failure`.
- Kết quả chạy tại môi trường bàn giao: **280/280 kiểm tra backend đạt**; test CTest đạt 1/1; kiểm thử AddressSanitizer + UndefinedBehaviorSanitizer O0 đạt 1/1.
- Bộ test bao phủ đăng ký môn, bảng xếp hạng, chọn ứng viên không đứng đầu, CRUD lớp, trạng thái/sức chứa, cập nhật hồ sơ có/không xung đột, đặt trùng ngày, chuẩn hóa môn, ngày sinh, chuyển đổi định dạng lớp cũ, khôi phục giao dịch đứt giữa chừng, dữ liệu mẫu hợp lệ.
- **Chưa biên dịch và chạy ứng dụng Qt 6 trong môi trường này** do không có thư viện Qt 6. Cần build ứng dụng trong Qt Creator / CMake với Qt 6 Widgets và kiểm tra UI trên Windows trước khi nộp.

### Checklist kiểm thử giao diện thủ công trên Windows

1. Đăng nhập, chọn Ghi nhớ tài khoản, thoát rồi mở lại: chỉ tên tài khoản được giữ; mật khẩu rỗng. Bỏ chọn rồi đăng nhập lại để xóa tên đã nhớ.
2. Thu nhỏ cửa sổ: bộ lọc ở cả ba trang quản lý không chồng nhãn, không làm mất nút tìm kiếm/xóa.
3. Chọn học sinh, xem danh sách gia sư, chọn TOP 2 (không phải TOP 1): tạo lớp đúng người đã chọn, điểm thể hiện `/100`, lịch mới không trùng với các lớp trước.
4. Thử xóa môn mà học sinh đang có lớp hoặc đổi lịch gia sư không bao gồm lịch lớp: thao tác phải báo lý do và không lưu.
5. Thử tạo hai lớp của cùng gia sư vào cùng T2: lớp thứ hai phải bị chặn; khác T4 (nếu cả hai còn rảnh) phải được phép.
6. Chuyển lớp sang Hoàn thành, xác nhận số học sinh còn nhận của gia sư thay đổi; không thể bật lại lớp hoàn thành trong form sửa.
7. Đóng và chạy lại ứng dụng: trạng thái và số lớp/học sinh trên dashboard phải đồng bộ với dữ liệu trong file.

## Những việc cần làm nếu dùng với dữ liệu thật

- Xác nhận đơn vị học phí (`đ/giờ`) của dữ liệu được nhập từ phiên bản cũ.
- Nhập giờ học bắt đầu/kết thúc nếu muốn cho gia sư dạy hai lớp cùng ngày nhưng khác ca.
- Tạo bản sao dự phòng thư mục `data` trước khi thay bản mới; không ghi đè dữ liệu người dùng bằng dữ liệu mẫu đi kèm ZIP.
