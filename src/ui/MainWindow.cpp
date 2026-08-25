#include "MainWindow.h"

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QComboBox>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QLabel* makeLabel(const QString& text, const QString& style = QString()) {
    auto* label = new QLabel(text);
    if (!style.isEmpty()) {
        label->setStyleSheet(style);
    }
    return label;
}

QFrame* makeLine(QWidget* parent = nullptr) {
    auto* line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setStyleSheet("background: #e8e7eb; border: none;");
    return line;
}

QProgressBar* makeProgress(int value, const QString& chunkColor, int height = 5) {
    auto* bar = new QProgressBar();
    bar->setRange(0, 100);
    bar->setValue(value);
    bar->setTextVisible(false);
    bar->setFixedHeight(height);
    bar->setStyleSheet(
        QString(
            "QProgressBar { background: #dedee3; border: none; border-radius: %1px; }"
            "QProgressBar::chunk { background: %2; border-radius: %1px; }")
            .arg(height / 2)
            .arg(chunkColor));
    return bar;
}

QWidget* makeStatusDot(const QString& color, const QString& text, const QString& value) {
    auto* row = new QWidget();
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* dot = new QFrame(row);
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QString("background: %1; border: none; border-radius: 4px;").arg(color));

    auto* label = makeLabel(text, "color: #292832; font-size: 12px;");
    auto* count = makeLabel(value, "color: #191821; font-size: 12px; font-weight: 700;");

    layout->addWidget(dot);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(count);
    return row;
}

QLabel* makeBadge(const QString& text, const QString& bg, const QString& fg) {
    auto* badge = new QLabel(text);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(QString(
        "background: %1; color: %2; border: none; border-radius: 6px; "
        "padding: 4px 7px; font-size: 10px; font-weight: 600;")
        .arg(bg, fg));
    badge->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    return badge;
}

QWidget* makeStudentCell(const QString& initials, const QString& name,
                         const QString& avatarBg, const QString& avatarFg) {
    auto* cell = new QWidget();
    auto* layout = new QHBoxLayout(cell);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* avatar = new QLabel(initials);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(25, 25);
    avatar->setStyleSheet(QString(
        "background: %1; color: %2; border: none; border-radius: 12px; "
        "font-size: 9px; font-weight: 700;")
        .arg(avatarBg, avatarFg));

    auto* nameLabel = makeLabel(name, "color: #22212a; font-size: 11px;");
    layout->addWidget(avatar);
    layout->addWidget(nameLabel);
    layout->addStretch();
    return cell;
}

QFrame* makeStatCard(const QString& title,
                     const QString& value,
                     const QString& iconText,
                     const QString& iconBg,
                     const QString& iconFg,
                     const QString& delta,
                     const QString& deltaColor,
                     bool showProgress = false) {
    auto* card = new QFrame();
    card->setObjectName("dashboardStatCard");
    card->setMinimumHeight(126);
    card->setStyleSheet(
        "QFrame#dashboardStatCard { background: #f0eef0; border: none; border-radius: 4px; }");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 15, 14, 13);
    layout->setSpacing(3);

    auto* topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(8);

    auto* textCol = new QVBoxLayout();
    textCol->setContentsMargins(0, 0, 0, 0);
    textCol->setSpacing(1);
    textCol->addWidget(makeLabel(title,
        "color: #6b6871; font-size: 12px; font-weight: 500;"));
    textCol->addWidget(makeLabel(value,
        "color: #17161c; font-size: 22px; font-weight: 700;"));

    auto* icon = new QLabel(iconText);
    icon->setAlignment(Qt::AlignCenter);
    icon->setFixedSize(38, 38);
    icon->setStyleSheet(QString(
        "background: %1; color: %2; border: none; border-radius: 8px; "
        "font-size: 12px; font-weight: 800;")
        .arg(iconBg, iconFg));

    topRow->addLayout(textCol, 1);
    topRow->addWidget(icon, 0, Qt::AlignTop);
    layout->addLayout(topRow);
    layout->addStretch();

    if (showProgress) {
        layout->addWidget(makeProgress(78, "#0867ba", 5));
    } else {
        auto* deltaLabel = makeLabel(delta + "  so với tháng trước",
            QString("color: %1; font-size: 9px;").arg(deltaColor));
        layout->addWidget(deltaLabel);
    }

    return card;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();

    setWindowTitle("Quản lý gia sư");
    resize(1280, 800);
    setMinimumSize(1050, 690);

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
    panel->setFixedWidth(232);
    panel->setObjectName("mainSidebar");
    panel->setStyleSheet("QWidget#mainSidebar { background: #061323; color: white; }");

    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(14, 14, 14, 12);
    layout->setSpacing(4);

    auto* brandRow = new QWidget(panel);
    auto* brandLayout = new QHBoxLayout(brandRow);
    brandLayout->setContentsMargins(9, 3, 4, 13);
    brandLayout->setSpacing(8);

    auto* brandIcon = new QLabel("◇");
    brandIcon->setAlignment(Qt::AlignCenter);
    brandIcon->setFixedSize(20, 20);
    brandIcon->setStyleSheet("color: #2d8cff; font-size: 18px; font-weight: 800;");
    auto* brandTitle = new QLabel("QUẢN LÝ GIA SƯ");
    brandTitle->setStyleSheet("color: #ffffff; font-size: 12px; font-weight: 800;");
    brandLayout->addWidget(brandIcon);
    brandLayout->addWidget(brandTitle);
    brandLayout->addStretch();
    layout->addWidget(brandRow);

    auto* group = new QButtonGroup(panel);
    group->setExclusive(true);

    const QStringList labels = {
        "▦   Tổng quan",
        "♙   Học sinh",
        "♟   Gia sư",
        "⌕   Tìm gia sư",
        "▣   Lớp học",
        "▤   Hợp đồng",
        "▥   Thống kê"
    };

    for (int i = 0; i < labels.size(); ++i) {
        auto* btn = new QPushButton(labels[i], panel);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(44);
        btn->setStyleSheet(
            "QPushButton { text-align: left; padding: 0 10px; color: #f4f7fb; "
            "background: transparent; border: none; border-radius: 4px; font-size: 12px; }"
            "QPushButton:hover { background: #0b1d31; }"
            "QPushButton:checked { color: #ffffff; font-weight: 700; background: #07192c; }"
        );
        if (i == 0) {
            btn->setChecked(true);
        }
        group->addButton(btn, i);
        connect(btn, &QPushButton::clicked, this, [this, i]() { switchPage(i); });
        layout->addWidget(btn);
    }

    layout->addStretch();

    auto* userBox = new QWidget(panel);
    auto* userLayout = new QHBoxLayout(userBox);
    userLayout->setContentsMargins(7, 8, 7, 9);
    userLayout->setSpacing(8);

    auto* avatar = new QLabel("A");
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(28, 28);
    avatar->setStyleSheet(
        "background: #237bd2; color: white; border-radius: 14px; "
        "font-weight: 700; font-size: 11px;");

    auto* userText = new QLabel("Admin\nQuản trị viên");
    userText->setStyleSheet("color: #ffffff; font-size: 10px; font-weight: 600;");
    userLayout->addWidget(avatar);
    userLayout->addWidget(userText);
    userLayout->addStretch();
    layout->addWidget(userBox);

    auto* logoutButton = new QPushButton("↪  Đăng xuất", panel);
    logoutButton->setFixedHeight(36);
    logoutButton->setCursor(Qt::PointingHandCursor);
    logoutButton->setStyleSheet(
        "QPushButton { background: #112137; color: white; border: none; border-radius: 3px; "
        "font-size: 10px; font-weight: 600; }"
        "QPushButton:hover { background: #182d48; }"
    );
    layout->addWidget(logoutButton);

    return panel;
}

QWidget* MainWindow::createDashboardPage() {
    auto* page = new QWidget(this);
    page->setObjectName("dashboardPage");
    page->setStyleSheet("QWidget#dashboardPage { background: #fbfafc; }");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header giống ảnh mẫu: ô tìm kiếm bên trái, chuông và tài khoản bên phải.
    auto* header = new QWidget(page);
    header->setFixedHeight(52);
    header->setStyleSheet("background: #ffffff; border-bottom: 1px solid #ecebf0;");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(22, 8, 18, 8);
    headerLayout->setSpacing(10);

    auto* search = new QLineEdit(header);
    search->setPlaceholderText("⌕  Tìm kiếm hệ thống...");
    search->setFixedWidth(315);
    search->setFixedHeight(32);
    search->setStyleSheet(
        "QLineEdit { background: #f1f0f3; color: #33313a; border: none; border-radius: 4px; "
        "padding: 0 12px; font-size: 11px; }"
        "QLineEdit::placeholder { color: #9a98a1; }");
    headerLayout->addWidget(search);
    headerLayout->addStretch();

    auto* bell = new QPushButton("♧", header);
    bell->setFixedSize(32, 32);
    bell->setStyleSheet(
        "QPushButton { background: transparent; color: #55525a; border: none; font-size: 17px; }"
        "QPushButton:hover { background: #f1f0f3; border-radius: 16px; }");
    headerLayout->addWidget(bell);

    auto* sep = new QFrame(header);
    sep->setFixedSize(1, 30);
    sep->setStyleSheet("background: #ebe9ed; border: none;");
    headerLayout->addWidget(sep);

    auto* account = new QWidget(header);
    auto* accountLayout = new QHBoxLayout(account);
    accountLayout->setContentsMargins(0, 0, 0, 0);
    accountLayout->setSpacing(8);

    auto* accountText = new QLabel("Admin User\n<span style='color:#8b8992; font-size:9px;'>Sẵn sàng</span>");
    accountText->setTextFormat(Qt::RichText);
    accountText->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    accountText->setStyleSheet("color: #3b3941; font-size: 11px; font-weight: 600;");
    auto* accountAvatar = new QLabel("●");
    accountAvatar->setAlignment(Qt::AlignCenter);
    accountAvatar->setFixedSize(32, 32);
    accountAvatar->setStyleSheet(
        "background: #171924; color: white; border-radius: 16px; font-size: 12px;");
    accountLayout->addWidget(accountText);
    accountLayout->addWidget(accountAvatar);
    headerLayout->addWidget(account);
    root->addWidget(header);

    auto* content = new QWidget(page);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 18, 24, 18);
    contentLayout->setSpacing(14);

    // Tiêu đề + nhóm nút thao tác.
    auto* headingRow = new QWidget(content);
    auto* headingLayout = new QHBoxLayout(headingRow);
    headingLayout->setContentsMargins(0, 0, 0, 0);
    headingLayout->setSpacing(8);

    auto* headingText = new QVBoxLayout();
    headingText->setContentsMargins(0, 0, 0, 0);
    headingText->setSpacing(1);
    headingText->addWidget(makeLabel("BÁO CÁO TỔNG HỢP",
        "color: #435267; font-size: 9px; font-weight: 700;"));
    headingText->addWidget(makeLabel("Tổng quan hệ thống",
        "color: #111016; font-size: 20px; font-weight: 800;"));
    headingLayout->addLayout(headingText);
    headingLayout->addStretch();

    const QString actionStylePrimary =
        "QPushButton { background: #0c0d17; color: white; border: 1px solid #0c0d17; "
        "border-radius: 2px; padding: 8px 13px; font-size: 10px; font-weight: 700; }"
        "QPushButton:hover { background: #20222f; }";
    const QString actionStyleBlue =
        "QPushButton { background: #0873c8; color: white; border: 1px solid #0873c8; "
        "border-radius: 2px; padding: 8px 13px; font-size: 10px; font-weight: 700; }"
        "QPushButton:hover { background: #0967b0; }";
    const QString actionStyleOutline =
        "QPushButton { background: #ffffff; color: #0666b3; border: 1px solid #6ca8dc; "
        "border-radius: 2px; padding: 8px 13px; font-size: 10px; font-weight: 600; }"
        "QPushButton:hover { background: #f2f8fd; }";

    auto* addStudentBtn = new QPushButton("♙  Thêm học sinh", headingRow);
    auto* addTutorBtn = new QPushButton("♟  Thêm gia sư", headingRow);
    auto* findTutorBtn = new QPushButton("⌕  Tìm gia sư", headingRow);
    auto* createClassBtn = new QPushButton("▣  Tạo lớp học", headingRow);
    addStudentBtn->setStyleSheet(actionStylePrimary);
    addTutorBtn->setStyleSheet(actionStyleBlue);
    findTutorBtn->setStyleSheet(actionStyleOutline);
    createClassBtn->setStyleSheet(actionStyleOutline);
    addStudentBtn->setCursor(Qt::PointingHandCursor);
    addTutorBtn->setCursor(Qt::PointingHandCursor);
    findTutorBtn->setCursor(Qt::PointingHandCursor);
    createClassBtn->setCursor(Qt::PointingHandCursor);

    connect(addStudentBtn, &QPushButton::clicked, this, [this]() { switchPage(1); });
    connect(addTutorBtn, &QPushButton::clicked, this, [this]() { switchPage(2); });
    connect(findTutorBtn, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(createClassBtn, &QPushButton::clicked, this, [this]() { switchPage(4); });

    headingLayout->addWidget(addStudentBtn);
    headingLayout->addWidget(addTutorBtn);
    headingLayout->addWidget(findTutorBtn);
    headingLayout->addWidget(createClassBtn);
    contentLayout->addWidget(headingRow);

    // 4 thẻ thống kê nằm trên cùng một hàng như ảnh mẫu.
    auto* statsRow = new QWidget(content);
    auto* statsLayout = new QHBoxLayout(statsRow);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(10);
    statsLayout->addWidget(makeStatCard("Tổng học sinh", "125", "HS", "#e1e2e7", "#0a0b11", "+12%", "#0475c5"), 1);
    statsLayout->addWidget(makeStatCard("Tổng gia sư", "68", "GS", "#d9e9f7", "#0872c3", "+5%", "#0475c5"), 1);
    statsLayout->addWidget(makeStatCard("Lớp đang hoạt động", "42", "LH", "#e5dedc", "#24140f", "-2%", "#d42828"), 1);
    statsLayout->addWidget(makeStatCard("Tỷ lệ ghép thành\ncông", "78%", "%", "#dce9f6", "#075ea3", QString(), "#0475c5", true), 1);
    contentLayout->addWidget(statsRow);

    // Khu vực chính: cột trái trạng thái, cột phải lớp gần đây + thông báo.
    auto* mainBody = new QWidget(content);
    auto* bodyLayout = new QHBoxLayout(mainBody);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(12);

    auto* leftColumn = new QWidget(mainBody);
    leftColumn->setMaximumWidth(315);
    auto* leftLayout = new QVBoxLayout(leftColumn);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    auto* studentStatus = new QFrame(leftColumn);
    studentStatus->setObjectName("studentStatusCard");
    studentStatus->setStyleSheet(
        "QFrame#studentStatusCard { background: #f0eef0; border: none; border-radius: 4px; }");
    auto* studentLayout = new QVBoxLayout(studentStatus);
    studentLayout->setContentsMargins(16, 15, 16, 14);
    studentLayout->setSpacing(9);
    studentLayout->addWidget(makeLabel("Trạng thái học sinh",
        "color: #18171d; font-size: 12px; font-weight: 700;"));
    studentLayout->addWidget(makeLine(studentStatus));
    studentLayout->addWidget(makeStatusDot("#0672c7", "Đã ghép", "98"));
    studentLayout->addWidget(makeProgress(78, "#0672c7", 5));
    studentLayout->addWidget(makeStatusDot("#878991", "Chưa ghép", "27"));
    studentLayout->addWidget(makeProgress(22, "#84868d", 5));
    leftLayout->addWidget(studentStatus, 1);

    auto* tutorStatus = new QFrame(leftColumn);
    tutorStatus->setObjectName("tutorStatusCard");
    tutorStatus->setStyleSheet(
        "QFrame#tutorStatusCard { background: #f0eef0; border: none; border-radius: 4px; }");
    auto* tutorLayout = new QVBoxLayout(tutorStatus);
    tutorLayout->setContentsMargins(16, 15, 16, 14);
    tutorLayout->setSpacing(9);
    tutorLayout->addWidget(makeLabel("Trạng thái gia sư",
        "color: #18171d; font-size: 12px; font-weight: 700;"));
    tutorLayout->addWidget(makeLine(tutorStatus));
    tutorLayout->addWidget(makeStatusDot("#0871c4", "Sẵn sàng", "45"));
    tutorLayout->addWidget(makeStatusDot("#24120f", "Bận", "18"));
    tutorLayout->addWidget(makeStatusDot("#8b8c92", "Không hoạt động", "5"));
    tutorLayout->addStretch();

    auto* miniBars = new QWidget(tutorStatus);
    auto* miniBarsLayout = new QHBoxLayout(miniBars);
    miniBarsLayout->setContentsMargins(0, 4, 0, 0);
    miniBarsLayout->setSpacing(7);
    auto* b1 = new QFrame(miniBars);
    b1->setFixedSize(63, 48);
    b1->setStyleSheet("background: #0871c4; border: none;");
    auto* b2 = new QFrame(miniBars);
    b2->setFixedSize(63, 22);
    b2->setStyleSheet("background: #25120f; border: none;");
    auto* b3 = new QFrame(miniBars);
    b3->setFixedSize(63, 8);
    b3->setStyleSheet("background: #86888f; border: none;");
    miniBarsLayout->addWidget(b1, 0, Qt::AlignBottom);
    miniBarsLayout->addWidget(b2, 0, Qt::AlignBottom);
    miniBarsLayout->addWidget(b3, 0, Qt::AlignBottom);
    miniBarsLayout->addStretch();
    tutorLayout->addWidget(miniBars);
    leftLayout->addWidget(tutorStatus, 1);
    bodyLayout->addWidget(leftColumn, 32);

    auto* rightColumn = new QWidget(mainBody);
    auto* rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto* classesCard = new QFrame(rightColumn);
    classesCard->setObjectName("recentClassesCard");
    classesCard->setStyleSheet(
        "QFrame#recentClassesCard { background: #f0eef0; border: none; border-radius: 4px; }");
    auto* classesLayout = new QVBoxLayout(classesCard);
    classesLayout->setContentsMargins(14, 13, 14, 10);
    classesLayout->setSpacing(8);

    auto* classesHeader = new QHBoxLayout();
    classesHeader->setContentsMargins(0, 0, 0, 0);
    classesHeader->addWidget(makeLabel("Lớp học gần đây",
        "color: #18171d; font-size: 12px; font-weight: 700;"));
    classesHeader->addStretch();
    auto* seeAll = new QPushButton("XEM TẤT CẢ", classesCard);
    seeAll->setStyleSheet(
        "QPushButton { background: transparent; color: #0065b4; border: none; "
        "font-size: 9px; font-weight: 800; }"
        "QPushButton:hover { text-decoration: underline; }");
    classesHeader->addWidget(seeAll);
    classesLayout->addLayout(classesHeader);
    classesLayout->addWidget(makeLine(classesCard));

    auto* table = new QTableWidget(4, 6, classesCard);
    table->setHorizontalHeaderLabels({"MÃ\nLỚP", "HỌC SINH", "GIA SƯ", "MÔN\nHỌC", "TRẠNG\nTHÁI", "THAO\nTÁC"});
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setFrameShape(QFrame::Box);
    table->setLineWidth(1);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setAlternatingRowColors(false);
    table->setStyleSheet(
        "QTableWidget { background: #ffffff; border: 1px solid #d8d7dc; color: #2b2930; font-size: 10px; }"
        "QHeaderView::section { background: #f0eef0; color: #62606a; border: none; "
        "padding: 4px 3px; font-size: 8px; font-weight: 800; }"
        "QTableWidget::item { border: none; padding: 3px; }"
    );
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setColumnWidth(0, 66);
    table->setColumnWidth(1, 150);
    table->setColumnWidth(2, 120);
    table->setColumnWidth(3, 95);
    table->setColumnWidth(4, 95);
    table->setColumnWidth(5, 60);
    table->horizontalHeader()->setMinimumSectionSize(45);
    table->verticalHeader()->setDefaultSectionSize(48);
    table->setFixedHeight(242);

    struct RecentClassRow {
        QString code;
        QString initials;
        QString student;
        QString tutor;
        QString subject;
        QString status;
        QString statusBg;
        QString statusFg;
        QString avatarBg;
        QString avatarFg;
    };

    const RecentClassRow rows[] = {
        {"LH-\n1042", "NA", "Nguyễn Văn A", "Trần\nThị B", "Toán\nLớp 10", "Đang\nhọc", "#d9e9f7", "#075f9e", "#0f3d68", "#ffffff"},
        {"LH-\n1043", "LM", "Lê Hoàng M", "Phạm\nVăn C", "Tiếng\nAnh\nIELTS", "Chờ\nxếp\nlịch", "#e6e6e8", "#52515a", "#6a260e", "#ffffff"},
        {"LH-\n1044", "VT", "Vũ Minh T", "Chưa\ncó", "Vật Lý\n12", "Cần\ngia\nsư", "#f8dddd", "#c12c2c", "#ffd3d3", "#cf3636"},
        {"LH-\n1045", "HK", "Hoàng Tú K", "Ngô\nPhương\nT", "Hóa\nLớp 9", "Đang\nhọc", "#d9e9f7", "#075f9e", "#49a7f4", "#ffffff"}
    };

    for (int r = 0; r < 4; ++r) {
        auto* codeItem = new QTableWidgetItem(rows[r].code);
        codeItem->setForeground(QColor("#0067b6"));
        QFont codeFont = codeItem->font();
        codeFont.setBold(true);
        codeItem->setFont(codeFont);
        codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setItem(r, 0, codeItem);
        table->setCellWidget(r, 1, makeStudentCell(rows[r].initials, rows[r].student, rows[r].avatarBg, rows[r].avatarFg));
        table->setItem(r, 2, new QTableWidgetItem(rows[r].tutor));
        table->setItem(r, 3, new QTableWidgetItem(rows[r].subject));
        table->setCellWidget(r, 4, makeBadge(rows[r].status, rows[r].statusBg, rows[r].statusFg));
        auto* more = new QLabel("⋯");
        more->setAlignment(Qt::AlignCenter);
        more->setStyleSheet("color: #6d6b72; font-size: 15px;");
        table->setCellWidget(r, 5, more);
    }

    classesLayout->addWidget(table);
    rightLayout->addWidget(classesCard, 1);

    auto* newsCard = new QFrame(rightColumn);
    newsCard->setObjectName("newsCard");
    newsCard->setStyleSheet(
        "QFrame#newsCard { background: #f0eef0; border: none; border-radius: 4px; }");
    auto* newsLayout = new QHBoxLayout(newsCard);
    newsLayout->setContentsMargins(16, 12, 16, 12);
    newsLayout->setSpacing(12);

    auto* newsText = new QVBoxLayout();
    newsText->setContentsMargins(0, 0, 0, 0);
    newsText->setSpacing(4);
    newsText->addWidget(makeLabel("Chương trình đào tạo mới",
        "color: #222129; font-size: 11px; font-weight: 700;"));
    auto* newsDesc = makeLabel(
        "Cập nhật tài liệu giảng dạy môn Toán cấp 3 theo chương trình\n"
        "GDPT 2018. Vui lòng thông báo cho các gia sư liên quan.",
        "color: #66646c; font-size: 9px;");
    newsDesc->setWordWrap(true);
    newsText->addWidget(newsDesc);
    newsLayout->addLayout(newsText, 1);

    // Khối QR giả lập bằng các ô vuông nhỏ, giúp bố cục giống ảnh tham chiếu.
    auto* qr = new QFrame(newsCard);
    qr->setFixedSize(74, 58);
    qr->setStyleSheet("background: #ffffff; border: none; padding: 3px;");
    auto* qrGrid = new QGridLayout(qr);
    qrGrid->setContentsMargins(5, 5, 5, 5);
    qrGrid->setSpacing(1);
    const int pattern[7][9] = {
        {1,1,1,0,1,0,1,1,1},
        {1,0,1,1,0,1,1,0,1},
        {1,1,1,0,1,0,1,1,1},
        {0,1,0,1,1,1,0,1,0},
        {1,0,1,1,0,1,1,0,1},
        {0,1,1,0,1,0,0,1,1},
        {1,1,0,1,0,1,1,1,0}
    };
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto* px = new QFrame(qr);
            px->setFixedSize(5, 5);
            px->setStyleSheet(pattern[r][c]
                ? "background: #0b6fbd; border: none;"
                : "background: transparent; border: none;");
            qrGrid->addWidget(px, r, c);
        }
    }
    newsLayout->addWidget(qr, 0, Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->addWidget(newsCard, 0);

    bodyLayout->addWidget(rightColumn, 68);
    contentLayout->addWidget(mainBody, 1);
    root->addWidget(content, 1);

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
    reqLabel->setStyleSheet("color: #475569;");
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
    info->setStyleSheet("color: #475569;");
    detailsLayout->addWidget(info);

    auto* breakTitle = new QLabel("VÌ SAO GIA SƯ NÀY PHÙ HỢP?");
    breakTitle->setStyleSheet("font-size: 14px; font-weight: 700; color: #1f2937;");
    detailsLayout->addWidget(breakTitle);

    auto* reasons = new QLabel("• Kinh nghiệm chuyên môn 25/25\n• Khu vực địa lý 10/10\n• Học phí yêu cầu 40/45\n• Lịch học trùng khớp 20/20");
    reasons->setStyleSheet("color: #475569;");
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
    page->setStyleSheet("background: #fbfafc;");

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(14, 12, 14, 12);
    root->setSpacing(10);

    auto* header = new QWidget(page);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    auto* heading = new QVBoxLayout();
    heading->setContentsMargins(0, 0, 0, 0);
    heading->setSpacing(2);
    auto* eyebrow = new QLabel("BÁO CÁO VẬN HÀNH");
    eyebrow->setStyleSheet("color: #526176; font-size: 9px; font-weight: 700;");
    auto* title = new QLabel("Thống kê hệ thống");
    title->setStyleSheet("color: #111016; font-size: 20px; font-weight: 800;");
    heading->addWidget(eyebrow);
    heading->addWidget(title);
    headerLayout->addLayout(heading);
    headerLayout->addStretch();
    auto* period = new QComboBox(header);
    period->addItem("30 ngày qua");
    period->setFixedSize(120, 32);
    period->setStyleSheet("QComboBox { background: white; border: 1px solid #d8d7dc; border-radius: 3px; padding: 0 8px; font-size: 10px; }");
    auto* exportButton = new QPushButton("Xuất báo cáo", header);
    exportButton->setFixedSize(112, 32);
    exportButton->setStyleSheet("QPushButton { background: #0873c8; color: white; border: none; border-radius: 3px; font-size: 10px; font-weight: 700; }");
    headerLayout->addWidget(period);
    headerLayout->addWidget(exportButton);
    root->addWidget(header);

    auto* statsRow = new QWidget(page);
    auto* statsLayout = new QHBoxLayout(statsRow);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(8);
    const QStringList labels = {"Tổng học sinh", "Tổng gia sư", "Gia sư sẵn sàng nhận lớp", "Lớp đang hoạt động", "Tỷ lệ ghép thành công"};
    const QStringList values = {"2,543", "842", "315", "1,120", "86%"};
    const QStringList deltas = {"+12%", "+5%", "LIVE", "+18%", "Cao"};
    for (int i = 0; i < labels.size(); ++i) {
        auto* card = new QFrame(statsRow);
        card->setFixedHeight(92);
        card->setStyleSheet(i == 2
            ? "QFrame { background: #111621; border: none; border-radius: 4px; }"
            : "QFrame { background: #f0eef0; border: none; border-radius: 4px; }");
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 9);
        auto* label = new QLabel(labels[i]);
        auto* value = new QLabel(values[i]);
        auto* delta = new QLabel(deltas[i]);
        const QString fg = i == 2 ? "#ffffff" : "#17161c";
        label->setStyleSheet(QString("color: %1; font-size: 9px;").arg(i == 2 ? "#d7dce6" : "#6b6871"));
        value->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: 800;").arg(fg));
        delta->setStyleSheet(QString("color: %1; font-size: 8px; font-weight: 700;").arg(i == 2 ? "#58c786" : "#0873c8"));
        cardLayout->addWidget(label);
        cardLayout->addWidget(value);
        cardLayout->addStretch();
        cardLayout->addWidget(delta);
        statsLayout->addWidget(card, 1);
    }
    root->addWidget(statsRow);

    auto* body = new QWidget(page);
    auto* bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(10);

    auto* matchCard = new QFrame(body);
    matchCard->setFixedWidth(180);
    matchCard->setStyleSheet("QFrame { background: #f0eef0; border: none; border-radius: 4px; }");
    auto* matchLayout = new QVBoxLayout(matchCard);
    matchLayout->setContentsMargins(14, 14, 14, 12);
    auto* matchTitle = new QLabel("Tình trạng ghép nối");
    matchTitle->setStyleSheet("color: #17161c; font-size: 11px; font-weight: 700;");
    matchLayout->addWidget(matchTitle);
    auto* donut = new QLabel("◯\n86%\nĐÃ GHÉP NỐI", matchCard);
    donut->setAlignment(Qt::AlignCenter);
    donut->setMinimumHeight(145);
    donut->setStyleSheet("color: #111621; font-size: 15px; font-weight: 800;");
    matchLayout->addWidget(donut);
    matchLayout->addWidget(makeStatusDot("#111621", "Lớp đã có gia sư", "1,420"));
    matchLayout->addWidget(makeStatusDot("#b4b3b9", "Đang chờ ghép", "231"));
    bodyLayout->addWidget(matchCard);

    auto* charts = new QFrame(body);
    charts->setStyleSheet("QFrame { background: #f0eef0; border: none; border-radius: 4px; }");
    auto* chartsLayout = new QVBoxLayout(charts);
    chartsLayout->setContentsMargins(14, 14, 14, 12);
    chartsLayout->setSpacing(9);
    auto* classHeader = new QHBoxLayout();
    auto* classTitle = new QLabel("Trạng thái lớp học");
    classTitle->setStyleSheet("color: #17161c; font-size: 11px; font-weight: 700;");
    classHeader->addWidget(classTitle);
    classHeader->addStretch();
    classHeader->addWidget(makeBadge("Tuần này", "#ffffff", "#5d5b64"));
    chartsLayout->addLayout(classHeader);
    chartsLayout->addWidget(new QLabel("Phân bổ theo giai đoạn hoạt động"));
    const QStringList stages = {"CHỜ BẮT ĐẦU", "ĐANG HỌC", "HOÀN THÀNH", "HỦY"};
    const QStringList stageColors = {"#86bdf0", "#111621", "#697b8e", "#d7d5d8"};
    auto* stageRow = new QHBoxLayout();
    for (int i = 0; i < stages.size(); ++i) {
        auto* col = new QVBoxLayout();
        auto* bar = new QProgressBar();
        bar->setRange(0, 100); bar->setValue(i == 0 ? 46 : i == 1 ? 72 : i == 2 ? 64 : 18);
        bar->setTextVisible(false); bar->setFixedHeight(5);
        bar->setStyleSheet(QString("QProgressBar { background: #dedde1; border: none; } QProgressBar::chunk { background: %1; }").arg(stageColors[i]));
        auto* stageLabel = new QLabel(stages[i]);
        stageLabel->setAlignment(Qt::AlignCenter);
        stageLabel->setStyleSheet("color: #64616a; font-size: 7px; font-weight: 700;");
        col->addWidget(bar); col->addWidget(stageLabel); stageRow->addLayout(col, 1);
    }
    chartsLayout->addLayout(stageRow);
    chartsLayout->addSpacing(2);
    auto* subjectTitle = new QHBoxLayout();
    subjectTitle->addWidget(new QLabel("Phân bổ gia sư theo môn học"));
    subjectTitle->addStretch();
    subjectTitle->addWidget(makeLabel("Xem chi tiết", "color: #69727f; font-size: 8px;"));
    chartsLayout->addLayout(subjectTitle);
    const QStringList subjects = {"Toán Học", "Tiếng Anh", "Vật Lý", "Hóa Học", "Ngữ Văn"};
    const QStringList subjectValues = {"32% (269)", "28% (235)", "18% (151)", "14% (118)", "8% (69)"};
    const QStringList subjectColors = {"#111621", "#0873c8", "#27435f", "#4ea8f4", "#d8d7dc"};
    for (int i = 0; i < subjects.size(); ++i) {
        auto* row = new QHBoxLayout();
        row->addWidget(new QLabel(subjects[i])); row->addStretch(); row->addWidget(new QLabel(subjectValues[i]));
        chartsLayout->addLayout(row);
        auto* bar = new QProgressBar(); bar->setRange(0, 100); bar->setValue(32 - i * 6); bar->setTextVisible(false); bar->setFixedHeight(5);
        bar->setStyleSheet(QString("QProgressBar { background: #dedde1; border: none; } QProgressBar::chunk { background: %1; }").arg(subjectColors[i]));
        chartsLayout->addWidget(bar);
    }
    bodyLayout->addWidget(charts, 1);
    root->addWidget(body, 1);
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
