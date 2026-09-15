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
#include <QPainter>
#include <QPaintEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

using namespace std;

namespace {

// Palette matching the user's reference design
const QString SIDEBAR_BG = "#060D1E";
const QString SIDEBAR_ACTIVE = "#111E38";
const QString SIDEBAR_BORDER = "#1E293B";
const QString SIDEBAR_TEXT = "#94A3B8";

const QString BG = "#F8FAFC";
const QString CARD_BG = "#FFFFFF";
const QString BORDER = "#E2E8F0";

const QString TEXT_MAIN = "#0F172A";
const QString TEXT_MUTED = "#64748B";
const QString TEXT_SUBTLE = "#334155";

const QString BLUE = "#0284C7";
const QString BLUE_BG = "#E0F2FE";
const QString DARK_BTN = "#0F172A";
const QString DARK_HOVER = "#1E293B";

const QString RED = "#DC2626";
const QString RED_BG = "#FEE2E2";
const QString GREEN = "#16A34A";
const QString GREEN_BG = "#DCFCE7";
const QString GRAY_PILL_BG = "#F1F5F9";
const QString GRAY_PILL_TEXT = "#475569";

QLabel* label(const QString& text, int size = 13, bool bold = false,
              const QString& color = TEXT_MAIN) {
    auto* l = new QLabel(text);
    l->setStyleSheet(QString("color:%1;font-size:%2px;%3")
                     .arg(color).arg(size).arg(bold ? "font-weight:700;" : "font-weight:400;"));
    return l;
}

QFrame* card(QWidget* parent = nullptr) {
    auto* f = new QFrame(parent);
    f->setObjectName("appCard");
    f->setStyleSheet(
        "QFrame#appCard { background:#FFFFFF; border:1px solid #E2E8F0; border-radius:12px; }");
    return f;
}

QPushButton* button(const QString& text, bool primary = true, QWidget* parent = nullptr) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedHeight(38);
    b->setStyleSheet(primary
        ? "QPushButton { background:#0F172A; color:white; border:none; border-radius:6px; "
          "padding:0 16px; font-size:12px; font-weight:600; }"
          "QPushButton:hover { background:#1E293B; }"
        : "QPushButton { background:white; color:#0284C7; border:1px solid #0284C7; "
          "border-radius:6px; padding:0 16px; font-size:12px; font-weight:600; }"
          "QPushButton:hover { background:#F0F9FF; }");
    return b;
}

QPushButton* blueButton(const QString& text, QWidget* parent = nullptr) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedHeight(38);
    b->setStyleSheet(
        "QPushButton { background:#0284C7; color:white; border:none; border-radius:6px; "
        "padding:0 16px; font-size:12px; font-weight:600; }"
        "QPushButton:hover { background:#0369A1; }");
    return b;
}

QWidget* pillBadge(const QString& text, const QString& bg, const QString& fg) {
    auto* container = new QWidget;
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* b = new QLabel(text, container);
    b->setAlignment(Qt::AlignCenter);
    b->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:11px; padding:3px 12px; "
        "font-size:11px; font-weight:600;").arg(bg, fg));
    layout->addWidget(b);
    return container;
}

QProgressBar* progress(int value, const QString& color = BLUE, int height = 6) {
    auto* p = new QProgressBar;
    p->setRange(0, 100);
    p->setValue(value);
    p->setTextVisible(false);
    p->setFixedHeight(height);
    p->setStyleSheet(QString(
        "QProgressBar { background:#E2E8F0; border:none; border-radius:%2px; }"
        "QProgressBar::chunk { background:%1; border-radius:%2px; }")
        .arg(color).arg(height / 2));
    return p;
}

QWidget* userAvatarItem(const QString& initials, const QString& name,
                       const QString& bg = "#0F172A", const QString& fg = "#FFFFFF") {
    auto* w = new QWidget;
    auto* l = new QHBoxLayout(w);
    l->setContentsMargins(4, 2, 4, 2);
    l->setSpacing(10);

    auto* a = new QLabel(initials, w);
    a->setAlignment(Qt::AlignCenter);
    a->setFixedSize(30, 30);
    a->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:15px; font-size:11px; font-weight:700;")
        .arg(bg, fg));
    l->addWidget(a);

    auto* n = label(name, 13, true, TEXT_MAIN);
    l->addWidget(n);
    l->addStretch();
    return w;
}

void styleTable(QTableWidget* table) {
    table->setAlternatingRowColors(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setStyleSheet(
        "QTableWidget { background:#FFFFFF; border:none; color:#334155; font-size:12px; }"
        "QTableWidget::item { border-bottom:1px solid #F1F5F9; padding:10px 8px; }"
        "QTableWidget::item:selected { background:#F8FAFC; color:#0F172A; }"
        "QHeaderView::section { background:#FFFFFF; color:#64748B; border:none; "
        "border-bottom:1px solid #E2E8F0; padding:10px 8px; font-size:11px; "
        "font-weight:700; }");
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(60);
    table->verticalHeader()->setDefaultSectionSize(50);
}

// Header widget identical to the user reference screenshot
QWidget* header(QWidget* parent) {
    auto* h = new QWidget(parent);
    h->setFixedHeight(60);
    h->setStyleSheet("background:#FFFFFF; border-bottom:1px solid #E2E8F0;");
    auto* l = new QHBoxLayout(h);
    l->setContentsMargins(24, 11, 24, 11);
    l->setSpacing(12);

    auto* search = new QLineEdit(h);
    search->setPlaceholderText(QString::fromUtf8("🔍  Tìm kiếm hệ thống..."));
    search->setFixedSize(320, 36);
    search->setStyleSheet(
        "QLineEdit { background:#F1F5F9; border:1px solid #E2E8F0; "
        "border-radius:6px; padding:0 14px; color:#0F172A; font-size:12px; }"
        "QLineEdit:focus { border:1px solid #0284C7; background:#FFFFFF; }");
    l->addWidget(search);
    l->addStretch();

    // Notification bell icon with red badge
    auto* notifBtn = new QPushButton(h);
    notifBtn->setFixedSize(36, 36);
    notifBtn->setCursor(Qt::PointingHandCursor);
    notifBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; border-radius:6px; } "
        "QPushButton:hover { background:#F1F5F9; }");
    auto* notifLayout = new QHBoxLayout(notifBtn);
    notifLayout->setContentsMargins(0, 0, 0, 0);
    auto* bellIcon = new QLabel(QString::fromUtf8("🔔"), notifBtn);
    bellIcon->setAlignment(Qt::AlignCenter);
    bellIcon->setStyleSheet("font-size:16px; color:#475569; background:transparent;");
    notifLayout->addWidget(bellIcon);

    auto* redDot = new QLabel(notifBtn);
    redDot->setFixedSize(7, 7);
    redDot->setStyleSheet("background:#DC2626; border-radius:3px;");
    redDot->move(22, 6);
    l->addWidget(notifBtn);

    // Separator line
    auto* sep = new QFrame(h);
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Plain);
    sep->setFixedSize(1, 26);
    sep->setStyleSheet("background:#E2E8F0; border:none;");
    l->addWidget(sep);

    // Account block
    auto* account = new QWidget(h);
    auto* al = new QHBoxLayout(account);
    al->setContentsMargins(4, 0, 0, 0);
    al->setSpacing(10);

    auto* textBlock = new QVBoxLayout;
    textBlock->setContentsMargins(0, 0, 0, 0);
    textBlock->setSpacing(1);
    auto* nameLabel = label("Admin User", 12, true, TEXT_MAIN);
    nameLabel->setAlignment(Qt::AlignRight);
    auto* statusLabel = label(QString::fromUtf8("Sẵn sàng"), 11, false, TEXT_MUTED);
    statusLabel->setAlignment(Qt::AlignRight);
    textBlock->addWidget(nameLabel);
    textBlock->addWidget(statusLabel);
    al->addLayout(textBlock);

    auto* avatar = new QLabel(QString::fromUtf8("👤"), account);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(34, 34);
    avatar->setStyleSheet(
        "background:#0F172A; color:white; border-radius:17px; font-size:14px;");
    al->addWidget(avatar);

    l->addWidget(account);
    return h;
}

// 4 Metric cards in grid
QFrame* makeMetricCard(const QString& titleText, const QString& value,
                      const QString& trendVal, const QString& trendText,
                      const QString& iconEmoji, const QString& iconBg,
                      const QString& iconFg, bool isTrendPositive = true,
                      bool hasProgressBar = false, int progressVal = 0) {
    auto* f = card();
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(18, 16, 18, 16);
    l->setSpacing(6);

    auto* top = new QHBoxLayout;
    top->setContentsMargins(0, 0, 0, 0);
    top->addWidget(label(titleText, 13, false, TEXT_MUTED));
    top->addStretch();

    auto* icon = new QLabel(iconEmoji, f);
    icon->setAlignment(Qt::AlignCenter);
    icon->setFixedSize(36, 36);
    icon->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:8px; font-size:16px;")
        .arg(iconBg, iconFg));
    top->addWidget(icon);
    l->addLayout(top);

    auto* valLabel = label(value, 28, true, TEXT_MAIN);
    l->addWidget(valLabel);

    if (hasProgressBar) {
        l->addSpacing(4);
        l->addWidget(progress(progressVal, BLUE, 6));
    } else {
        auto* bottom = new QHBoxLayout;
        bottom->setContentsMargins(0, 0, 0, 0);
        bottom->setSpacing(4);

        QString trendColor = isTrendPositive ? BLUE : RED;
        auto* trend = label(trendVal, 12, true, trendColor);
        auto* note = label(trendText, 11, false, TEXT_MUTED);
        bottom->addWidget(trend);
        bottom->addWidget(note);
        bottom->addStretch();
        l->addLayout(bottom);
    }
    return f;
}

// Custom widget for training banner network illustration
class BannerIllustration : public QWidget {
public:
    explicit BannerIllustration(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedSize(140, 68);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Background card
        p.setPen(QPen(QColor("#E2E8F0"), 1));
        p.setBrush(QColor("#F8FAFC"));
        p.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);

        // Connected graph nodes
        struct Node { int x, y; QString symbol; QColor col; };
        Node nodes[] = {
            {30, 24, QString::fromUtf8("∑"), QColor("#0284C7")},
            {70, 18, QString::fromUtf8("🎓"), QColor("#0F172A")},
            {110, 26, QString::fromUtf8("∫"), QColor("#0284C7")},
            {45, 48, QString::fromUtf8("π"), QColor("#64748B")},
            {95, 48, QString::fromUtf8("√"), QColor("#0284C7")}
        };

        p.setPen(QPen(QColor("#BAE6FD"), 1, Qt::DashLine));
        p.drawLine(30, 24, 70, 18);
        p.drawLine(70, 18, 110, 26);
        p.drawLine(30, 24, 45, 48);
        p.drawLine(70, 18, 95, 48);
        p.drawLine(45, 48, 95, 48);

        for (const auto& n : nodes) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#FFFFFF"));
            p.drawEllipse(QPoint(n.x, n.y), 10, 10);
            p.setPen(QPen(QColor("#E2E8F0"), 1));
            p.drawEllipse(QPoint(n.x, n.y), 10, 10);

            p.setPen(n.col);
            QFont f = p.font();
            f.setPointSize(8);
            f.setBold(true);
            p.setFont(f);
            p.drawText(QRect(n.x - 10, n.y - 10, 20, 20), Qt::AlignCenter, n.symbol);
        }
    }
};

// Filter input box helper
void addFilter(QHBoxLayout* l, const QString& placeholder, int width = 200) {
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(38);
    edit->setMinimumWidth(width);
    edit->setStyleSheet(
        "QLineEdit { background:white; border:1px solid #DDE4EC; border-radius:6px; "
        "padding:0 12px; font-size:12px; color:#344054; }"
        "QLineEdit:focus { border:1px solid #0284C7; }");
    l->addWidget(edit, 1);
}

QComboBox* combo(const QStringList& items, int width = 150) {
    auto* c = new QComboBox;
    c->addItems(items);
    c->setFixedSize(width, 38);
    c->setStyleSheet(
        "QComboBox { background:white; border:1px solid #DDE4EC; border-radius:6px; "
        "padding:0 10px; font-size:12px; color:#344054; }"
        "QComboBox::drop-down { border:none; width:24px; }");
    return c;
}

QFrame* infoTile(const QString& titleText, const QString& value,
                 const QString& color = BLUE) {
    auto* f = card();
    f->setMinimumHeight(70);
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(14, 12, 14, 10);
    l->setSpacing(3);
    l->addWidget(label(titleText, 11, true, TEXT_MUTED));
    l->addWidget(label(value, 18, true, color));
    return f;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();

    setWindowTitle(QString::fromUtf8("QUẢN LÝ GIA SƯ - Hệ thống quản trị"));
    resize(1360, 840);
    setMinimumSize(1100, 720);

    auto* central = new QWidget(this);
    central->setStyleSheet(QString("background:%1;").arg(BG));
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createSidebar(), 0);

    stackedWidget = new QStackedWidget(central);
    stackedWidget->setObjectName("contentStack");
    stackedWidget->setStyleSheet(QString("QStackedWidget#contentStack { background:%1; border:none; }").arg(BG));

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
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
}

QString MainWindow::joinStrings(const vector<string>& items) {
    QStringList parts;
    for (const auto& item : items) {
        if (!item.empty()) parts << QString::fromStdString(item);
    }
    return parts.join(", ");
}

QWidget* MainWindow::createSidebar() {
    auto* panel = new QWidget;
    panel->setFixedWidth(230);
    panel->setObjectName("appSidebar");
    panel->setStyleSheet(QString(
        "QWidget#appSidebar { background:%1; color:white; border-right:1px solid #0F172A; }")
        .arg(SIDEBAR_BG));

    auto* l = new QVBoxLayout(panel);
    l->setContentsMargins(14, 20, 14, 16);
    l->setSpacing(6);

    // Brand logo + text
    auto* brandWidget = new QWidget(panel);
    auto* bl = new QHBoxLayout(brandWidget);
    bl->setContentsMargins(8, 6, 8, 16);
    bl->setSpacing(10);

    auto* logoIcon = new QLabel(QString::fromUtf8("🎓"), brandWidget);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setStyleSheet("font-size:22px; color:#0284C7; background:transparent;");
    bl->addWidget(logoIcon);

    auto* brandTitle = new QLabel(QString::fromUtf8("QUẢN LÝ GIA SƯ"), brandWidget);
    brandTitle->setStyleSheet(
        "color:#FFFFFF; font-size:14px; font-weight:800; letter-spacing:0.5px; background:transparent;");
    bl->addWidget(brandTitle);
    bl->addStretch();
    l->addWidget(brandWidget);

    // Navigation buttons
    auto* group = new QButtonGroup(panel);
    group->setExclusive(true);

    struct NavItem { QString icon; QString text; };
    const QVector<NavItem> items = {
        {QString::fromUtf8("⊞"), QString::fromUtf8("Tổng quan")},
        {QString::fromUtf8("👥"), QString::fromUtf8("Học sinh")},
        {QString::fromUtf8("👨‍🏫"), QString::fromUtf8("Gia sư")},
        {QString::fromUtf8("👤⁺"), QString::fromUtf8("Tìm gia sư")},
        {QString::fromUtf8("📖"), QString::fromUtf8("Lớp học")},
        {QString::fromUtf8("📄"), QString::fromUtf8("Hợp đồng")},
        {QString::fromUtf8("📊"), QString::fromUtf8("Thống kê")}
    };

    for (int i = 0; i < items.size(); ++i) {
        auto* b = new QPushButton(QString("  %1   %2").arg(items[i].icon, items[i].text), panel);
        b->setCheckable(true);
        b->setFixedHeight(44);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString(
            "QPushButton { text-align:left; padding-left:14px; background:transparent; "
            "color:%1; border:none; border-radius:8px; font-size:13px; font-weight:500; } "
            "QPushButton:hover { background:%2; color:#FFFFFF; } "
            "QPushButton:checked { background:%2; color:#FFFFFF; font-weight:700; }")
            .arg(SIDEBAR_TEXT, SIDEBAR_ACTIVE));

        if (i == 0) b->setChecked(true);
        group->addButton(b, i);
        connect(b, &QPushButton::clicked, this, [this, i]() { switchPage(i); });
        l->addWidget(b);
    }

    l->addStretch();

    // User Profile Card at bottom
    auto* profile = new QWidget(panel);
    profile->setStyleSheet("background:transparent;");
    auto* pl = new QHBoxLayout(profile);
    pl->setContentsMargins(6, 8, 6, 8);
    pl->setSpacing(10);

    auto* pa = new QLabel(QString::fromUtf8("👤"), profile);
    pa->setAlignment(Qt::AlignCenter);
    pa->setFixedSize(36, 36);
    pa->setStyleSheet("background:#0284C7; color:white; border-radius:18px; font-size:16px;");
    pl->addWidget(pa);

    auto* userTexts = new QVBoxLayout;
    userTexts->setContentsMargins(0, 0, 0, 0);
    userTexts->setSpacing(1);
    auto* userName = new QLabel("Admin", profile);
    userName->setStyleSheet("color:#FFFFFF; font-size:13px; font-weight:700; background:transparent;");
    auto* userRole = new QLabel(QString::fromUtf8("Quản trị viên"), profile);
    userRole->setStyleSheet("color:#64748B; font-size:11px; background:transparent;");
    userTexts->addWidget(userName);
    userTexts->addWidget(userRole);
    pl->addLayout(userTexts);
    pl->addStretch();
    l->addWidget(profile);

    // Logout button
    auto* logout = new QPushButton(QString::fromUtf8("  🚪  Đăng xuất"), panel);
    logout->setFixedHeight(38);
    logout->setCursor(Qt::PointingHandCursor);
    logout->setStyleSheet(
        "QPushButton { text-align:center; background:#111E38; color:#94A3B8; "
        "border:1px solid #1E293B; border-radius:6px; font-size:12px; font-weight:600; } "
        "QPushButton:hover { background:#1E293B; color:#FFFFFF; }");
    l->addWidget(logout);

    return panel;
}

QWidget* MainWindow::createDashboardPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    root->addWidget(header(page));

    // Scrollable content area
    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(18);

    // Top Section: Page Header & Action Buttons
    auto* headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(0, 0, 0, 0);

    auto* titleCol = new QVBoxLayout;
    titleCol->setContentsMargins(0, 0, 0, 0);
    titleCol->setSpacing(2);
    auto* eyebrow = label(QString::fromUtf8("BÁO CÁO TỔNG HỢP"), 11, true, TEXT_MUTED);
    auto* title = label(QString::fromUtf8("Tổng quan hệ thống"), 24, true, TEXT_MAIN);
    titleCol->addWidget(eyebrow);
    titleCol->addWidget(title);
    headerRow->addLayout(titleCol, 1);

    // 4 Action buttons
    auto* actions = new QHBoxLayout;
    actions->setSpacing(10);

    auto* btnAddStudent = button(QString::fromUtf8("👤⁺  Thêm học sinh"), true);
    auto* btnAddTutor = blueButton(QString::fromUtf8("👨‍🏫⁺  Thêm gia sư"));
    auto* btnFindTutor = button(QString::fromUtf8("🔍  Tìm gia sư"), false);
    auto* btnCreateClass = button(QString::fromUtf8("📖  Tạo lớp học"), false);

    actions->addWidget(btnAddStudent);
    actions->addWidget(btnAddTutor);
    actions->addWidget(btnFindTutor);
    actions->addWidget(btnCreateClass);
    headerRow->addLayout(actions);

    connect(btnAddStudent, &QPushButton::clicked, this, [this]() { switchPage(1); });
    connect(btnAddTutor, &QPushButton::clicked, this, [this]() { switchPage(2); });
    connect(btnFindTutor, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(btnCreateClass, &QPushButton::clicked, this, [this]() { switchPage(4); });

    l->addLayout(headerRow);

    // Grid of 4 Metric Cards
    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(14);

    metrics->addWidget(makeMetricCard(
        QString::fromUtf8("Tổng học sinh"), "125",
        "+12%", QString::fromUtf8(" so với tháng trước"),
        QString::fromUtf8("👥"), "#F1F5F9", "#475569", true), 1);

    metrics->addWidget(makeMetricCard(
        QString::fromUtf8("Tổng gia sư"), "68",
        "+5%", QString::fromUtf8(" so với tháng trước"),
        QString::fromUtf8("🎓"), "#E0F2FE", "#0284C7", true), 1);

    metrics->addWidget(makeMetricCard(
        QString::fromUtf8("Lớp đang hoạt động"), "42",
        "-2%", QString::fromUtf8(" so với tháng trước"),
        QString::fromUtf8("📥"), "#F1F5F9", "#475569", false), 1);

    metrics->addWidget(makeMetricCard(
        QString::fromUtf8("Tỷ lệ ghép thành công"), "78%",
        "", "",
        QString::fromUtf8("🤝"), "#E0F2FE", "#0284C7", true,
        true, 78), 1);

    l->addLayout(metrics);

    // Lower Section: Two Columns (Left: Status Cards; Right: Recent Classes & Banner)
    auto* body = new QHBoxLayout;
    body->setSpacing(16);

    // --- LEFT COLUMN ---
    auto* leftCol = new QVBoxLayout;
    leftCol->setSpacing(14);

    // Card 1: Trạng thái học sinh
    auto* studentStatusCard = card();
    studentStatusCard->setFixedWidth(330);
    auto* scLayout = new QVBoxLayout(studentStatusCard);
    scLayout->setContentsMargins(18, 18, 18, 18);
    scLayout->setSpacing(14);

    scLayout->addWidget(label(QString::fromUtf8("Trạng thái học sinh"), 15, true, TEXT_MAIN));

    // Row: Đã ghép 98
    auto* row1 = new QHBoxLayout;
    auto* dot1 = new QLabel(QString::fromUtf8("●  Đã ghép"), studentStatusCard);
    dot1->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(BLUE));
    auto* val1 = label("98", 15, true, TEXT_MAIN);
    row1->addWidget(dot1);
    row1->addStretch();
    row1->addWidget(val1);
    scLayout->addLayout(row1);
    scLayout->addWidget(progress(78, BLUE, 6));

    scLayout->addSpacing(4);

    // Row: Chưa ghép 27
    auto* row2 = new QHBoxLayout;
    auto* dot2 = new QLabel(QString::fromUtf8("●  Chưa ghép"), studentStatusCard);
    dot2->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(TEXT_MUTED));
    auto* val2 = label("27", 15, true, TEXT_MAIN);
    row2->addWidget(dot2);
    row2->addStretch();
    row2->addWidget(val2);
    scLayout->addLayout(row2);
    scLayout->addWidget(progress(22, "#94A3B8", 6));

    scLayout->addSpacing(6);
    leftCol->addWidget(studentStatusCard);

    // Card 2: Trạng thái gia sư
    auto* tutorStatusCard = card();
    tutorStatusCard->setFixedWidth(330);
    auto* tcLayout = new QVBoxLayout(tutorStatusCard);
    tcLayout->setContentsMargins(18, 18, 18, 18);
    tcLayout->setSpacing(12);

    tcLayout->addWidget(label(QString::fromUtf8("Trạng thái gia sư"), 15, true, TEXT_MAIN));

    auto* tRow1 = new QHBoxLayout;
    auto* tDot1 = new QLabel(QString::fromUtf8("●  Sẵn sàng"), tutorStatusCard);
    tDot1->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(BLUE));
    auto* tVal1 = label("45", 14, true, TEXT_MAIN);
    tRow1->addWidget(tDot1);
    tRow1->addStretch();
    tRow1->addWidget(tVal1);
    tcLayout->addLayout(tRow1);

    auto* tRow2 = new QHBoxLayout;
    auto* tDot2 = new QLabel(QString::fromUtf8("●  Bận"), tutorStatusCard);
    tDot2->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(TEXT_MAIN));
    auto* tVal2 = label("18", 14, true, TEXT_MAIN);
    tRow2->addWidget(tDot2);
    tRow2->addStretch();
    tRow2->addWidget(tVal2);
    tcLayout->addLayout(tRow2);

    auto* tRow3 = new QHBoxLayout;
    auto* tDot3 = new QLabel(QString::fromUtf8("●  Không hoạt động"), tutorStatusCard);
    tDot3->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(TEXT_MUTED));
    auto* tVal3 = label("5", 14, true, TEXT_MAIN);
    tRow3->addWidget(tDot3);
    tRow3->addStretch();
    tRow3->addWidget(tVal3);
    tcLayout->addLayout(tRow3);

    // Mini bar chart columns (blue, black, gray)
    tcLayout->addSpacing(8);
    auto* barChart = new QHBoxLayout;
    barChart->setContentsMargins(0, 0, 0, 0);
    barChart->setSpacing(10);
    barChart->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

    auto* col1 = new QFrame(tutorStatusCard);
    col1->setFixedSize(65, 52);
    col1->setStyleSheet(QString("background:%1; border-radius:4px;").arg(BLUE));

    auto* col2 = new QFrame(tutorStatusCard);
    col2->setFixedSize(65, 24);
    col2->setStyleSheet(QString("background:%1; border-radius:4px;").arg(DARK_BTN));

    auto* col3 = new QFrame(tutorStatusCard);
    col3->setFixedSize(65, 10);
    col3->setStyleSheet("background:#94A3B8; border-radius:4px;");

    barChart->addWidget(col1);
    barChart->addWidget(col2);
    barChart->addWidget(col3);
    barChart->addStretch();
    tcLayout->addLayout(barChart);

    leftCol->addWidget(tutorStatusCard);
    leftCol->addStretch();
    body->addLayout(leftCol, 0);

    // --- RIGHT COLUMN ---
    auto* rightCol = new QVBoxLayout;
    rightCol->setSpacing(14);

    // Card: Lớp học gần đây
    auto* recentCard = card();
    auto* rcLayout = new QVBoxLayout(recentCard);
    rcLayout->setContentsMargins(20, 18, 20, 18);
    rcLayout->setSpacing(12);

    auto* rHead = new QHBoxLayout;
    rHead->addWidget(label(QString::fromUtf8("Lớp học gần đây"), 15, true, TEXT_MAIN));
    rHead->addStretch();
    auto* seeAll = new QPushButton(QString::fromUtf8("XEM TẤT CẢ"), recentCard);
    seeAll->setCursor(Qt::PointingHandCursor);
    seeAll->setStyleSheet(
        "QPushButton { background:transparent; color:#0284C7; border:none; "
        "font-size:11px; font-weight:700; } QPushButton:hover { color:#0369A1; }");
    connect(seeAll, &QPushButton::clicked, this, [this]() { switchPage(4); });
    rHead->addWidget(seeAll);
    rcLayout->addLayout(rHead);

    // Table
    auto* table = new QTableWidget(4, 6, recentCard);
    table->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ LỚP"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("MÔN HỌC"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(table);
    table->setFixedHeight(225);

    // Fill table rows matching user reference screenshot
    struct RowData {
        QString code;
        QString initials;
        QString studentName;
        QString avatarBg;
        QString avatarFg;
        QString tutorName;
        QString subject;
        QString statusText;
        QString statusBg;
        QString statusFg;
    };

    const QVector<RowData> recentData = {
        {"LH-1042", "NA", QString::fromUtf8("Nguyễn Văn A"), "#0F172A", "#FFFFFF",
         QString::fromUtf8("Trần Thị B"), QString::fromUtf8("Toán Lớp 10"),
         QString::fromUtf8("Đang học"), BLUE_BG, BLUE},

        {"LH-1043", "LM", QString::fromUtf8("Lê Hoàng M"), "#451A03", "#FFFFFF",
         QString::fromUtf8("Phạm Văn C"), QString::fromUtf8("Tiếng Anh IELTS"),
         QString::fromUtf8("Chờ xếp lịch"), GRAY_PILL_BG, GRAY_PILL_TEXT},

        {"LH-1044", "VT", QString::fromUtf8("Vũ Minh T"), "#FFEDD5", "#C2410C",
         QString::fromUtf8("Chưa có"), QString::fromUtf8("Vật Lý 12"),
         QString::fromUtf8("Cần gia sư"), RED_BG, RED},

        {"LH-1045", "HK", QString::fromUtf8("Hoàng Tú K"), "#0284C7", "#FFFFFF",
         QString::fromUtf8("Ngô Phương T"), QString::fromUtf8("Hóa Lớp 9"),
         QString::fromUtf8("Đang học"), BLUE_BG, BLUE}
    };

    table->setColumnWidth(0, 85);
    table->setColumnWidth(1, 190);
    table->setColumnWidth(2, 130);
    table->setColumnWidth(3, 140);
    table->setColumnWidth(4, 110);
    table->setColumnWidth(5, 70);

    for (int i = 0; i < recentData.size(); ++i) {
        const auto& d = recentData[i];

        // Mã lớp (blue link style)
        auto* codeLabel = label(d.code, 12, true, BLUE);
        codeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setCellWidget(i, 0, codeLabel);

        // Học sinh (avatar + tên)
        table->setCellWidget(i, 1, userAvatarItem(d.initials, d.studentName, d.avatarBg, d.avatarFg));

        // Gia sư
        auto* tutorLabel = label(d.tutorName, 12, false, TEXT_SUBTLE);
        tutorLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setCellWidget(i, 2, tutorLabel);

        // Môn học
        auto* subjCol = new QWidget;
        auto* subjLayout = new QVBoxLayout(subjCol);
        subjLayout->setContentsMargins(4, 4, 4, 4);
        subjLayout->setSpacing(0);
        auto* subjMain = label(d.subject, 12, false, TEXT_MAIN);
        subjLayout->addWidget(subjMain);
        table->setCellWidget(i, 3, subjCol);

        // Trạng thái (pill badge)
        table->setCellWidget(i, 4, pillBadge(d.statusText, d.statusBg, d.statusFg));

        // Thao tác
        auto* actionBtn = new QPushButton(QString::fromUtf8("•••"), recentCard);
        actionBtn->setFixedSize(30, 26);
        actionBtn->setCursor(Qt::PointingHandCursor);
        actionBtn->setStyleSheet(
            "QPushButton { background:transparent; color:#64748B; border:none; font-size:14px; font-weight:bold; } "
            "QPushButton:hover { color:#0F172A; background:#F1F5F9; border-radius:4px; }");
        table->setCellWidget(i, 5, actionBtn);
    }

    rcLayout->addWidget(table);
    rightCol->addWidget(recentCard);

    // Bottom Banner: "Chương trình đào tạo mới"
    auto* bannerCard = card();
    auto* bcLayout = new QHBoxLayout(bannerCard);
    bcLayout->setContentsMargins(20, 16, 20, 16);
    bcLayout->setSpacing(16);

    auto* bannerTextCol = new QVBoxLayout;
    bannerTextCol->setContentsMargins(0, 0, 0, 0);
    bannerTextCol->setSpacing(4);

    auto* bannerTitle = label(QString::fromUtf8("Chương trình đào tạo mới"), 14, true, TEXT_MAIN);
    auto* bannerDesc = label(
        QString::fromUtf8("Cập nhật tài liệu giảng dạy môn Toán cấp 3 theo chương trình GDPT 2018. "
                          "Vui lòng thông báo cho các gia sư liên quan."),
        12, false, TEXT_MUTED);
    bannerDesc->setWordWrap(true);

    bannerTextCol->addWidget(bannerTitle);
    bannerTextCol->addWidget(bannerDesc);
    bcLayout->addLayout(bannerTextCol, 1);

    auto* illustration = new BannerIllustration(bannerCard);
    bcLayout->addWidget(illustration, 0, Qt::AlignVCenter);

    rightCol->addWidget(bannerCard);
    rightCol->addStretch();
    body->addLayout(rightCol, 1);

    l->addLayout(body);
    l->addStretch();

    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

QWidget* MainWindow::createStudentPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    // Header
    auto* head = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("QUẢN LÝ"), 11, true, TEXT_MUTED));
    tCol->addWidget(label(QString::fromUtf8("Quản lý học sinh"), 24, true, TEXT_MAIN));
    head->addLayout(tCol, 1);
    head->addWidget(button(QString::fromUtf8("👤⁺  Thêm học sinh"), true));
    l->addLayout(head);

    // Summary tiles
    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTile(QString::fromUtf8("Tổng học sinh"),
        QString::number(students.size() > 0 ? students.size() : 125)), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Đã ghép"), "98", GREEN), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Chưa ghép"), "27", RED), 1);
    l->addLayout(summary);

    // Filter card
    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 12, 14, 12);
    fl->setSpacing(10);
    addFilter(fl, QString::fromUtf8("⌕  Tìm kiếm theo mã, tên học sinh..."), 280);
    fl->addWidget(combo({QString::fromUtf8("Tất cả khối lớp"), "Lớp 10", "Lớp 11", "Lớp 12"}, 150));
    fl->addWidget(combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")}, 150));
    fl->addStretch();
    auto* reset = button(QString::fromUtf8("Đặt lại"), false);
    reset->setFixedWidth(90);
    fl->addWidget(reset);
    l->addWidget(filters);

    // Table
    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    auto* table = new QTableWidget(0, 7, tableCard);
    table->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ HS"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("LỚP & MÔN"),
        QString::fromUtf8("LỊCH RẢNH"),
        QString::fromUtf8("NGÂN SÁCH"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(table);
    populateStudentTable(table);
    tl->addWidget(table);
    l->addWidget(tableCard, 1);

    return page;
}

QWidget* MainWindow::createTutorPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* head = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("QUẢN LÝ"), 11, true, TEXT_MUTED));
    tCol->addWidget(label(QString::fromUtf8("Quản lý gia sư"), 24, true, TEXT_MAIN));
    head->addLayout(tCol, 1);
    head->addWidget(blueButton(QString::fromUtf8("👨‍🏫⁺  Thêm gia sư")));
    l->addLayout(head);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTile(QString::fromUtf8("Tổng gia sư"),
        QString::number(tutors.size() > 0 ? tutors.size() : 68)), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Sẵn sàng"), "45", GREEN), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Đang bận"), "18", RED), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Mức phí TB"), "180k/buổi", BLUE), 1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 12, 14, 12);
    fl->setSpacing(10);
    addFilter(fl, QString::fromUtf8("⌕  Tìm kiếm gia sư theo tên, chuyên môn..."), 280);
    fl->addWidget(combo({QString::fromUtf8("Tất cả môn"), "Toán", "Lý", "Hóa", "Tiếng Anh"}, 140));
    fl->addWidget(combo({QString::fromUtf8("Tất cả khu vực"), "Hải Châu", "Thanh Khê", "Liên Chiểu"}, 150));
    fl->addWidget(combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Sẵn sàng"), QString::fromUtf8("Bận")}, 140));
    l->addWidget(filters);

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    auto* table = new QTableWidget(0, 9, tableCard);
    table->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ GS"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("MÔN DẠY"),
        QString::fromUtf8("KINH NGHIỆM"),
        QString::fromUtf8("HỌC PHÍ"),
        QString::fromUtf8("LỊCH RẢNH"),
        QString::fromUtf8("KHU VỰC"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(table);
    populateTutorTable(table);
    tl->addWidget(table);
    l->addWidget(tableCard, 1);

    return page;
}

QWidget* MainWindow::createMatchingPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("SMART MATCHING"), 11, true, TEXT_MUTED));
    tCol->addWidget(label(QString::fromUtf8("Tìm Gia Sư Thông Minh"), 24, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Hệ thống tự động phân tích yêu cầu học sinh và đề xuất gia sư tối ưu nhất."), 12, false, TEXT_MUTED));
    l->addLayout(tCol);

    auto* chooseRow = new QHBoxLayout;
    chooseRow->setSpacing(14);

    auto* studentCard = card();
    auto* sc = new QVBoxLayout(studentCard);
    sc->setContentsMargins(18, 16, 18, 16);
    sc->addWidget(label(QString::fromUtf8("BƯỚC 1: Chọn học sinh"), 12, true, BLUE));
    auto* studentCombo = combo({"SV001 - Nguyễn Minh Anh", "SV002 - Trần Văn Bảo", "SV003 - Lê Thị Lan"}, 280);
    sc->addWidget(studentCombo);
    chooseRow->addWidget(studentCard, 1);

    auto* request = card();
    auto* rc = new QVBoxLayout(request);
    rc->setContentsMargins(18, 16, 18, 16);
    auto* rh = new QHBoxLayout;
    rh->addWidget(label(QString::fromUtf8("YÊU CẦU HỌC SINH"), 12, true, BLUE));
    rh->addStretch();
    rh->addWidget(pillBadge(QString::fromUtf8("Đã tải dữ liệu"), GREEN_BG, GREEN));
    rc->addLayout(rh);

    auto* reqGrid = new QGridLayout;
    reqGrid->setHorizontalSpacing(24);
    reqGrid->setVerticalSpacing(4);
    reqGrid->addWidget(label(QString::fromUtf8("Môn học"), 11, false, TEXT_MUTED), 0, 0);
    reqGrid->addWidget(label(QString::fromUtf8("Toán"), 13, true, TEXT_MAIN), 1, 0);
    reqGrid->addWidget(label(QString::fromUtf8("Khu vực"), 11, false, TEXT_MUTED), 0, 1);
    reqGrid->addWidget(label(QString::fromUtf8("Hải Châu"), 13, true, TEXT_MAIN), 1, 1);
    reqGrid->addWidget(label(QString::fromUtf8("Ngân sách"), 11, false, TEXT_MUTED), 0, 2);
    reqGrid->addWidget(label(QString::fromUtf8("200.000đ/buổi"), 13, true, BLUE), 1, 2);
    reqGrid->addWidget(label(QString::fromUtf8("Lịch học"), 11, false, TEXT_MUTED), 0, 3);
    reqGrid->addWidget(label(QString::fromUtf8("T2, T4, T6"), 13, true, TEXT_MAIN), 1, 3);
    rc->addLayout(reqGrid);
    chooseRow->addWidget(request, 2);
    l->addLayout(chooseRow);

    auto* find = blueButton(QString::fromUtf8("✦  TÌM GIA SƯ PHÙ HỢP NHẤT"));
    find->setFixedSize(240, 42);
    l->addWidget(find, 0, Qt::AlignCenter);

    auto* result = new QHBoxLayout;
    result->setSpacing(14);

    auto* candidate = new QFrame;
    candidate->setStyleSheet(QString("QFrame { background:%1; border-radius:12px; }").arg(SIDEBAR_BG));
    candidate->setMinimumWidth(280);
    auto* cl = new QVBoxLayout(candidate);
    cl->setContentsMargins(20, 18, 20, 18);
    cl->setSpacing(8);

    auto* cTop = new QHBoxLayout;
    cTop->addWidget(label("TOP 1", 11, true, "#F59E0B"));
    cTop->addStretch();
    cTop->addWidget(label("95%", 20, true, BLUE));
    cl->addLayout(cTop);

    auto* candAvatar = new QLabel(QString::fromUtf8("👨‍🏫"), candidate);
    candAvatar->setAlignment(Qt::AlignCenter);
    candAvatar->setFixedSize(48, 48);
    candAvatar->setStyleSheet("background:#111E38; border-radius:24px; font-size:24px;");
    cl->addWidget(candAvatar, 0, Qt::AlignCenter);

    auto* cn = label(QString::fromUtf8("Trần Quốc Huy"), 18, true, "#FFFFFF");
    cn->setAlignment(Qt::AlignCenter);
    cl->addWidget(cn);

    auto* cc = label(QString::fromUtf8("GS003 • Toán, Lý • 8 năm kinh nghiệm"), 11, false, SIDEBAR_TEXT);
    cc->setAlignment(Qt::AlignCenter);
    cl->addWidget(cc);

    cl->addSpacing(6);
    cl->addWidget(label(QString::fromUtf8("ĐỘ PHÙ HỢP"), 11, true, SIDEBAR_TEXT));
    cl->addWidget(progress(95, BLUE, 8));

    result->addWidget(candidate, 1);

    auto* detail = card();
    auto* dl = new QVBoxLayout(detail);
    dl->setContentsMargins(20, 18, 20, 18);
    dl->setSpacing(10);
    dl->addWidget(label(QString::fromUtf8("Chi tiết tiêu chí matching"), 15, true, TEXT_MAIN));

    auto* chips = new QHBoxLayout;
    chips->addWidget(infoTile(QString::fromUtf8("Kinh nghiệm"), "25/25", BLUE));
    chips->addWidget(infoTile(QString::fromUtf8("Khu vực"), "10/10", GREEN));
    chips->addWidget(infoTile(QString::fromUtf8("Học phí"), "40/45", "#F59E0B"));
    chips->addWidget(infoTile(QString::fromUtf8("Lịch học"), "20/20", BLUE));
    dl->addLayout(chips);

    dl->addWidget(label(QString::fromUtf8("LÝ DO ĐỀ XUẤT:"), 12, true, BLUE));
    const QStringList reasons = {
        QString::fromUtf8("✓ Kinh nghiệm chuyên môn đáp ứng xuất sắc yêu cầu môn Toán."),
        QString::fromUtf8("✓ Khu vực Hải Châu phù hợp thuận tiện với địa điểm học sinh."),
        QString::fromUtf8("✓ Học phí nằm hoàn toàn trong ngân sách đề xuất."),
        QString::fromUtf8("✓ Lịch rảnh T2, T4, T6 trùng khớp hoàn toàn 100%.")
    };
    for (const auto& r : reasons) dl->addWidget(label(r, 12, false, TEXT_MUTED));
    dl->addStretch();

    auto* actionsRow = new QHBoxLayout;
    actionsRow->addStretch();
    actionsRow->addWidget(button(QString::fromUtf8("Xem hồ sơ đầy đủ"), false));
    actionsRow->addWidget(blueButton(QString::fromUtf8("✔  Chọn gia sư này & Tạo lớp")));
    dl->addLayout(actionsRow);

    result->addWidget(detail, 2);
    l->addLayout(result, 1);

    return page;
}

QWidget* MainWindow::createClassPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* head = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("QUẢN LÝ"), 11, true, TEXT_MUTED));
    tCol->addWidget(label(QString::fromUtf8("Quản lý lớp học"), 24, true, TEXT_MAIN));
    head->addLayout(tCol, 1);
    head->addWidget(button(QString::fromUtf8("📖⁺  Tạo lớp học"), false));
    l->addLayout(head);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTile(QString::fromUtf8("Tổng lớp"), "42", BLUE), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Đang học"), "31", GREEN), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Chờ xếp lịch"), "7", "#F59E0B"), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Cần gia sư"), "4", RED), 1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 12, 14, 12);
    fl->setSpacing(10);
    addFilter(fl, QString::fromUtf8("⌕  Tìm mã lớp, học sinh, gia sư..."), 280);
    fl->addWidget(combo({QString::fromUtf8("Tất cả môn"), "Toán", "Lý", "Hóa", "Văn"}, 140));
    fl->addWidget(combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư")}, 160));
    l->addWidget(filters);

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    auto* table = new QTableWidget(4, 8, tableCard);
    table->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ LỚP"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("MÔN"),
        QString::fromUtf8("LỊCH HỌC"),
        QString::fromUtf8("HỌC PHÍ"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(table);

    const QString rows[4][8] = {
        {"LH-1042", "Nguyễn Văn A", "Trần Thị B", "Toán Lớp 10", "T2, T4, T6", "200k/buổi", "Đang học", "Xem"},
        {"LH-1043", "Lê Hoàng M", "Phạm Văn C", "Tiếng Anh IELTS", "T3, T5", "250k/buổi", "Chờ xếp lịch", "Xem"},
        {"LH-1044", "Vũ Minh T", "Chưa có", "Vật Lý 12", "T2, T5, T7", "220k/buổi", "Cần gia sư", "Xem"},
        {"LH-1045", "Hoàng Tú K", "Ngô Phương T", "Hóa Lớp 9", "T4, T6", "180k/buổi", "Đang học", "Xem"}
    };
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            table->setItem(i, j, new QTableWidgetItem(rows[i][j]));
        }
    }
    tl->addWidget(table);
    l->addWidget(tableCard, 1);

    return page;
}

QWidget* MainWindow::createContractPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* head = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("QUẢN LÝ"), 11, true, TEXT_MUTED));
    tCol->addWidget(label(QString::fromUtf8("Quản lý hợp đồng"), 24, true, TEXT_MAIN));
    head->addLayout(tCol, 1);
    head->addWidget(blueButton(QString::fromUtf8("📄⁺  Tạo hợp đồng")));
    l->addLayout(head);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTile(QString::fromUtf8("Tổng hợp đồng"), "18", BLUE), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Đang hiệu lực"), "12", GREEN), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Chờ xử lý"), "4", "#F59E0B"), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Hoàn thành"), "2", "#8B5CF6"), 1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 12, 14, 12);
    fl->setSpacing(10);
    addFilter(fl, QString::fromUtf8("⌕  Tìm mã hợp đồng, lớp, học sinh..."), 280);
    fl->addWidget(combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đang hiệu lực"), QString::fromUtf8("Chờ xử lý"), QString::fromUtf8("Hoàn thành")}, 170));
    l->addWidget(filters);

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    auto* table = new QTableWidget(4, 8, tableCard);
    table->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ HĐ"),
        QString::fromUtf8("MÃ LỚP"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("BẮT ĐẦU"),
        QString::fromUtf8("TỔNG PHÍ"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(table);

    const QString rows[4][8] = {
        {"HD001", "LH-1042", "Nguyễn Văn A", "Trần Thị B", "01/09/2026", "2.400.000đ", "Đang hiệu lực", "Xem"},
        {"HD002", "LH-1043", "Lê Hoàng M", "Phạm Văn C", "10/09/2026", "3.000.000đ", "Chờ xử lý", "Xem"},
        {"HD003", "LH-1044", "Vũ Minh T", "Chưa có", "01/10/2026", "2.640.000đ", "Chờ xử lý", "Xem"},
        {"HD004", "LH-1045", "Hoàng Tú K", "Ngô Phương T", "15/09/2026", "2.160.000đ", "Đang hiệu lực", "Xem"}
    };
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            table->setItem(i, j, new QTableWidgetItem(rows[i][j]));
        }
    }
    tl->addWidget(table);
    l->addWidget(tableCard, 1);

    return page;
}

QWidget* MainWindow::createStatisticsPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* titleRow = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("BÁO CÁO VẬN HÀNH"), 11, true, TEXT_MUTED));
    tCol->addWidget(label(QString::fromUtf8("Thống kê hệ thống"), 24, true, TEXT_MAIN));
    titleRow->addLayout(tCol, 1);
    titleRow->addWidget(combo({QString::fromUtf8("30 ngày qua"), QString::fromUtf8("7 ngày qua"), QString::fromUtf8("Quý này")}, 130));
    titleRow->addWidget(button(QString::fromUtf8("⇩  Xuất báo cáo"), true));
    l->addLayout(titleRow);

    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Tổng học sinh"), "2,543", "+12%", QString::fromUtf8(" tăng trưởng"), QString::fromUtf8("👥"), "#F1F5F9", "#475569", true), 1);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Tổng gia sư"), "842", "+5%", QString::fromUtf8(" tăng trưởng"), QString::fromUtf8("🎓"), "#E0F2FE", BLUE, true), 1);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Lớp hoạt động"), "1,120", "+18%", QString::fromUtf8(" nhận lớp"), QString::fromUtf8("📥"), "#F1F5F9", "#475569", true), 1);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Tỷ lệ ghép"), "86%", "", "", QString::fromUtf8("🤝"), "#E0F2FE", BLUE, true, true, 86), 1);
    l->addLayout(metrics);

    auto* chartCard = card();
    auto* cl = new QVBoxLayout(chartCard);
    cl->setContentsMargins(20, 18, 20, 18);
    cl->setSpacing(12);
    cl->addWidget(label(QString::fromUtf8("Phân bổ gia sư theo môn học"), 15, true, TEXT_MAIN));

    const QString subjects[5] = {"Toán Học", "Tiếng Anh", "Vật Lý", "Hóa Học", "Ngữ Văn"};
    const int vals[5] = {32, 28, 18, 14, 8};
    for (int i = 0; i < 5; ++i) {
        auto* row = new QHBoxLayout;
        row->addWidget(label(subjects[i], 12, false, TEXT_MAIN));
        row->addStretch();
        row->addWidget(label(QString("%1%").arg(vals[i]), 12, true, BLUE));
        cl->addLayout(row);
        cl->addWidget(progress(vals[i], BLUE, 6));
    }
    l->addWidget(chartCard, 1);

    return page;
}

void MainWindow::populateStudentTable(QTableWidget* table) {
    table->setRowCount(static_cast<int>(students.size()));
    for (size_t i = 0; i < students.size(); ++i) {
        Student* student = students[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(student->getPersonID())));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(student->getFullName())));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(student->getGrade() + " - " + joinStrings(student->getNeedSubjects()).toStdString())));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::fromStdString(student->getAvailableSchedule())));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::fromStdString(student->getBudgetPerHour())));
        table->setItem(static_cast<int>(i), 5, new QTableWidgetItem(student->getIsMatched() ? QString::fromUtf8("Đã ghép") : QString::fromUtf8("Chưa ghép")));
        table->setItem(static_cast<int>(i), 6, new QTableWidgetItem(QString::fromUtf8("Xem")));
    }
}

void MainWindow::populateTutorTable(QTableWidget* table) {
    table->setRowCount(static_cast<int>(tutors.size()));
    for (size_t i = 0; i < tutors.size(); ++i) {
        Tutor* tutor = tutors[i];
        table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(tutor->getPersonID())));
        table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(tutor->getFullName())));
        table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(joinStrings(tutor->getSubjects())));
        table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(QString::number(tutor->getYearsOfExperience()) + " năm"));
        table->setItem(static_cast<int>(i), 4, new QTableWidgetItem(QString::fromStdString(tutor->getRatePerHour())));
        table->setItem(static_cast<int>(i), 5, new QTableWidgetItem(QString::fromStdString(tutor->getAvailability())));
        table->setItem(static_cast<int>(i), 6, new QTableWidgetItem(joinStrings(tutor->getTeachingAreas())));
        table->setItem(static_cast<int>(i), 7, new QTableWidgetItem(tutor->getIsAvailable() ? QString::fromUtf8("Sẵn sàng") : QString::fromUtf8("Bận")));
        table->setItem(static_cast<int>(i), 8, new QTableWidgetItem(QString::fromUtf8("Xem")));
    }
}

void MainWindow::switchPage(int index) {
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(index);
    }
}
