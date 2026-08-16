#include "MainWindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QVariant>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();

    setWindowTitle("Quản lý gia sư");
    resize(1280, 760);

    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* sidebar = createSidebar();
    mainLayout->addWidget(sidebar, 0);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(createDashboardPage());
    stackedWidget->addWidget(createStudentPage());
    stackedWidget->addWidget(createTutorPage());
    stackedWidget->addWidget(createMatchingPage());
    stackedWidget->addWidget(createClassPage());
    stackedWidget->addWidget(createContractPage());
    stackedWidget->addWidget(createStatisticsPage());

    mainLayout->addWidget(stackedWidget, 1);
    setCentralWidget(central);
}

MainWindow::~MainWindow() {
    for (std::size_t i = 0; i < students.size(); ++i) {
        delete students[i];
    }
    for (std::size_t i = 0; i < tutors.size(); ++i) {
        delete tutors[i];
    }
}

QString MainWindow::joinStrings(const std::vector<std::string>& items) {
    QStringList parts;
    for (const auto& item : items) {
        if (!item.empty()) {
            parts << QString::fromStdString(item);
        }
    }
    return parts.join(", ");
}

QWidget* MainWindow::createSidebar() {
    auto* panel = new QWidget(this);
    panel->setFixedWidth(220);
    panel->setStyleSheet("background: #071827; color: white;");

    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 18, 16, 12);
    layout->setSpacing(10);

    auto* title = new QLabel("QUẢN LÝ GIA SƯ");
    title->setStyleSheet("color: white; font-size: 22px; font-weight: 700; margin-bottom: 8px;");
    layout->addWidget(title);

    const QStringList items = {"Tổng quan", "Học sinh", "Gia sư", "Tìm gia sư", "Lớp học", "Hợp đồng", "Thống kê"};
    for (int i = 0; i < items.size(); ++i) {
        auto* btn = new QPushButton(items[i]);
        btn->setCheckable(true);
        if (i == 0) btn->setChecked(true);
        btn->setStyleSheet(
            "QPushButton { text-align: left; padding: 12px 14px; border-radius: 8px; color: white; background: transparent; border: none; }"
            "QPushButton:checked { background: rgba(255,255,255,0.08); }"
            "QPushButton:hover { background: rgba(255,255,255,0.06); }"
        );
        connect(btn, &QPushButton::clicked, this, [this, i]() { switchPage(i); });
        layout->addWidget(btn);
    }

    layout->addStretch();

    auto* adminLabel = new QLabel("Admin\nQuản trị viên");
    adminLabel->setStyleSheet("color: #dfe7f3; font-size: 12px; line-height: 18px;");
    layout->addWidget(adminLabel);

    auto* logoutButton = new QPushButton("Đăng xuất");
    logoutButton->setStyleSheet("QPushButton { background: #0f172a; border: 1px solid #2d4059; color: white; border-radius: 8px; padding: 10px; }");
    layout->addWidget(logoutButton);

    return panel;
}

QWidget* MainWindow::createDashboardPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(28, 20, 28, 20);
    root->setSpacing(18);

    auto* topBar = new QWidget(page);
    topBar->setFixedHeight(56);
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0, 0, 0, 0);
    auto* search = new QLineEdit(topBar);
    search->setPlaceholderText("Tìm kiếm học sinh...");
    search->setFixedWidth(420);
    topLayout->addWidget(search, 0, Qt::AlignLeft);
    topLayout->addStretch();
    auto* userLabel = new QLabel("Admin User");
    userLabel->setStyleSheet("font-weight: 600; color: #1f2937;");
    topLayout->addWidget(userLabel);
    root->addWidget(topBar);

    auto* statsGrid = new QWidget(page);
    auto* gridLayout = new QGridLayout(statsGrid);
    gridLayout->setHorizontalSpacing(18);
    gridLayout->setVerticalSpacing(18);

    for (int i = 0; i < 4; ++i) {
        auto* card = new QFrame(statsGrid);
        card->setStyleSheet("QFrame { background: #eef2f7; border: 1px solid #e0e7f1; border-radius: 12px; }");
        card->setFixedHeight(104);
        auto* cLayout = new QVBoxLayout(card);
        cLayout->setContentsMargins(18, 16, 18, 16);
        auto* label = new QLabel(i == 0 ? "Tổng học sinh" : i == 1 ? "Tổng gia sư" : i == 2 ? "Lớp đang hoạt động" : "Tỷ lệ ghép thành công");
        label->setStyleSheet("color: #475569; font-size: 14px;");
        auto* value = new QLabel(i == 0 ? "125" : i == 1 ? "68" : i == 2 ? "42" : "78%");
        value->setStyleSheet("font-size: 28px; font-weight: 700; color: #0f172a;");
        cLayout->addWidget(label);
        cLayout->addWidget(value);
        gridLayout->addWidget(card, i / 2, i % 2);
    }

    root->addWidget(statsGrid);

    auto* actions = new QWidget(page);
    auto* actionLayout = new QHBoxLayout(actions);
    actionLayout->setSpacing(12);
    const QStringList actionLabels = {"Thêm học sinh", "Thêm gia sư", "Tìm gia sư", "Tạo lớp học"};
    for (const auto& label : actionLabels) {
        auto* btn = new QPushButton(label);
        btn->setStyleSheet("QPushButton { background: #0d6efd; color: white; border-radius: 8px; padding: 10px 16px; }");
        actionLayout->addWidget(btn);
    }
    root->addWidget(actions);

    auto* bottom = new QWidget(page);
    auto* bottomLayout = new QHBoxLayout(bottom);
    bottomLayout->setSpacing(18);

    auto* leftPanel = new QFrame(bottom);
    leftPanel->setStyleSheet("QFrame { background: #eef2f7; border: 1px solid #e0e7f1; border-radius: 12px; }");
    leftPanel->setFixedHeight(220);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    auto* leftTitle = new QLabel("Trạng thái học sinh");
    leftTitle->setStyleSheet("font-size: 16px; font-weight: 700;");
    leftLayout->addWidget(leftTitle);
    auto* row1 = new QHBoxLayout();
    row1->addWidget(new QLabel("Đã ghép"));
    row1->addStretch();
    row1->addWidget(new QLabel("98"));
    leftLayout->addLayout(row1);
    auto* row2 = new QHBoxLayout();
    row2->addWidget(new QLabel("Chưa ghép"));
    row2->addStretch();
    row2->addWidget(new QLabel("27"));
    leftLayout->addLayout(row2);
    bottomLayout->addWidget(leftPanel, 1);

    auto* rightPanel = new QFrame(bottom);
    rightPanel->setStyleSheet("QFrame { background: #eef2f7; border: 1px solid #e0e7f1; border-radius: 12px; }");
    rightPanel->setFixedHeight(220);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    auto* rightTitle = new QLabel("Trạng thái gia sư");
    rightTitle->setStyleSheet("font-size: 16px; font-weight: 700;");
    rightLayout->addWidget(rightTitle);
    auto* r1 = new QHBoxLayout();
    r1->addWidget(new QLabel("Sẵn sàng"));
    r1->addStretch();
    r1->addWidget(new QLabel("45"));
    rightLayout->addLayout(r1);
    auto* r2 = new QHBoxLayout();
    r2->addWidget(new QLabel("Đang bận"));
    r2->addStretch();
    r2->addWidget(new QLabel("18"));
    rightLayout->addLayout(r2);
    auto* r3 = new QHBoxLayout();
    r3->addWidget(new QLabel("Không hoạt động"));
    r3->addStretch();
    r3->addWidget(new QLabel("5"));
    rightLayout->addLayout(r3);
    bottomLayout->addWidget(rightPanel, 1);

    root->addWidget(bottom);
    return page;
}

QWidget* MainWindow::createStudentPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    auto* header = new QWidget(page);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel("Quản lý học sinh");
    title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1f2937;");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    auto* addBtn = new QPushButton("+ Thêm học sinh");
    addBtn->setStyleSheet("QPushButton { background: #0d6efd; color: white; border-radius: 8px; padding: 10px 16px; }");
    headerLayout->addWidget(addBtn);
    root->addWidget(header);

    auto* filterBar = new QWidget(page);
    auto* filterLayout = new QHBoxLayout(filterBar);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->setSpacing(12);

    auto* search = new QLineEdit(filterBar);
    search->setPlaceholderText("Tìm kiếm học sinh...");
    search->setFixedWidth(320);
    filterLayout->addWidget(search);

    auto* filterClass = new QComboBox(filterBar);
    filterClass->addItem("Tất cả lớp");
    filterClass->addItem("Lớp 10");
    filterClass->addItem("Lớp 11");
    filterClass->addItem("Lớp 12");
    filterLayout->addWidget(filterClass);

    auto* filterStatus = new QComboBox(filterBar);
    filterStatus->addItem("Tất cả trạng thái");
    filterStatus->addItem("Đã ghép");
    filterStatus->addItem("Chưa ghép");
    filterLayout->addWidget(filterStatus);

    filterLayout->addStretch();
    root->addWidget(filterBar);

    auto* table = new QTableWidget(0, 7, page);
    table->setHorizontalHeaderLabels({"Mã HS", "Học sinh", "Lớp & Môn", "Lịch rảnh", "Ngân sách", "Trạng thái", "Thao tác"});
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setStyleSheet("QTableWidget { background: white; border: 1px solid #e2e8f0; border-radius: 10px; gridline-color: #edf2f7; }"
                        "QHeaderView::section { background: #eef2f7; color: #334155; padding: 10px; font-weight: 600; }");

    populateStudentTable(table);
    table->setColumnWidth(0, 90);
    table->setColumnWidth(1, 180);
    table->setColumnWidth(2, 180);
    table->setColumnWidth(3, 160);
    table->setColumnWidth(4, 120);
    table->setColumnWidth(5, 120);
    root->addWidget(table);

    return page;
}

QWidget* MainWindow::createTutorPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(14);

    auto* header = new QWidget(page);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel("Quản lý gia sư");
    title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1f2937;");
    headerLayout->addWidget(title);
    headerLayout->addStretch();

    auto* addBtn = new QPushButton("+ Thêm gia sư");
    addBtn->setStyleSheet("QPushButton { background: #0d6efd; color: white; border-radius: 8px; padding: 10px 16px; }");
    headerLayout->addWidget(addBtn);
    root->addWidget(header);

    auto* filterBar = new QWidget(page);
    auto* filterLayout = new QHBoxLayout(filterBar);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->setSpacing(12);

    auto* search = new QLineEdit(filterBar);
    search->setPlaceholderText("Tìm kiếm gia sư...");
    search->setFixedWidth(320);
    filterLayout->addWidget(search);

    auto* subjectBox = new QComboBox(filterBar);
    subjectBox->addItem("Tất cả môn");
    subjectBox->addItem("Toán");
    subjectBox->addItem("Lý");
    subjectBox->addItem("Hóa");
    filterLayout->addWidget(subjectBox);

    auto* areaBox = new QComboBox(filterBar);
    areaBox->addItem("Tất cả khu vực");
    areaBox->addItem("Hải Châu");
    areaBox->addItem("Thanh Khê");
    areaBox->addItem("Liên Chiểu");
    filterLayout->addWidget(areaBox);

    auto* statusBox = new QComboBox(filterBar);
    statusBox->addItem("Tất cả trạng thái");
    statusBox->addItem("Sẵn sàng");
    statusBox->addItem("Bận");
    statusBox->addItem("Không hoạt động");
    filterLayout->addWidget(statusBox);

    filterLayout->addStretch();
    root->addWidget(filterBar);

    auto* table = new QTableWidget(0, 9, page);
    table->setHorizontalHeaderLabels({"Mã GS", "Gia sư", "Môn dạy", "Kinh nghiệm", "Học phí", "Lịch rảnh", "Khu vực", "Trạng thái", "Thao tác"});
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setStyleSheet("QTableWidget { background: white; border: 1px solid #e2e8f0; border-radius: 10px; gridline-color: #edf2f7; }"
                        "QHeaderView::section { background: #eef2f7; color: #334155; padding: 10px; font-weight: 600; }");

    populateTutorTable(table);

    table->setColumnWidth(0, 80);
    table->setColumnWidth(1, 170);
    table->setColumnWidth(2, 150);
    table->setColumnWidth(3, 120);
    table->setColumnWidth(4, 130);
    table->setColumnWidth(5, 150);
    table->setColumnWidth(6, 120);
    table->setColumnWidth(7, 120);
    root->addWidget(table);

    return page;
}

QWidget* MainWindow::createMatchingPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(18);

    auto* topBar = new QWidget(page);
    auto* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel("Tìm gia sư phù hợp");
    title->setStyleSheet("font-size: 22px; font-weight: 700; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    auto* actionBtn = new QPushButton("Tìm gia sư phù hợp");
    actionBtn->setStyleSheet("QPushButton { background: #1f2937; color: white; border-radius: 8px; padding: 10px 18px; }");
    topLayout->addWidget(actionBtn);
    root->addWidget(topBar);

    auto* selectionRow = new QWidget(page);
    auto* selectionLayout = new QHBoxLayout(selectionRow);
    selectionLayout->setContentsMargins(0, 0, 0, 0);
    selectionLayout->setSpacing(20);

    auto* leftSelect = new QFrame(selectionRow);
    leftSelect->setStyleSheet("QFrame { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }");
    leftSelect->setFixedHeight(120);
    auto* leftLayout = new QVBoxLayout(leftSelect);
    auto* leftTitle = new QLabel("BƯỚC 1: CHỌN HỌC SINH");
    leftTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #1f2937;");
    auto* studentCombo = new QComboBox(leftSelect);
    studentCombo->addItem("SV001 - Nguyễn Minh Anh");
    studentCombo->addItem("SV002 - Trần Văn Bảo");
    studentCombo->addItem("SV003 - Lê Thị Lan");
    leftLayout->addWidget(leftTitle);
    leftLayout->addWidget(studentCombo);
    selectionLayout->addWidget(leftSelect, 1);

    auto* rightSelect = new QFrame(selectionRow);
    rightSelect->setStyleSheet("QFrame { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }");
    rightSelect->setFixedHeight(120);
    auto* rightLayout = new QVBoxLayout(rightSelect);
    auto* rightTitle = new QLabel("YÊU CẦU CỦA HỌC SINH");
    rightTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #0f172a;");
    auto* reqLabel = new QLabel("Môn học: Toán\nKhu vực: Hải Châu\nNgân sách: 200.000đ\nLịch học: T2, T4, T6");
    reqLabel->setStyleSheet("color: #475569; line-height: 1.6;");
    reqLabel->setWordWrap(true);
    rightLayout->addWidget(rightTitle);
    rightLayout->addWidget(reqLabel);
    selectionLayout->addWidget(rightSelect, 1);

    root->addWidget(selectionRow);

    auto* resultArea = new QWidget(page);
    auto* resultLayout = new QHBoxLayout(resultArea);
    resultLayout->setSpacing(18);

    auto* candidate = new QFrame(resultArea);
    candidate->setStyleSheet("QFrame { background: #0f172a; color: white; border-radius: 16px; }");
    candidate->setMinimumHeight(360);
    auto* candidateLayout = new QVBoxLayout(candidate);
    candidateLayout->setContentsMargins(20, 20, 20, 20);
    candidateLayout->setSpacing(10);

    auto* badge = new QLabel("TOP 1");
    badge->setStyleSheet("background: #facc15; color: #111827; padding: 6px 12px; border-radius: 8px; font-weight: 700;");
    badge->setFixedWidth(70);
    candidateLayout->addWidget(badge);

    auto* avatar = new QLabel("Ảnh");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(120, 120);
    avatar->setStyleSheet("background: #dbeafe; border-radius: 60px; color: #1e3a8a; font-weight: 700;");
    candidateLayout->addWidget(avatar, 0, Qt::AlignCenter);

    auto* name = new QLabel("Trần Quốc Huy");
    name->setAlignment(Qt::AlignCenter);
    name->setStyleSheet("font-size: 20px; font-weight: 700; color: white;");
    candidateLayout->addWidget(name);

    auto* code = new QLabel("GS003");
    code->setAlignment(Qt::AlignCenter);
    code->setStyleSheet("font-size: 14px; color: #cbd5e1;");
    candidateLayout->addWidget(code);

    auto* score = new QLabel("95%\nĐỘ PHÙ HỢP");
    score->setAlignment(Qt::AlignCenter);
    score->setStyleSheet("font-size: 28px; font-weight: 700; color: #60a5fa;");
    candidateLayout->addWidget(score);

    resultLayout->addWidget(candidate, 1);

    auto* details = new QFrame(resultArea);
    details->setStyleSheet("QFrame { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }");
    auto* detailsLayout = new QVBoxLayout(details);
    detailsLayout->setContentsMargins(18, 18, 18, 18);
    detailsLayout->setSpacing(12);

    auto* detailTitle = new QLabel("Kinh nghiệm chuyên môn");
    detailTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #1f2937;");
    detailsLayout->addWidget(detailTitle);

    auto* info = new QLabel("Kinh nghiệm: 8 năm\nKhu vực: Hải Châu\nHọc phí: 170k/buổi\nLịch trùng: T2, T4, T6");
    info->setStyleSheet("color: #475569; line-height: 1.8;");
    detailsLayout->addWidget(info);

    auto* breakTitle = new QLabel("VÌ SAO GIA SƯ NÀY PHÙ HỢP?");
    breakTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #1f2937;");
    detailsLayout->addWidget(breakTitle);

    auto* reasons = new QLabel("• Kinh nghiệm chuyên môn 25/25\n• Khu vực địa lý 10/10\n• Học phí yêu cầu 40/45\n• Lịch học trùng khớp 20/20");
    reasons->setStyleSheet("color: #475569; line-height: 1.8;");
    detailsLayout->addWidget(reasons);

    auto* actionsRow = new QHBoxLayout();
    auto* detailButton = new QPushButton("Xem chi tiết");
    detailButton->setStyleSheet("QPushButton { background: #e0f2fe; color: #0f172a; border-radius: 8px; padding: 10px 14px; }");
    auto* chooseButton = new QPushButton("CHỌN GIA SƯ NÀY");
    chooseButton->setStyleSheet("QPushButton { background: #0d6efd; color: white; border-radius: 8px; padding: 10px 18px; }");
    actionsRow->addWidget(detailButton);
    actionsRow->addWidget(chooseButton);
    detailsLayout->addLayout(actionsRow);

    resultLayout->addWidget(details, 2);
    root->addWidget(resultArea);

    return page;
}

QWidget* MainWindow::createClassPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto* top = new QWidget(page);
    auto* topLayout = new QHBoxLayout(top);
    topLayout->setContentsMargins(0, 0, 0, 0);
    auto* title = new QLabel("Quản lý lớp học");
    title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1f2937;");
    topLayout->addWidget(title);
    topLayout->addStretch();
    auto* addBtn = new QPushButton("+ Tạo lớp học");
    addBtn->setStyleSheet("QPushButton { background: #0d6efd; color: white; border-radius: 8px; padding: 10px 16px; }");
    topLayout->addWidget(addBtn);
    root->addWidget(top);

    auto* table = new QTableWidget(4, 10, page);
    table->setHorizontalHeaderLabels({"Mã lớp", "Học sinh", "Gia sư", "Môn", "Lịch học", "Ngày bắt đầu", "Ngày kết thúc", "Học phí", "Trạng thái", "Thao tác"});
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setStyleSheet("QTableWidget { background: white; border: 1px solid #e2e8f0; border-radius: 10px; gridline-color: #edf2f7; }"
                        "QHeaderView::section { background: #eef2f7; color: #334155; padding: 10px; font-weight: 600; }");

    const QString rows[4][10] = {
        {"CL001", "Nguyễn Minh Anh", "Trần Quốc Huy", "Toán", "T2,T4,T6", "01/09/2026", "30/06/2027", "200k", "Active", "Xem"},
        {"CL002", "Trần Văn Bảo", "Nguyễn Văn B", "Lý", "T3,T5", "10/09/2026", "31/12/2026", "250k", "Pending", "Xem"},
        {"CL003", "Lê Thị Lan", "Lê Minh C", "Văn", "T2,T5,T7", "01/10/2026", "30/06/2027", "300k", "Completed", "Xem"},
        {"CL004", "Phạm Quốc Huy", "Phạm Văn D", "Hóa", "T4,T6", "15/09/2026", "15/01/2027", "180k", "Active", "Xem"},
    };

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 10; ++j) {
            auto* item = new QTableWidgetItem(rows[i][j]);
            table->setItem(i, j, item);
        }
    }

    table->setColumnWidth(0, 90);
    table->setColumnWidth(1, 150);
    table->setColumnWidth(2, 150);
    table->setColumnWidth(3, 100);
    table->setColumnWidth(4, 120);
    table->setColumnWidth(5, 120);
    table->setColumnWidth(6, 120);
    table->setColumnWidth(7, 100);
    table->setColumnWidth(8, 100);

    root->addWidget(table);
    return page;
}

QWidget* MainWindow::createContractPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto* top = new QWidget(page);
    auto* topLayout = new QHBoxLayout(top);
    topLayout->setContentsMargins(0, 0, 0, 0);
    auto* title = new QLabel("Quản lý hợp đồng");
    title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1f2937;");
    topLayout->addWidget(title);
    topLayout->addStretch();
    auto* addBtn = new QPushButton("+ Tạo hợp đồng");
    addBtn->setStyleSheet("QPushButton { background: #0d6efd; color: white; border-radius: 8px; padding: 10px 16px; }");
    topLayout->addWidget(addBtn);
    root->addWidget(top);

    auto* table = new QTableWidget(4, 9, page);
    table->setHorizontalHeaderLabels({"Mã HĐ", "Mã lớp", "Học sinh", "Gia sư", "Ngày bắt đầu", "Ngày kết thúc", "Tổng phí", "Trạng thái", "Thao tác"});
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setStyleSheet("QTableWidget { background: white; border: 1px solid #e2e8f0; border-radius: 10px; gridline-color: #edf2f7; }"
                        "QHeaderView::section { background: #eef2f7; color: #334155; padding: 10px; font-weight: 600; }");

    const QString rows[4][9] = {
        {"HD001", "CL001", "Nguyễn Minh Anh", "Trần Quốc Huy", "01/09/2026", "30/06/2027", "200.000đ", "Active", "Xem"},
        {"HD002", "CL002", "Trần Văn Bảo", "Nguyễn Văn B", "10/09/2026", "31/12/2026", "250.000đ", "Pending", "Xem"},
        {"HD003", "CL003", "Lê Thị Lan", "Lê Minh C", "01/10/2026", "30/06/2027", "300.000đ", "Completed", "Xem"},
        {"HD004", "CL004", "Phạm Quốc Huy", "Phạm Văn D", "15/09/2026", "15/01/2027", "180.000đ", "Active", "Xem"},
    };

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 9; ++j) {
            auto* item = new QTableWidgetItem(rows[i][j]);
            table->setItem(i, j, item);
        }
    }

    table->setColumnWidth(0, 90);
    table->setColumnWidth(1, 90);
    table->setColumnWidth(2, 170);
    table->setColumnWidth(3, 170);
    table->setColumnWidth(4, 120);
    table->setColumnWidth(5, 120);
    table->setColumnWidth(6, 110);
    table->setColumnWidth(7, 100);
    root->addWidget(table);

    return page;
}

QWidget* MainWindow::createStatisticsPage() {
    auto* page = new QWidget(this);
    page->setStyleSheet("background: #f5f7fb;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(18);

    auto* title = new QLabel("Thống kê hệ thống");
    title->setStyleSheet("font-size: 20px; font-weight: 700; color: #1f2937;");
    root->addWidget(title);

    auto* statsGrid = new QWidget(page);
    auto* gridLayout = new QGridLayout(statsGrid);
    gridLayout->setHorizontalSpacing(18);
    gridLayout->setVerticalSpacing(18);

    const QStringList labels = {"Tổng học sinh", "Tổng gia sư", "Lớp đang hoạt động", "Tỷ lệ ghép thành công"};
    const QStringList values = {"125", "68", "42", "78%"};
    for (int i = 0; i < labels.size(); ++i) {
        auto* card = new QFrame(statsGrid);
        card->setStyleSheet("QFrame { background: #eef2f7; border: 1px solid #e0e7f1; border-radius: 12px; }");
        card->setFixedHeight(110);
        auto* cLayout = new QVBoxLayout(card);
        cLayout->setContentsMargins(16, 14, 16, 14);
        auto* lab = new QLabel(labels[i]);
        lab->setStyleSheet("color: #475569; font-size: 14px;");
        auto* val = new QLabel(values[i]);
        val->setStyleSheet("font-size: 28px; font-weight: 700; color: #0f172a;");
        cLayout->addWidget(lab);
        cLayout->addWidget(val);
        gridLayout->addWidget(card, i / 2, i % 2);
    }
    root->addWidget(statsGrid);

    auto* bottom = new QWidget(page);
    auto* bottomLayout = new QHBoxLayout(bottom);
    bottomLayout->setSpacing(16);

    auto* leftCard = new QFrame(bottom);
    leftCard->setStyleSheet("QFrame { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }");
    leftCard->setMinimumHeight(240);
    auto* leftLayout = new QVBoxLayout(leftCard);
    leftLayout->addWidget(new QLabel("Tình trạng ghép nối"));
    auto* ring = new QLabel("86%\nĐã ghép" );
    ring->setAlignment(Qt::AlignCenter);
    ring->setStyleSheet("font-size: 26px; font-weight: 700; color: #0f172a;");
    leftLayout->addWidget(ring, 0, Qt::AlignCenter);
    bottomLayout->addWidget(leftCard, 1);

    auto* rightCard = new QFrame(bottom);
    rightCard->setStyleSheet("QFrame { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }");
    rightCard->setMinimumHeight(240);
    auto* rightLayout = new QVBoxLayout(rightCard);
    rightLayout->addWidget(new QLabel("Trạng thái lớp học"));
    rightLayout->addWidget(new QLabel("• Lớp đã có gia sư: 1,420\n• Đang chờ ghép: 231\n• Hoàn thành: 86%"));
    bottomLayout->addWidget(rightCard, 1);

    root->addWidget(bottom);
    return page;
}

void MainWindow::populateStudentTable(QTableWidget* table) {
    table->setRowCount(static_cast<int>(students.size()));
    for (std::size_t i = 0; i < students.size(); ++i) {
        Student* student = students[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(student->getPersonID())));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(student->getFullName())));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(student->getGrade() + " - " + joinStrings(student->getNeedSubjects()).toStdString())));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(student->getAvailableSchedule())));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::fromStdString(student->getBudgetPerHour())));
        table->setItem(static_cast<int>(i), 5, new QTableWidgetItem(student->getIsMatched() ? "Đã ghép" : "Chưa ghép"));
        table->setItem(static_cast<int>(i), 6, new QTableWidgetItem("Xem"));
    }
}

void MainWindow::populateTutorTable(QTableWidget* table) {
    table->setRowCount(static_cast<int>(tutors.size()));
    for (std::size_t i = 0; i < tutors.size(); ++i) {
        Tutor* tutor = tutors[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(tutor->getPersonID())));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(tutor->getFullName())));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(joinStrings(tutor->getSubjects())));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(tutor->getYearsOfExperience()) + " năm"));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::fromStdString(tutor->getRatePerHour())));
        table->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::fromStdString(tutor->getAvailability())));
        table->setItem(static_cast<int>(i), 6, new QTableWidgetItem(joinStrings(tutor->getTeachingAreas())));
        table->setItem(static_cast<int>(i), 7, new QTableWidgetItem(tutor->getIsAvailable() ? "Sẵn sàng" : "Bận"));
        table->setItem(static_cast<int>(i), 8, new QTableWidgetItem("Xem"));
    }
}

void MainWindow::switchPage(int index) {
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(index);
    }
}
