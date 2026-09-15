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

// Color palette matching the user's reference designs
const QString SIDEBAR_BG = "#060D1E";
const QString SIDEBAR_ACTIVE = "#0284C7";
const QString SIDEBAR_HOVER = "#111E38";
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
const QString PEACH_BG = "#FFEDD5";
const QString PEACH_TEXT = "#C2410C";
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

QWidget* pillBadge(const QString& text, const QString& bg, const QString& fg, bool withDot = false) {
    auto* container = new QWidget;
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QString content = withDot ? QString("●  %1").arg(text) : text;
    auto* b = new QLabel(content, container);
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
    table->verticalHeader()->setDefaultSectionSize(52);
}

// Custom reusable Donut Chart Widget
class DonutChartWidget : public QWidget {
    int percent;
    QColor arcColor;
    QColor bgColor;
    QString centerMain;
    QString centerSub;
    bool isDark;
public:
    DonutChartWidget(int percent, const QColor& arcColor, const QColor& bgColor,
                     const QString& centerMain, const QString& centerSub,
                     bool isDark = false, QWidget* parent = nullptr)
        : QWidget(parent), percent(percent), arcColor(arcColor), bgColor(bgColor),
          centerMain(centerMain), centerSub(centerSub), isDark(isDark) {
        setFixedSize(140, 140);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QRectF r(12, 12, width() - 24, height() - 24);
        int penWidth = 14;

        // Background circle
        QPen bgPen(bgColor, penWidth, Qt::SolidLine, Qt::RoundCap);
        p.setPen(bgPen);
        p.drawArc(r, 0, 360 * 16);

        // Progress arc
        QPen arcPen(arcColor, penWidth, Qt::SolidLine, Qt::RoundCap);
        p.setPen(arcPen);
        p.drawArc(r, 90 * 16, -int((percent / 100.0) * 360 * 16));

        // Center text main
        p.setPen(isDark ? QColor("#FFFFFF") : QColor("#0F172A"));
        QFont fMain = p.font();
        fMain.setPointSize(16);
        fMain.setBold(true);
        p.setFont(fMain);
        p.drawText(QRectF(0, height() / 2 - 20, width(), 24), Qt::AlignCenter, centerMain);

        // Center text sub
        p.setPen(isDark ? QColor("#38BDF8") : QColor("#64748B"));
        QFont fSub = p.font();
        fSub.setPointSize(8);
        fSub.setBold(true);
        p.setFont(fSub);
        p.drawText(QRectF(0, height() / 2 + 5, width(), 16), Qt::AlignCenter, centerSub);
    }
};

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

        p.setPen(QPen(QColor("#E2E8F0"), 1));
        p.setBrush(QColor("#F8FAFC"));
        p.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);

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
    auto* statusLabel = label(QString::fromUtf8("● Sẵn sàng"), 11, false, GREEN);
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

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();

    setWindowTitle(QString::fromUtf8("QUẢN LÝ GIA SƯ - Hệ thống quản trị"));
    resize(1360, 860);
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

    // Brand logo + text + version subtitle
    auto* brandWidget = new QWidget(panel);
    auto* bl = new QHBoxLayout(brandWidget);
    bl->setContentsMargins(8, 6, 8, 16);
    bl->setSpacing(10);

    auto* logoIcon = new QLabel(QString::fromUtf8("🎓"), brandWidget);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setFixedSize(36, 36);
    logoIcon->setStyleSheet("background:#0284C7; border-radius:8px; font-size:20px; color:#FFFFFF;");
    bl->addWidget(logoIcon);

    auto* brandCol = new QVBoxLayout;
    brandCol->setContentsMargins(0, 0, 0, 0);
    brandCol->setSpacing(1);
    auto* brandTitle = new QLabel(QString::fromUtf8("QUẢN LÝ GIA SƯ"), brandWidget);
    brandTitle->setStyleSheet(
        "color:#FFFFFF; font-size:14px; font-weight:800; letter-spacing:0.5px; background:transparent;");
    auto* brandSub = new QLabel(QString::fromUtf8("TUTORFLOW v2.4"), brandWidget);
    brandSub->setStyleSheet("color:#64748B; font-size:10px; font-weight:600; background:transparent;");
    brandCol->addWidget(brandTitle);
    brandCol->addWidget(brandSub);
    bl->addLayout(brandCol);
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
            "QPushButton:checked { background:%3; color:#FFFFFF; font-weight:700; }")
            .arg(SIDEBAR_TEXT, SIDEBAR_HOVER, SIDEBAR_ACTIVE));

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
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Tổng học sinh"), "125", "+12%", QString::fromUtf8(" so với tháng trước"), QString::fromUtf8("👥"), "#F1F5F9", "#475569", true), 1);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Tổng gia sư"), "68", "+5%", QString::fromUtf8(" so với tháng trước"), QString::fromUtf8("🎓"), "#E0F2FE", "#0284C7", true), 1);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Lớp đang hoạt động"), "42", "-2%", QString::fromUtf8(" so với tháng trước"), QString::fromUtf8("📥"), "#F1F5F9", "#475569", false), 1);
    metrics->addWidget(makeMetricCard(QString::fromUtf8("Tỷ lệ ghép thành công"), "78%", "", "", QString::fromUtf8("🤝"), "#E0F2FE", "#0284C7", true, true, 78), 1);
    l->addLayout(metrics);

    // Lower Section: Two Columns
    auto* body = new QHBoxLayout;
    body->setSpacing(16);

    // Left Column
    auto* leftCol = new QVBoxLayout;
    leftCol->setSpacing(14);

    // Card 1: Trạng thái học sinh
    auto* studentStatusCard = card();
    studentStatusCard->setFixedWidth(330);
    auto* scLayout = new QVBoxLayout(studentStatusCard);
    scLayout->setContentsMargins(18, 18, 18, 18);
    scLayout->setSpacing(14);
    scLayout->addWidget(label(QString::fromUtf8("Trạng thái học sinh"), 15, true, TEXT_MAIN));

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

    auto* row2 = new QHBoxLayout;
    auto* dot2 = new QLabel(QString::fromUtf8("●  Chưa ghép"), studentStatusCard);
    dot2->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(TEXT_MUTED));
    auto* val2 = label("27", 15, true, TEXT_MAIN);
    row2->addWidget(dot2);
    row2->addStretch();
    row2->addWidget(val2);
    scLayout->addLayout(row2);
    scLayout->addWidget(progress(22, "#94A3B8", 6));

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

    // Right Column
    auto* rightCol = new QVBoxLayout;
    rightCol->setSpacing(14);

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

    struct RowData {
        QString code; QString initials; QString studentName; QString avatarBg; QString avatarFg;
        QString tutorName; QString subject; QString statusText; QString statusBg; QString statusFg;
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
        auto* codeLabel = label(d.code, 12, true, BLUE);
        table->setCellWidget(i, 0, codeLabel);
        table->setCellWidget(i, 1, userAvatarItem(d.initials, d.studentName, d.avatarBg, d.avatarFg));
        table->setCellWidget(i, 2, label(d.tutorName, 12, false, TEXT_SUBTLE));
        table->setCellWidget(i, 3, label(d.subject, 12, false, TEXT_MAIN));
        table->setCellWidget(i, 4, pillBadge(d.statusText, d.statusBg, d.statusFg));

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

    // Banner: Chương trình đào tạo mới
    auto* bannerCard = card();
    auto* bcLayout = new QHBoxLayout(bannerCard);
    bcLayout->setContentsMargins(20, 16, 20, 16);
    bcLayout->setSpacing(16);

    auto* bannerTextCol = new QVBoxLayout;
    bannerTextCol->setContentsMargins(0, 0, 0, 0);
    bannerTextCol->setSpacing(4);
    bannerTextCol->addWidget(label(QString::fromUtf8("Chương trình đào tạo mới"), 14, true, TEXT_MAIN));
    auto* bannerDesc = label(
        QString::fromUtf8("Cập nhật tài liệu giảng dạy môn Toán cấp 3 theo chương trình GDPT 2018. "
                          "Vui lòng thông báo cho các gia sư liên quan."), 12, false, TEXT_MUTED);
    bannerDesc->setWordWrap(true);
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

// -------------------------------------------------------------
// IMAGE 2: QUẢN LÝ HỌC SINH
// -------------------------------------------------------------
QWidget* MainWindow::createStudentPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    // Top Header Banner Card
    auto* topCard = new QFrame(content);
    topCard->setStyleSheet("background:#F1F5F9; border:1px solid #E2E8F0; border-radius:10px;");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý học sinh"), 22, true, TEXT_MAIN);
    auto* pageDesc = label(QString::fromUtf8("Quản lý danh sách, tìm kiếm và phân bổ học sinh cho gia sư."), 13, false, TEXT_MUTED);
    tCol->addWidget(pageTitle);
    tCol->addWidget(pageDesc);
    topLayout->addLayout(tCol, 1);

    auto* btnAddStudent = new QPushButton(QString::fromUtf8("＋  Thêm học sinh"), topCard);
    btnAddStudent->setFixedHeight(40);
    btnAddStudent->setCursor(Qt::PointingHandCursor);
    btnAddStudent->setStyleSheet(
        "QPushButton { background:#0F172A; color:white; border:none; border-radius:6px; "
        "padding:0 20px; font-size:13px; font-weight:600; } QPushButton:hover { background:#1E293B; }");
    topLayout->addWidget(btnAddStudent);
    l->addWidget(topCard);

    // Filter Bar Card
    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);

    auto* searchBox = new QLineEdit(filters);
    searchBox->setPlaceholderText(QString::fromUtf8("🔍  Tìm kiếm theo mã, tên học sinh..."));
    searchBox->setFixedHeight(38);
    searchBox->setStyleSheet(
        "QLineEdit { background:white; border:1px solid #DDE4EC; border-radius:6px; "
        "padding:0 12px; font-size:12px; color:#344054; } QLineEdit:focus { border:1px solid #0284C7; }");
    fl->addWidget(searchBox, 1);

    fl->addWidget(combo({QString::fromUtf8("Tất cả khối lớp"), "Lớp 10", "Lớp 11", "Lớp 12", "Lớp 5"}, 160));
    fl->addWidget(combo({QString::fromUtf8("Trạng thái"), QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")}, 140));
    l->addWidget(filters);

    // Table Card
    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(18, 14, 18, 14);

    auto* table = new QTableWidget(3, 7, tableCard);
    table->setHorizontalHeaderLabels({
        QString::fromUtf8("Mã HS"),
        QString::fromUtf8("Học sinh"),
        QString::fromUtf8("Lớp & Môn"),
        QString::fromUtf8("Lịch rảnh"),
        QString::fromUtf8("Ngân sách"),
        QString::fromUtf8("Trạng thái"),
        QString::fromUtf8("Thao tác")
    });
    styleTable(table);
    table->setFixedHeight(220);

    // Helper for schedule day pill badges
    auto makeScheduleWidget = [](const QStringList& days) {
        auto* w = new QWidget;
        auto* l = new QHBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(6);
        l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        for (const auto& d : days) {
            auto* p = new QLabel(d, w);
            p->setAlignment(Qt::AlignCenter);
            p->setFixedSize(30, 24);
            p->setStyleSheet(
                "background:#F1F5F9; color:#475569; border-radius:4px; font-size:11px; font-weight:600;");
            l->addWidget(p);
        }
        return w;
    };

    // Helper for Student cell (avatar + name + phone)
    auto makeStudentItem = [](const QString& initial, const QString& name, const QString& phone,
                              const QString& bg, const QString& fg) {
        auto* w = new QWidget;
        auto* l = new QHBoxLayout(w);
        l->setContentsMargins(4, 4, 4, 4);
        l->setSpacing(10);

        auto* av = new QLabel(initial, w);
        av->setAlignment(Qt::AlignCenter);
        av->setFixedSize(32, 32);
        av->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:16px; font-size:12px; font-weight:700;").arg(bg, fg));
        l->addWidget(av);

        auto* texts = new QVBoxLayout;
        texts->setContentsMargins(0, 0, 0, 0);
        texts->setSpacing(1);
        auto* n = label(name, 13, true, TEXT_MAIN);
        auto* p = label(phone, 11, false, TEXT_MUTED);
        texts->addWidget(n);
        texts->addWidget(p);
        l->addLayout(texts);
        l->addStretch();
        return w;
    };

    // Helper for Grade & Subject
    auto makeSubjectItem = [](const QString& grade, const QString& subject) {
        auto* w = new QWidget;
        auto* l = new QVBoxLayout(w);
        l->setContentsMargins(4, 4, 4, 4);
        l->setSpacing(2);

        auto* g = new QLabel(grade, w);
        g->setStyleSheet("background:#F1F5F9; color:#475569; border-radius:4px; padding:2px 6px; font-size:10px; font-weight:600;");
        g->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        auto* s = label(subject, 13, true, TEXT_MAIN);
        l->addWidget(g);
        l->addWidget(s);
        return w;
    };

    // Row 1: HS001
    table->setCellWidget(0, 0, label("HS001", 12, false, TEXT_MUTED));
    table->setCellWidget(0, 1, makeStudentItem("N", QString::fromUtf8("Nguyễn Minh Anh"), "0901234567", "#DBEAFE", "#1E40AF"));
    table->setCellWidget(0, 2, makeSubjectItem(QString::fromUtf8("Lớp 10"), QString::fromUtf8("Toán, Lý")));
    table->setCellWidget(0, 3, makeScheduleWidget({"T2", "T4", "T6"}));
    table->setCellWidget(0, 4, label(QString::fromUtf8("200.000đ/h"), 13, true, BLUE));
    table->setCellWidget(0, 5, pillBadge(QString::fromUtf8("Chưa ghép"), PEACH_BG, PEACH_TEXT, true));
    table->setCellWidget(0, 6, new QLabel(""));

    // Row 2: HS002
    table->setCellWidget(1, 0, label("HS002", 12, false, TEXT_MUTED));
    table->setCellWidget(1, 1, makeStudentItem("T", QString::fromUtf8("Trần Văn Bình"), "0987654321", "#E2E8F0", "#475569"));
    table->setCellWidget(1, 2, makeSubjectItem(QString::fromUtf8("Lớp 12"), QString::fromUtf8("Tiếng Anh")));
    table->setCellWidget(1, 3, makeScheduleWidget({"T7", "CN"}));
    table->setCellWidget(1, 4, label(QString::fromUtf8("250.000đ/h"), 13, true, BLUE));
    table->setCellWidget(1, 5, pillBadge(QString::fromUtf8("Đã ghép"), GREEN_BG, GREEN, true));
    table->setCellWidget(1, 6, new QLabel(""));

    // Row 3: HS003
    table->setCellWidget(2, 0, label("HS003", 12, false, TEXT_MUTED));
    table->setCellWidget(2, 1, makeStudentItem("L", QString::fromUtf8("Lê Thị Cẩm"), "0912345888", "#E2E8F0", "#475569"));
    table->setCellWidget(2, 2, makeSubjectItem(QString::fromUtf8("Lớp 5"), QString::fromUtf8("Toán, Tiếng Việt")));
    table->setCellWidget(2, 3, makeScheduleWidget({"T3", "T5"}));
    table->setCellWidget(2, 4, label(QString::fromUtf8("150.000đ/h"), 13, true, BLUE));
    table->setCellWidget(2, 5, pillBadge(QString::fromUtf8("Chưa ghép"), PEACH_BG, PEACH_TEXT, true));
    table->setCellWidget(2, 6, new QLabel(""));

    tl->addWidget(table);
    l->addWidget(tableCard);

    // Pagination Footer
    auto* pagination = new QHBoxLayout;
    pagination->addWidget(label(QString::fromUtf8("Hiển thị 1-3 trong 24 học sinh"), 12, false, TEXT_MUTED));
    pagination->addStretch();

    auto makePageBtn = [](const QString& t, bool active = false) {
        auto* b = new QPushButton(t);
        b->setFixedSize(32, 32);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(active
            ? "background:#E2E8F0; color:#0F172A; border:1px solid #CBD5E1; border-radius:4px; font-weight:700;"
            : "background:white; color:#64748B; border:1px solid #E2E8F0; border-radius:4px; font-weight:500;");
        return b;
    };

    pagination->addWidget(makePageBtn("<"));
    pagination->addWidget(makePageBtn("1", true));
    pagination->addWidget(makePageBtn("2"));
    pagination->addWidget(makePageBtn("3"));
    pagination->addWidget(makePageBtn(">"));
    l->addLayout(pagination);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// IMAGE 1: TÌM GIA SƯ THÔNG MINH (SMART MATCHING)
// -------------------------------------------------------------
QWidget* MainWindow::createMatchingPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    // Title Block
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(3);
    tCol->addWidget(label(QString::fromUtf8("Tìm Gia Sư Thông Minh"), 24, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Hệ thống tự động phân tích và đề xuất gia sư phù hợp nhất dựa trên yêu cầu của học sinh và hồ sơ năng lực của gia sư."), 13, false, TEXT_MUTED));
    l->addLayout(tCol);

    // Top Requirement Section (Step 1 + Student Requirements)
    auto* reqRow = new QHBoxLayout;
    reqRow->setSpacing(14);

    // Left Card: BƯỚC 1: CHỌN HỌC SINH
    auto* studentCard = card();
    studentCard->setFixedWidth(360);
    auto* scLayout = new QVBoxLayout(studentCard);
    scLayout->setContentsMargins(18, 16, 18, 16);
    scLayout->setSpacing(10);

    auto* scHead = new QHBoxLayout;
    auto* scTitle = label(QString::fromUtf8("👤  BƯỚC 1: CHỌN HỌC SINH"), 12, true, TEXT_MAIN);
    auto* reqBadge = pillBadge("REQ-001", "#E0F2FE", BLUE);
    scHead->addWidget(scTitle);
    scHead->addStretch();
    scHead->addWidget(reqBadge);
    scLayout->addLayout(scHead);

    // Student selection item card
    auto* selBox = new QFrame(studentCard);
    selBox->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* selLayout = new QHBoxLayout(selBox);
    selLayout->setContentsMargins(10, 10, 10, 10);
    selLayout->setSpacing(10);

    auto* av = new QLabel("MA", selBox);
    av->setAlignment(Qt::AlignCenter);
    av->setFixedSize(34, 34);
    av->setStyleSheet("background:#0284C7; color:white; border-radius:17px; font-size:12px; font-weight:700;");
    selLayout->addWidget(av);

    auto* sTexts = new QVBoxLayout;
    sTexts->setContentsMargins(0, 0, 0, 0);
    sTexts->setSpacing(1);
    auto* sName = label(QString::fromUtf8("SV001 - Nguyễn Minh Anh"), 13, true, TEXT_MAIN);
    auto* sStat = label(QString::fromUtf8("Trạng thái: Đang tìm gia sư"), 11, false, TEXT_MUTED);
    sTexts->addWidget(sName);
    sTexts->addWidget(sStat);
    selLayout->addLayout(sTexts);
    selLayout->addStretch();

    auto* arrow = new QLabel(QString::fromUtf8("⌄"), selBox);
    arrow->setStyleSheet("font-size:16px; color:#64748B; font-weight:bold;");
    selLayout->addWidget(arrow);

    scLayout->addWidget(selBox);
    reqRow->addWidget(studentCard);

    // Right Card: YÊU CẦU CỦA HỌC SINH
    auto* reqCard = card();
    auto* rcLayout = new QVBoxLayout(reqCard);
    rcLayout->setContentsMargins(18, 16, 18, 16);
    rcLayout->setSpacing(10);

    rcLayout->addWidget(label(QString::fromUtf8("📋  YÊU CẦU CỦA HỌC SINH"), 12, true, TEXT_MAIN));

    auto* tilesRow = new QHBoxLayout;
    tilesRow->setSpacing(10);

    // Tile 1: Môn học & Lớp
    auto* t1 = new QFrame(reqCard);
    t1->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* t1l = new QVBoxLayout(t1);
    t1l->setContentsMargins(12, 10, 12, 10);
    t1l->addWidget(label(QString::fromUtf8("MÔN HỌC & LỚP"), 10, true, TEXT_MUTED));
    auto* t1v = new QHBoxLayout;
    t1v->addWidget(label(QString::fromUtf8("Toán Học"), 13, true, TEXT_MAIN));
    auto* gradeBadge = new QLabel(QString::fromUtf8("Lớp 10"), t1);
    gradeBadge->setStyleSheet("background:#0F172A; color:white; border-radius:4px; padding:2px 6px; font-size:10px; font-weight:700;");
    t1v->addWidget(gradeBadge);
    t1v->addStretch();
    t1l->addLayout(t1v);
    tilesRow->addWidget(t1, 1);

    // Tile 2: Khu vực
    auto* t2 = new QFrame(reqCard);
    t2->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* t2l = new QVBoxLayout(t2);
    t2l->setContentsMargins(12, 10, 12, 10);
    t2l->addWidget(label(QString::fromUtf8("KHU VỰC"), 10, true, TEXT_MUTED));
    auto* t2v = new QHBoxLayout;
    auto* pin = new QLabel(QString::fromUtf8("📍"), t2);
    pin->setStyleSheet("color:#DC2626; font-size:12px;");
    t2v->addWidget(pin);
    t2v->addWidget(label(QString::fromUtf8("Hải Châu"), 13, true, TEXT_MAIN));
    t2v->addStretch();
    t2l->addLayout(t2v);
    tilesRow->addWidget(t2, 1);

    // Tile 3: Ngân sách
    auto* t3 = new QFrame(reqCard);
    t3->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* t3l = new QVBoxLayout(t3);
    t3l->setContentsMargins(12, 10, 12, 10);
    t3l->addWidget(label(QString::fromUtf8("NGÂN SÁCH (BUỔI)"), 10, true, TEXT_MUTED));
    t3l->addWidget(label(QString::fromUtf8("200,000đ"), 14, true, BLUE));
    tilesRow->addWidget(t3, 1);

    // Tile 4: Lịch học yêu cầu
    auto* t4 = new QFrame(reqCard);
    t4->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* t4l = new QVBoxLayout(t4);
    t4l->setContentsMargins(12, 10, 12, 10);
    t4l->addWidget(label(QString::fromUtf8("LỊCH HỌC YÊU CẦU"), 10, true, TEXT_MUTED));

    auto* daysLayout = new QHBoxLayout;
    daysLayout->setSpacing(4);
    daysLayout->setContentsMargins(0, 0, 0, 0);

    struct DayItem { QString d; bool act; };
    QVector<DayItem> days = {{"T2", true}, {"T3", false}, {"T4", true}, {"T5", false}, {"T6", true}};
    for (const auto& item : days) {
        auto* dl = new QLabel(item.d, t4);
        dl->setAlignment(Qt::AlignCenter);
        dl->setFixedSize(24, 20);
        dl->setStyleSheet(item.act
            ? "background:#0284C7; color:white; border-radius:3px; font-size:10px; font-weight:700;"
            : "background:#E2E8F0; color:#94A3B8; border-radius:3px; font-size:10px; font-weight:600;");
        daysLayout->addWidget(dl);
    }
    daysLayout->addStretch();
    t4l->addLayout(daysLayout);
    tilesRow->addWidget(t4, 1);

    rcLayout->addLayout(tilesRow);
    reqRow->addWidget(reqCard, 1);
    l->addLayout(reqRow);

    // Center Action Button
    auto* findBtn = new QPushButton(QString::fromUtf8("↺   TÌM GIA SƯ PHÙ HỢP"), content);
    findBtn->setFixedSize(250, 44);
    findBtn->setCursor(Qt::PointingHandCursor);
    findBtn->setStyleSheet(
        "QPushButton { background:#060D1E; color:white; border:none; border-radius:22px; "
        "font-size:13px; font-weight:700; letter-spacing:0.5px; } QPushButton:hover { background:#111E38; }");
    l->addWidget(findBtn, 0, Qt::AlignCenter);

    // Matching Results Section Header
    auto* resHeader = new QHBoxLayout;
    resHeader->addWidget(label(QString::fromUtf8("💫  Kết Quả Matching"), 16, true, TEXT_MAIN));
    resHeader->addStretch();
    resHeader->addWidget(pillBadge(QString::fromUtf8("Tìm thấy 12 gia sư phù hợp"), "#F1F5F9", "#64748B"));
    l->addLayout(resHeader);

    // Result Details: Two Columns
    auto* matchBody = new QHBoxLayout;
    matchBody->setSpacing(14);

    // Left Candidate Card (Dark Theme)
    auto* candidateCard = new QFrame(content);
    candidateCard->setFixedWidth(330);
    candidateCard->setStyleSheet(QString("background:%1; border-radius:12px;").arg(SIDEBAR_BG));
    auto* candLayout = new QVBoxLayout(candidateCard);
    candLayout->setContentsMargins(20, 20, 20, 20);
    candLayout->setSpacing(10);
    candLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // TOP 1 yellow ribbon
    auto* topRibbon = new QLabel("TOP 1", candidateCard);
    topRibbon->setAlignment(Qt::AlignCenter);
    topRibbon->setFixedSize(54, 26);
    topRibbon->setStyleSheet("background:#EAB308; color:#000000; font-size:11px; font-weight:800; border-radius:4px;");
    candLayout->addWidget(topRibbon, 0, Qt::AlignLeft);

    // Tutor Avatar Photo Frame
    auto* tutorPhoto = new QLabel(QString::fromUtf8("👨‍🏫"), candidateCard);
    tutorPhoto->setAlignment(Qt::AlignCenter);
    tutorPhoto->setFixedSize(90, 90);
    tutorPhoto->setStyleSheet(
        "background:#111E38; border:2px solid #38BDF8; border-radius:12px; font-size:44px;");
    candLayout->addWidget(tutorPhoto, 0, Qt::AlignCenter);

    auto* gsBadge = new QLabel("GS003", candidateCard);
    gsBadge->setAlignment(Qt::AlignCenter);
    gsBadge->setFixedSize(65, 22);
    gsBadge->setStyleSheet("background:#111E38; color:#94A3B8; border-radius:11px; font-size:10px; font-weight:700;");
    candLayout->addWidget(gsBadge, 0, Qt::AlignCenter);

    auto* cName = new QLabel(QString::fromUtf8("Trần Quốc Huy"), candidateCard);
    cName->setAlignment(Qt::AlignCenter);
    cName->setStyleSheet("color:#FFFFFF; font-size:20px; font-weight:800; background:transparent;");
    candLayout->addWidget(cName);

    auto* cDegree = new QLabel(QString::fromUtf8("🎓 Cử nhân Sư phạm Toán"), candidateCard);
    cDegree->setAlignment(Qt::AlignCenter);
    cDegree->setStyleSheet("color:#38BDF8; font-size:12px; font-weight:500; background:transparent;");
    candLayout->addWidget(cDegree);

    candLayout->addSpacing(8);

    // Circular Donut Progress (95% Fit)
    auto* donut = new DonutChartWidget(95, QColor("#0284C7"), QColor("#111E38"), "95%", QString::fromUtf8("ĐỘ PHÙ HỢP"), true, candidateCard);
    candLayout->addWidget(donut, 0, Qt::AlignCenter);

    candLayout->addSpacing(6);
    auto* botNote = new QLabel(QString::fromUtf8("Được đề xuất tự động từ thuật toán"), candidateCard);
    botNote->setAlignment(Qt::AlignCenter);
    botNote->setStyleSheet("color:#64748B; font-size:11px; background:transparent;");
    candLayout->addWidget(botNote);

    matchBody->addWidget(candidateCard);

    // Right Details Column
    auto* matchRight = card();
    auto* mrLayout = new QVBoxLayout(matchRight);
    mrLayout->setContentsMargins(22, 20, 22, 20);
    mrLayout->setSpacing(14);

    // 4 Summary Metrics
    auto* grid4 = new QHBoxLayout;
    grid4->setSpacing(10);

    auto makeStatBox = [](const QString& t, const QString& val, const QString& valColor = TEXT_MAIN) {
        auto* b = new QFrame;
        b->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
        auto* l = new QVBoxLayout(b);
        l->setContentsMargins(14, 10, 14, 10);
        l->addWidget(label(t, 10, true, TEXT_MUTED));
        l->addWidget(label(val, 16, true, valColor));
        return b;
    };

    grid4->addWidget(makeStatBox(QString::fromUtf8("KINH NGHIỆM"), QString::fromUtf8("8 Năm")));
    grid4->addWidget(makeStatBox(QString::fromUtf8("KHU VỰC"), QString::fromUtf8("Hải Châu")));
    grid4->addWidget(makeStatBox(QString::fromUtf8("HỌC PHÍ"), QString::fromUtf8("170k/buổi"), BLUE));
    grid4->addWidget(makeStatBox(QString::fromUtf8("LỊCH TRỐNG"), QString::fromUtf8("T2, T4, T6")));
    mrLayout->addLayout(grid4);

    // Criteria Breakdown Grid
    mrLayout->addWidget(label(QString::fromUtf8("CHI TIẾT ĐIỂM MATCHING"), 12, true, TEXT_MUTED));

    auto* matchGrid = new QGridLayout;
    matchGrid->setHorizontalSpacing(24);
    matchGrid->setVerticalSpacing(10);

    auto addScoreRow = [matchGrid](int row, int col, const QString& title, const QString& score, int val) {
        auto* block = new QVBoxLayout;
        block->setSpacing(4);
        auto* top = new QHBoxLayout;
        top->addWidget(label(title, 12, false, TEXT_MAIN));
        top->addStretch();
        top->addWidget(label(score, 12, true, BLUE));
        block->addLayout(top);
        block->addWidget(progress(val, BLUE, 6));
        matchGrid->addLayout(block, row, col);
    };

    addScoreRow(0, 0, QString::fromUtf8("Kinh nghiệm chuyên môn"), "25/25", 100);
    addScoreRow(0, 1, QString::fromUtf8("Khu vực địa lý"), "10/10", 100);
    addScoreRow(1, 0, QString::fromUtf8("Học phí yêu cầu"), "40/45", 88);
    addScoreRow(1, 1, QString::fromUtf8("Lịch học trùng khớp"), "20/20", 100);

    mrLayout->addLayout(matchGrid);

    // "VÌ SAO GIA SƯ NÀY PHÙ HỢP?" Card
    auto* whyCard = new QFrame(matchRight);
    whyCard->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* wl = new QVBoxLayout(whyCard);
    wl->setContentsMargins(14, 12, 14, 12);
    wl->setSpacing(8);

    auto* whyTitle = new QHBoxLayout;
    auto* bulb = new QLabel(QString::fromUtf8("💡"), whyCard);
    bulb->setStyleSheet("color:#0284C7; font-size:14px;");
    whyTitle->addWidget(bulb);
    whyTitle->addWidget(label(QString::fromUtf8("VÌ SAO GIA SƯ NÀY PHÙ HỢP?"), 11, true, BLUE));
    whyTitle->addStretch();
    wl->addLayout(whyTitle);

    auto addWhyPoint = [wl](const QString& text) {
        auto* r = new QHBoxLayout;
        r->setSpacing(8);
        auto* chk = new QLabel(QString::fromUtf8("✔"), wl->parentWidget());
        chk->setStyleSheet("color:#16A34A; font-weight:bold; font-size:12px;");
        auto* t = label(text, 12, false, TEXT_MAIN);
        t->setWordWrap(true);
        r->addWidget(chk, 0, Qt::AlignTop);
        r->addWidget(t, 1);
        wl->addLayout(r);
    };

    addWhyPoint(QString::fromUtf8("Học phí 170k thấp hơn ngân sách tối đa (200k), tiết kiệm cho học viên."));
    addWhyPoint(QString::fromUtf8("Lịch dạy trống hoàn toàn trùng khớp với yêu cầu (T2, T4, T6)."));
    addWhyPoint(QString::fromUtf8("Kinh nghiệm 8 năm chuyên luyện thi môn Toán Lớp 10 tại khu vực Hải Châu."));

    mrLayout->addWidget(whyCard);

    // Action buttons
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto* btnDetail = new QPushButton(QString::fromUtf8("👁  Xem Chi Tiết"), matchRight);
    btnDetail->setFixedHeight(40);
    btnDetail->setCursor(Qt::PointingHandCursor);
    btnDetail->setStyleSheet(
        "QPushButton { background:white; color:#0F172A; border:1px solid #CBD5E1; border-radius:6px; "
        "padding:0 18px; font-size:12px; font-weight:600; } QPushButton:hover { background:#F8FAFC; }");

    auto* btnAccept = new QPushButton(QString::fromUtf8("🤝  CHỌN GIA SƯ NÀY"), matchRight);
    btnAccept->setFixedHeight(40);
    btnAccept->setCursor(Qt::PointingHandCursor);
    btnAccept->setStyleSheet(
        "QPushButton { background:#0284C7; color:white; border:none; border-radius:6px; "
        "padding:0 24px; font-size:12px; font-weight:700; } QPushButton:hover { background:#0369A1; }");

    btnRow->addWidget(btnDetail);
    btnRow->addWidget(btnAccept);
    mrLayout->addLayout(btnRow);

    matchBody->addWidget(matchRight, 1);
    l->addLayout(matchBody);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// IMAGE 3: THỐNG KÊ HỆ THỐNG
// -------------------------------------------------------------
QWidget* MainWindow::createStatisticsPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(18);

    // Title Row
    auto* titleRow = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("Thống kê hệ thống"), 24, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Tổng quan dữ liệu hoạt động của nền tảng quản lý gia sư. Các chỉ số được cập nhật theo thời gian thực để hỗ trợ ra quyết định nhanh chóng."), 13, false, TEXT_MUTED));
    titleRow->addLayout(tCol, 1);

    auto* timeDropdown = combo({QString::fromUtf8("📅  30 ngày qua"), QString::fromUtf8("7 ngày qua"), QString::fromUtf8("Quý này")}, 145);
    auto* exportBtn = new QPushButton(QString::fromUtf8("📥  Xuất báo cáo"), content);
    exportBtn->setFixedHeight(38);
    exportBtn->setCursor(Qt::PointingHandCursor);
    exportBtn->setStyleSheet(
        "QPushButton { background:#0F172A; color:white; border:none; border-radius:6px; "
        "padding:0 18px; font-size:12px; font-weight:600; } QPushButton:hover { background:#1E293B; }");

    titleRow->addWidget(timeDropdown);
    titleRow->addWidget(exportBtn);
    l->addLayout(titleRow);

    // 5 Metric Cards in a Row
    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(12);

    auto makeStatCard = [](const QString& iconEmoji, const QString& trend, const QString& val,
                           const QString& sub, const QString& iconBg = "#DBEAFE", const QString& iconFg = "#0284C7",
                           bool isDark = false) {
        auto* f = new QFrame;
        f->setStyleSheet(isDark
            ? QString("background:%1; border-radius:12px;").arg(SIDEBAR_BG)
            : "background:#FFFFFF; border:1px solid #E2E8F0; border-radius:12px;");
        auto* l = new QVBoxLayout(f);
        l->setContentsMargins(16, 14, 16, 14);
        l->setSpacing(8);

        auto* top = new QHBoxLayout;
        auto* i = new QLabel(iconEmoji, f);
        i->setAlignment(Qt::AlignCenter);
        i->setFixedSize(36, 36);
        i->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:8px; font-size:16px;").arg(iconBg, iconFg));
        top->addWidget(i);
        top->addStretch();

        if (!trend.isEmpty()) {
            auto* tr = new QLabel(trend, f);
            tr->setAlignment(Qt::AlignCenter);
            tr->setStyleSheet(trend.contains("LIVE")
                ? "background:#052E16; color:#22C55E; border-radius:10px; padding:2px 8px; font-size:10px; font-weight:800;"
                : "background:#F1F5F9; color:#475569; border-radius:10px; padding:2px 8px; font-size:10px; font-weight:700;");
            top->addWidget(tr);
        }
        l->addLayout(top);

        auto* num = label(val, 24, true, isDark ? "#FFFFFF" : TEXT_MAIN);
        auto* desc = label(sub, 12, false, isDark ? "#94A3B8" : TEXT_MUTED);
        l->addWidget(num);
        l->addWidget(desc);
        return f;
    };

    metrics->addWidget(makeStatCard(QString::fromUtf8("👥"), "+12%", "2,543", QString::fromUtf8("Tổng học sinh"), "#DBEAFE", "#0284C7"), 1);
    metrics->addWidget(makeStatCard(QString::fromUtf8("👨‍🏫"), "+5%", "842", QString::fromUtf8("Tổng gia sư"), "#0F172A", "#FFFFFF"), 1);
    // Dark highlight card
    metrics->addWidget(makeStatCard(QString::fromUtf8("👤"), QString::fromUtf8("● LIVE"), "315", QString::fromUtf8("Gia sư sẵn sàng nhận lớp"), "#111E38", "#38BDF8", true), 1);
    metrics->addWidget(makeStatCard(QString::fromUtf8("📖"), "+18%", "1,120", QString::fromUtf8("Lớp đang hoạt động"), "#DBEAFE", "#0284C7"), 1);
    metrics->addWidget(makeStatCard(QString::fromUtf8("⚙"), QString::fromUtf8("Cao"), "86 %", QString::fromUtf8("Tỷ lệ ghép thành công"), "#FEF3C7", "#D97706"), 1);

    l->addLayout(metrics);

    // Two Column Analytics Section
    auto* statsBody = new QHBoxLayout;
    statsBody->setSpacing(14);

    // Left Column: Tình trạng ghép nối
    auto* matchCard = card();
    matchCard->setFixedWidth(340);
    auto* mcLayout = new QVBoxLayout(matchCard);
    mcLayout->setContentsMargins(20, 18, 20, 18);
    mcLayout->setSpacing(14);

    auto* mcHead = new QHBoxLayout;
    mcHead->addWidget(label(QString::fromUtf8("Tình trạng ghép nối"), 15, true, TEXT_MAIN));
    mcHead->addStretch();
    auto* menuDot = new QLabel(QString::fromUtf8("⋮"), matchCard);
    menuDot->setStyleSheet("color:#64748B; font-size:18px; font-weight:bold;");
    mcHead->addWidget(menuDot);
    mcLayout->addLayout(mcHead);

    // Big Donut Chart (86%)
    auto* donutStat = new DonutChartWidget(86, QColor("#060D1E"), QColor("#E2E8F0"), "86%", QString::fromUtf8("ĐÃ GHÉP NỐI"), false, matchCard);
    mcLayout->addWidget(donutStat, 0, Qt::AlignCenter);

    mcLayout->addSpacing(8);

    // Status items
    auto* sRow1 = new QHBoxLayout;
    sRow1->addWidget(label(QString::fromUtf8("●  Lớp đã có gia sư"), 13, false, TEXT_MAIN));
    sRow1->addStretch();
    sRow1->addWidget(label("1,420", 14, true, TEXT_MAIN));
    mcLayout->addLayout(sRow1);

    auto* sRow2 = new QHBoxLayout;
    auto* dotGray = new QLabel(QString::fromUtf8("●  Đang chờ ghép"), matchCard);
    dotGray->setStyleSheet("color:#94A3B8; font-size:13px;");
    sRow2->addWidget(dotGray);
    sRow2->addStretch();
    sRow2->addWidget(label("231", 14, true, TEXT_MAIN));
    mcLayout->addLayout(sRow2);

    statsBody->addWidget(matchCard);

    // Right Column: Two stacked cards
    auto* rightStatsCol = new QVBoxLayout;
    rightStatsCol->setSpacing(14);

    // Card 1: Trạng thái lớp học
    auto* stagesCard = card();
    auto* stLayout = new QVBoxLayout(stagesCard);
    stLayout->setContentsMargins(20, 18, 20, 18);
    stLayout->setSpacing(12);

    auto* stHead = new QHBoxLayout;
    auto* stText = new QVBoxLayout;
    stText->setSpacing(2);
    stText->addWidget(label(QString::fromUtf8("Trạng thái lớp học"), 15, true, TEXT_MAIN));
    stText->addWidget(label(QString::fromUtf8("Phân bổ theo giai đoạn hoạt động"), 12, false, TEXT_MUTED));
    stHead->addLayout(stText);
    stHead->addStretch();
    auto* pillWeek = new QLabel(QString::fromUtf8("Tuần này"), stagesCard);
    pillWeek->setStyleSheet("background:#F1F5F9; color:#475569; border-radius:4px; padding:4px 10px; font-size:11px; font-weight:600;");
    stHead->addWidget(pillWeek);
    stLayout->addLayout(stHead);

    auto* barStages = new QHBoxLayout;
    barStages->setSpacing(12);

    auto makeStageBar = [](const QString& name, const QString& barColor) {
        auto* c = new QVBoxLayout;
        c->setSpacing(6);
        auto* bar = new QFrame;
        bar->setFixedHeight(7);
        bar->setStyleSheet(QString("background:%1; border-radius:3px;").arg(barColor));
        auto* l = label(name, 11, true, TEXT_MUTED);
        l->setAlignment(Qt::AlignCenter);
        c->addWidget(bar);
        c->addWidget(l);
        return c;
    };

    barStages->addLayout(makeStageBar(QString::fromUtf8("CHỜ BẮT ĐẦU"), "#93C5FD"), 1);
    barStages->addLayout(makeStageBar(QString::fromUtf8("ĐANG HĐ"), "#060D1E"), 1);
    barStages->addLayout(makeStageBar(QString::fromUtf8("HOÀN THÀNH"), "#64748B"), 1);
    barStages->addLayout(makeStageBar(QString::fromUtf8("HỦY"), "#E2E8F0"), 1);
    stLayout->addLayout(barStages);
    rightStatsCol->addWidget(stagesCard);

    // Card 2: Phân bổ gia sư theo môn học
    auto* subjectsCard = card();
    auto* subLayout = new QVBoxLayout(subjectsCard);
    subLayout->setContentsMargins(20, 18, 20, 18);
    subLayout->setSpacing(10);

    auto* subHead = new QHBoxLayout;
    subHead->addWidget(label(QString::fromUtf8("Phân bổ gia sư theo môn học"), 15, true, TEXT_MAIN));
    subHead->addStretch();
    auto* subLink = new QLabel(QString::fromUtf8("Xem chi tiết"), subjectsCard);
    subLink->setStyleSheet("color:#64748B; font-size:12px; font-weight:600;");
    subHead->addWidget(subLink);
    subLayout->addLayout(subHead);

    struct SubjRow { QString icon; QString name; int pct; int count; QString color; };
    const QVector<SubjRow> sRows = {
        {QString::fromUtf8("⊞"), QString::fromUtf8("Toán Học"), 32, 269, "#060D1E"},
        {QString::fromUtf8("🌐"), QString::fromUtf8("Tiếng Anh"), 28, 235, "#0284C7"},
        {QString::fromUtf8("🔬"), QString::fromUtf8("Vật Lý"), 18, 151, "#1E3A8A"},
        {QString::fromUtf8("⚗"), QString::fromUtf8("Hóa Học"), 14, 118, "#38BDF8"},
        {QString::fromUtf8("📖"), QString::fromUtf8("Ngữ Văn"), 8, 69, "#CBD5E1"}
    };

    for (const auto& sr : sRows) {
        auto* rLayout = new QVBoxLayout;
        rLayout->setSpacing(3);
        auto* headR = new QHBoxLayout;
        headR->addWidget(label(QString("%1  %2").arg(sr.icon, sr.name), 12, false, TEXT_MAIN));
        headR->addStretch();
        headR->addWidget(label(QString("%1% (%2)").arg(sr.pct).arg(sr.count), 12, true, TEXT_MUTED));
        rLayout->addLayout(headR);
        rLayout->addWidget(progress(sr.pct, sr.color, 6));
        subLayout->addLayout(rLayout);
    }

    rightStatsCol->addWidget(subjectsCard);
    statsBody->addLayout(rightStatsCol, 1);

    l->addLayout(statsBody);
    l->addStretch();

    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// QUẢN LÝ GIA SƯ
// -------------------------------------------------------------
QWidget* MainWindow::createTutorPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* topCard = new QFrame(content);
    topCard->setStyleSheet("background:#F1F5F9; border:1px solid #E2E8F0; border-radius:10px;");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    tCol->addWidget(label(QString::fromUtf8("Quản lý gia sư"), 22, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Danh sách gia sư, chuyên môn và lịch nhận lớp."), 13, false, TEXT_MUTED));
    topLayout->addLayout(tCol, 1);

    auto* btnAddTutor = blueButton(QString::fromUtf8("👨‍🏫⁺  Thêm gia sư"), topCard);
    topLayout->addWidget(btnAddTutor);
    l->addWidget(topCard);

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
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    addFilter(fl, QString::fromUtf8("🔍  Tìm kiếm gia sư theo tên, chuyên môn..."), 280);
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

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// QUẢN LÝ LỚP HỌC
// -------------------------------------------------------------
QWidget* MainWindow::createClassPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* topCard = new QFrame(content);
    topCard->setStyleSheet("background:#F1F5F9; border:1px solid #E2E8F0; border-radius:10px;");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    tCol->addWidget(label(QString::fromUtf8("Quản lý lớp học"), 22, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Theo dõi lịch học, học sinh, gia sư và tiến độ lớp."), 13, false, TEXT_MUTED));
    topLayout->addLayout(tCol, 1);
    auto* btnCreateClass = button(QString::fromUtf8("📖⁺  Tạo lớp học"), false, topCard);
    topLayout->addWidget(btnCreateClass);
    l->addWidget(topCard);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTile(QString::fromUtf8("Tổng lớp"), "42", BLUE), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Đang học"), "31", GREEN), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Chờ xếp lịch"), "7", "#F59E0B"), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Cần gia sư"), "4", RED), 1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    addFilter(fl, QString::fromUtf8("🔍  Tìm mã lớp, học sinh, gia sư..."), 280);
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

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// QUẢN LÝ HỢP ĐỒNG
// -------------------------------------------------------------
QWidget* MainWindow::createContractPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* topCard = new QFrame(content);
    topCard->setStyleSheet("background:#F1F5F9; border:1px solid #E2E8F0; border-radius:10px;");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    tCol->addWidget(label(QString::fromUtf8("Quản lý hợp đồng"), 22, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Theo dõi toàn bộ hợp đồng giảng dạy và thanh toán."), 13, false, TEXT_MUTED));
    topLayout->addLayout(tCol, 1);
    auto* btnCreateContract = blueButton(QString::fromUtf8("📄⁺  Tạo hợp đồng"), topCard);
    topLayout->addWidget(btnCreateContract);
    l->addWidget(topCard);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTile(QString::fromUtf8("Tổng hợp đồng"), "18", BLUE), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Đang hiệu lực"), "12", GREEN), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Chờ xử lý"), "4", "#F59E0B"), 1);
    summary->addWidget(infoTile(QString::fromUtf8("Hoàn thành"), "2", "#8B5CF6"), 1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    addFilter(fl, QString::fromUtf8("🔍  Tìm mã hợp đồng, lớp, học sinh..."), 280);
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

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
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
