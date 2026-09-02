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
#include <QPushButton>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

namespace {

const QString NAVY = "#071426";
const QString NAVY_2 = "#0D1D33";
const QString BLUE = "#1677D2";
const QString BLUE_2 = "#3B9AF2";
const QString BG = "#F5F7FB";
const QString CARD = "#FFFFFF";
const QString BORDER = "#E5EAF0";
const QString TEXT = "#162033";
const QString MUTED = "#7A8494";
const QString GREEN = "#18A66A";
const QString GREEN_BG = "#E8F8F1";
const QString ORANGE = "#D88719";
const QString ORANGE_BG = "#FFF3DE";
const QString RED = "#D64A4A";
const QString RED_BG = "#FDECEC";

QLabel* label(const QString& text, int size = 11, bool bold = false,
              const QString& color = TEXT) {
    auto* l = new QLabel(text);
    l->setStyleSheet(QString("color:%1;font-size:%2px;%3")
                     .arg(color).arg(size).arg(bold ? "font-weight:700;" : ""));
    return l;
}

QFrame* card(QWidget* parent = nullptr) {
    auto* f = new QFrame(parent);
    f->setObjectName("card");
    f->setStyleSheet(
        "QFrame#card { background:#FFFFFF; border:1px solid #E5EAF0; "
        "border-radius:12px; }");
    return f;
}

QPushButton* button(const QString& text, bool primary = true, QWidget* parent = nullptr) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumHeight(34);
    b->setStyleSheet(primary
        ? "QPushButton { background:#1677D2; color:white; border:none; border-radius:7px; "
          "padding:0 15px; font-size:10px; font-weight:700; }"
          "QPushButton:hover { background:#0E66BA; }"
        : "QPushButton { background:white; color:#1677D2; border:1px solid #B9D7F3; "
          "border-radius:7px; padding:0 15px; font-size:10px; font-weight:600; }"
          "QPushButton:hover { background:#F0F7FE; }");
    return b;
}

QLabel* badge(const QString& text, const QString& bg, const QString& fg) {
    auto* b = new QLabel(text);
    b->setAlignment(Qt::AlignCenter);
    b->setStyleSheet(QString(
        "background:%1;color:%2;border-radius:10px;padding:4px 9px;"
        "font-size:9px;font-weight:700;").arg(bg, fg));
    b->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    return b;
}

QProgressBar* progress(int value, const QString& color = BLUE) {
    auto* p = new QProgressBar;
    p->setRange(0, 100);
    p->setValue(value);
    p->setTextVisible(false);
    p->setFixedHeight(7);
    p->setStyleSheet(QString(
        "QProgressBar { background:#E9EDF2; border:none; border-radius:4px; }"
        "QProgressBar::chunk { background:%1; border-radius:4px; }").arg(color));
    return p;
}

QWidget* avatar(const QString& initials, const QString& bg = "#E5F1FC") {
    auto* a = new QLabel(initials);
    a->setAlignment(Qt::AlignCenter);
    a->setFixedSize(32, 32);
    a->setStyleSheet(QString(
        "background:%1;color:#176DB8;border-radius:16px;font-size:9px;font-weight:800;")
        .arg(bg));
    return a;
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
        "QTableWidget { background:#FFFFFF; border:1px solid #E5EAF0; "
        "border-radius:10px; color:#273246; font-size:10px; }"
        "QTableWidget::item { border-bottom:1px solid #EEF1F4; padding:6px 8px; }"
        "QTableWidget::item:selected { background:#EDF6FF; color:#16314D; }"
        "QHeaderView::section { background:#F4F6F8; color:#687386; border:none; "
        "border-bottom:1px solid #E5EAF0; padding:9px 7px; font-size:8px; "
        "font-weight:800; }");
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(55);
    table->verticalHeader()->setDefaultSectionSize(45);
}

QWidget* header(QWidget* parent) {
    auto* h = new QWidget(parent);
    h->setFixedHeight(58);
    h->setStyleSheet("background:#FFFFFF;border-bottom:1px solid #E8ECF1;");
    auto* l = new QHBoxLayout(h);
    l->setContentsMargins(18, 10, 16, 10);
    l->setSpacing(10);

    auto* search = new QLineEdit(h);
    search->setPlaceholderText("⌕  Tìm kiếm hệ thống...");
    search->setFixedSize(285, 34);
    search->setStyleSheet(
        "QLineEdit { background:#F3F5F8; border:1px solid #EEF1F4; "
        "border-radius:8px; padding:0 12px; color:#344054; font-size:10px; }"
        "QLineEdit:focus { border:1px solid #B7D7F5; background:#FFFFFF; }");
    l->addWidget(search);
    l->addStretch();

    auto* notification = new QPushButton("●", h);
    notification->setFixedSize(28, 28);
    notification->setStyleSheet(
        "QPushButton { background:transparent;color:#1677D2;border:none;font-size:9px; }");
    l->addWidget(notification);

    auto* account = new QWidget(h);
    auto* al = new QHBoxLayout(account);
    al->setContentsMargins(8,0,0,0);
    al->setSpacing(8);
    auto* at = label("Admin User\nAdmin", 9, true);
    at->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    al->addWidget(at);
    auto* aa = new QLabel("A");
    aa->setAlignment(Qt::AlignCenter);
    aa->setFixedSize(30,30);
    aa->setStyleSheet("background:#101827;color:white;border-radius:15px;font-size:10px;font-weight:800;");
    al->addWidget(aa);
    l->addWidget(account);
    return h;
}

QWidget* titleBlock(const QString& eyebrow, const QString& titleText,
                    const QString& desc, QPushButton** action = nullptr,
                    const QString& actionText = QString()) {
    auto* row = new QWidget;
    auto* l = new QHBoxLayout(row);
    l->setContentsMargins(0,0,0,0);
    auto* texts = new QVBoxLayout;
    texts->setContentsMargins(0,0,0,0);
    texts->setSpacing(2);
    texts->addWidget(label(eyebrow, 8, true, "#6D7890"));
    texts->addWidget(label(titleText, 21, true, TEXT));
    auto* d = label(desc, 9, false, MUTED);
    d->setWordWrap(true);
    texts->addWidget(d);
    l->addLayout(texts, 1);
    if (action) {
        *action = button(actionText, true);
        l->addWidget(*action, 0, Qt::AlignTop);
    }
    return row;
}

QFrame* makeMetric(const QString& icon, const QString& titleText,
               const QString& value, const QString& note,
               const QString& accent = BLUE) {
    auto* f = card();
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(14,13,14,12);
    l->setSpacing(4);

    auto* top = new QHBoxLayout;
    top->setContentsMargins(0,0,0,0);
    top->addWidget(label(titleText, 9, false, MUTED));
    top->addStretch();
    auto* i = new QLabel(icon);
    i->setAlignment(Qt::AlignCenter);
    i->setFixedSize(30,30);
    i->setStyleSheet(QString("background:%1;color:%1;border-radius:9px;").arg(accent+"22"));
    i->setStyleSheet(QString("background:%1;color:%2;border-radius:9px;font-size:11px;font-weight:800;")
                     .arg(accent+"22", accent));
    top->addWidget(i);
    l->addLayout(top);
    l->addWidget(label(value, 20, true, TEXT));
    l->addWidget(label(note, 8, true, accent));
    return f;
}

void addFilter(QHBoxLayout* l, const QString& placeholder,
               const QStringList& values, int width = 190) {
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(38);
    edit->setMinimumWidth(width);
    edit->setStyleSheet(
        "QLineEdit { background:white;border:1px solid #DDE4EC;border-radius:8px;"
        "padding:0 11px;font-size:10px;color:#344054; }"
        "QLineEdit:focus { border:1px solid #86BCEB; }");
    l->addWidget(edit, 1);
    Q_UNUSED(values);
}

QComboBox* combo(const QStringList& items, int width = 150) {
    auto* c = new QComboBox;
    c->addItems(items);
    c->setFixedSize(width, 38);
    c->setStyleSheet(
        "QComboBox { background:white;border:1px solid #DDE4EC;border-radius:8px;"
        "padding:0 10px;font-size:10px;color:#344054; }"
        "QComboBox::drop-down { border:none;width:24px; }");
    return c;
}

QFrame* infoTile(const QString& titleText, const QString& value,
                 const QString& color = BLUE) {
    auto* f = card();
    f->setMinimumHeight(70);
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(12,10,12,9);
    l->setSpacing(3);
    l->addWidget(label(titleText, 8, true, MUTED));
    l->addWidget(label(value, 16, true, color));
    return f;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();

    setWindowTitle("Quản lý gia sư");
    resize(1360, 820);
    setMinimumSize(1100, 700);

    auto* central = new QWidget(this);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createSidebar(), 0);

    // Quan trọng: QStackedWidget là con trực tiếp của central để sidebar
    // luôn hiển thị đúng khi chuyển trang.
    stackedWidget = new QStackedWidget(central);
    stackedWidget->setObjectName("contentStack");
    stackedWidget->setStyleSheet("QStackedWidget#contentStack { background:#F5F7FB; border:none; }");

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
    for (std::size_t i = 0; i < students.size(); ++i) delete students[i];
    for (std::size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
}

QString MainWindow::joinStrings(const std::vector<std::string>& items) {
    QStringList parts;
    for (const auto& item : items) {
        if (!item.empty()) parts << QString::fromStdString(item);
    }
    return parts.join(", ");
}

QWidget* MainWindow::createSidebar() {
    auto* panel = new QWidget;
    panel->setFixedWidth(118);
    panel->setObjectName("sidebar");
    panel->setStyleSheet(
        "QWidget#sidebar { background:#071426; color:white; }");

    auto* l = new QVBoxLayout(panel);
    l->setContentsMargins(8,12,8,10);
    l->setSpacing(4);

    auto* brand = new QFrame(panel);
    brand->setStyleSheet("background:#0D1D33;border:1px solid #19304B;border-radius:10px;");
    auto* bl = new QVBoxLayout(brand);
    bl->setContentsMargins(7,9,7,9);
    bl->setSpacing(2);
    auto* logo = new QLabel("TF");
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(28,28);
    logo->setStyleSheet("background:#1677D2;color:white;border-radius:8px;font-size:10px;font-weight:900;");
    bl->addWidget(logo,0,Qt::AlignCenter);
    auto* bt = label("TUTOR",8,true,"#FFFFFF");
    bt->setAlignment(Qt::AlignCenter);
    auto* bs = label("FLOW",8,true,"#5AA8EA");
    bs->setAlignment(Qt::AlignCenter);
    bl->addWidget(bt); bl->addWidget(bs);
    l->addWidget(brand);
    l->addSpacing(8);

    auto* group = new QButtonGroup(panel);
    group->setExclusive(true);

    const QStringList names = {
        "⌂\nTổng quan", "♙\nHọc sinh", "♟\nGia sư", "⌕\nTìm gia sư",
        "▣\nLớp học", "▤\nHợp đồng", "▥\nThống kê"
    };

    for (int i=0; i<names.size(); ++i) {
        auto* b = new QPushButton(names[i], panel);
        b->setCheckable(true);
        b->setFixedHeight(48);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton { background:transparent;color:#AEBBCA;border:none;"
            "border-radius:8px;font-size:8px;font-weight:600;padding:3px 2px; }"
            "QPushButton:hover { background:#10243B;color:white; }"
            "QPushButton:checked { background:#1677D2;color:white;font-weight:800; }");
        if (i==0) b->setChecked(true);
        group->addButton(b,i);
        connect(b,&QPushButton::clicked,this,[this,i](){ switchPage(i); });
        l->addWidget(b);
    }

    l->addStretch();

    auto* profile = new QFrame(panel);
    profile->setStyleSheet("background:#0D1D33;border:1px solid #19304B;border-radius:8px;");
    auto* pl = new QVBoxLayout(profile);
    pl->setContentsMargins(5,7,5,7);
    auto* pa = new QLabel("A");
    pa->setAlignment(Qt::AlignCenter);
    pa->setFixedSize(25,25);
    pa->setStyleSheet("background:#1677D2;color:white;border-radius:13px;font-size:9px;font-weight:800;");
    pl->addWidget(pa,0,Qt::AlignCenter);
    auto* pn = label("Admin",8,true,"#FFFFFF");
    pn->setAlignment(Qt::AlignCenter);
    pl->addWidget(pn);
    auto* pr = label("Quản trị",7,false,"#8392A7");
    pr->setAlignment(Qt::AlignCenter);
    pl->addWidget(pr);
    l->addWidget(profile);

    auto* logout = new QPushButton("↪", panel);
    logout->setToolTip("Đăng xuất");
    logout->setFixedHeight(30);
    logout->setStyleSheet(
        "QPushButton { background:#10243B;color:#D9E2EC;border:none;border-radius:7px;font-size:12px; }"
        "QPushButton:hover { background:#173450; }");
    l->addWidget(logout);
    return panel;
}

QWidget* MainWindow::createDashboardPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    root->addWidget(header(page));

    auto* content = new QWidget;
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    QPushButton* unused = nullptr;
    l->addWidget(titleBlock("DASHBOARD", "Tổng quan hệ thống",
                            "Theo dõi nhanh học sinh, gia sư, lớp học và hiệu quả ghép nối.", &unused,
                            "Báo cáo mới"));

    auto* actions = new QHBoxLayout;
    actions->setSpacing(7);
    auto* a1 = button("＋ Thêm học sinh", true);
    auto* a2 = button("＋ Thêm gia sư", true);
    auto* a3 = button("⌕ Tìm gia sư", false);
    auto* a4 = button("▣ Tạo lớp học", false);
    actions->addStretch(); actions->addWidget(a1); actions->addWidget(a2); actions->addWidget(a3); actions->addWidget(a4);
    l->addLayout(actions);

    connect(a1,&QPushButton::clicked,this,[this](){switchPage(1);});
    connect(a2,&QPushButton::clicked,this,[this](){switchPage(2);});
    connect(a3,&QPushButton::clicked,this,[this](){switchPage(3);});
    connect(a4,&QPushButton::clicked,this,[this](){switchPage(4);});

    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(10);
    metrics->addWidget(makeMetric("HS","Tổng học sinh","125","+12% so với tháng trước"),1);
    metrics->addWidget(makeMetric("GS","Tổng gia sư","68","+5% so với tháng trước","#2C8BE0"),1);
    metrics->addWidget(makeMetric("LH","Lớp đang hoạt động","42","-2% so với tháng trước",ORANGE),1);
    metrics->addWidget(makeMetric("%","Tỷ lệ ghép thành công","78%","Đang tăng tốt",GREEN),1);
    l->addLayout(metrics);

    auto* body = new QHBoxLayout;
    body->setSpacing(10);

    auto* status = card();
    auto* sl = new QVBoxLayout(status);
    sl->setContentsMargins(14,13,14,13);
    sl->setSpacing(9);
    sl->addWidget(label("Trạng thái hôm nay",13,true));
    sl->addWidget(label("HỌC SINH",8,true,MUTED));
    sl->addWidget(label("Đã ghép                         98",10,false,TEXT));
    sl->addWidget(progress(78));
    sl->addWidget(label("Chưa ghép                       27",10,false,TEXT));
    sl->addWidget(progress(22,"#9AA4B2"));
    sl->addSpacing(5);
    sl->addWidget(label("GIA SƯ",8,true,MUTED));
    sl->addWidget(label("Sẵn sàng                        45",10,false,TEXT));
    sl->addWidget(label("Đang bận                        18",10,false,TEXT));
    sl->addWidget(label("Không hoạt động                  5",10,false,TEXT));
    body->addWidget(status,0);

    auto* recent = card();
    auto* rl = new QVBoxLayout(recent);
    rl->setContentsMargins(14,12,14,12);
    auto* rh = new QHBoxLayout;
    rh->addWidget(label("Lớp học gần đây",13,true));
    rh->addStretch();
    auto* see = button("XEM TẤT CẢ",false);
    see->setFixedHeight(28);
    rh->addWidget(see);
    rl->addLayout(rh);

    auto* t = new QTableWidget(4,6);
    t->setHorizontalHeaderLabels({"MÃ LỚP","HỌC SINH","GIA SƯ","MÔN","TRẠNG THÁI","THAO TÁC"});
    styleTable(t);
    const QString rows[4][6] = {
        {"LH-1042","Nguyễn Minh Anh","Trần Thị B","Toán 10","Đang học","Xem"},
        {"LH-1043","Lê Hoàng M","Phạm Văn C","Tiếng Anh","Chờ xử lý","Xem"},
        {"LH-1044","Vũ Minh T","Chưa có","Vật lý","Cần gia sư","Xem"},
        {"LH-1045","Hoàng Tú K","Ngô Phương T","Hóa 9","Đang học","Xem"}
    };
    for(int i=0;i<4;++i) for(int j=0;j<6;++j) t->setItem(i,j,new QTableWidgetItem(rows[i][j]));
    rl->addWidget(t,1);
    body->addWidget(recent,1);
    l->addLayout(body,1);

    auto* footer = card();
    auto* fl = new QHBoxLayout(footer);
    fl->setContentsMargins(12,9,12,9);
    fl->addWidget(label("💡  Gợi ý hệ thống",9,true,BLUE));
    fl->addWidget(label("Có 27 học sinh chưa được ghép. Hãy chạy Tìm Gia Sư Thông Minh để tối ưu tỷ lệ ghép.",9,false,MUTED),1);
    l->addWidget(footer);

    root->addWidget(content,1);
    return page;
}

QWidget* MainWindow::createStudentPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    QPushButton* add = nullptr;
    l->addWidget(titleBlock("QUẢN LÝ", "Quản lý học sinh",
                            "Danh sách học sinh và nhu cầu học tập.", &add,
                            "＋ Thêm học sinh"));

    auto* summary = new QHBoxLayout;
    summary->setSpacing(9);
    summary->addWidget(infoTile("Tổng học sinh",QString::number(static_cast<int>(students.size()) > 0 ? students.size() : 125)),1);
    summary->addWidget(infoTile("Đã ghép","98",GREEN),1);
    summary->addWidget(infoTile("Chưa ghép","27",ORANGE),1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(10,10,10,10);
    fl->setSpacing(8);
    addFilter(fl,"⌕  Tìm kiếm theo mã, tên học sinh...",{},300);
    fl->addWidget(combo({"Tất cả khối lớp","Lớp 10","Lớp 11","Lớp 12"},145));
    fl->addWidget(combo({"Tất cả trạng thái","Đã ghép","Chưa ghép"},145));
    fl->addStretch();
    auto* reset = button("Đặt lại",false);
    reset->setFixedWidth(82);
    fl->addWidget(reset);
    l->addWidget(filters);

    auto* table = new QTableWidget(0,7);
    table->setHorizontalHeaderLabels({"MÃ HS","HỌC SINH","LỚP & MÔN","LỊCH RẢNH","NGÂN SÁCH","TRẠNG THÁI","THAO TÁC"});
    styleTable(table);
    populateStudentTable(table);
    for(int i=0;i<table->rowCount();++i) {
        auto* st = table->item(i,5);
        if(st) st->setTextAlignment(Qt::AlignCenter);
        auto* action = table->item(i,6);
        if(action) action->setForeground(QColor(BLUE));
    }
    l->addWidget(table,1);

    auto* foot = new QHBoxLayout;
    foot->addWidget(label(QString("Hiển thị %1 học sinh").arg(table->rowCount()),9,false,MUTED));
    foot->addStretch();
    foot->addWidget(label("‹   1   2   3   ›",9,true,BLUE));
    l->addLayout(foot);

    root->addWidget(content,1);
    return page;
}

QWidget* MainWindow::createTutorPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    QPushButton* add = nullptr;
    l->addWidget(titleBlock("QUẢN LÝ", "Quản lý gia sư",
                            "Danh sách gia sư, chuyên môn và lịch nhận lớp.", &add,
                            "＋ Thêm gia sư"));

    auto* summary = new QHBoxLayout;
    summary->setSpacing(9);
    summary->addWidget(infoTile("Tổng gia sư",QString::number(static_cast<int>(tutors.size()) > 0 ? tutors.size() : 68)),1);
    summary->addWidget(infoTile("Sẵn sàng","45",GREEN),1);
    summary->addWidget(infoTile("Đang bận","18",ORANGE),1);
    summary->addWidget(infoTile("Mức phí TB","180k",BLUE),1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(10,10,10,10);
    fl->setSpacing(8);
    addFilter(fl,"⌕  Tìm kiếm gia sư...",{},250);
    fl->addWidget(combo({"Tất cả môn","Toán","Lý","Hóa","Tiếng Anh"},135));
    fl->addWidget(combo({"Tất cả khu vực","Hải Châu","Thanh Khê","Liên Chiểu"},155));
    fl->addWidget(combo({"Tất cả trạng thái","Sẵn sàng","Bận"},135));
    l->addWidget(filters);

    auto* table = new QTableWidget(0,9);
    table->setHorizontalHeaderLabels({"MÃ GS","GIA SƯ","MÔN DẠY","KINH NGHIỆM","HỌC PHÍ","LỊCH RẢNH","KHU VỰC","TRẠNG THÁI","THAO TÁC"});
    styleTable(table);
    populateTutorTable(table);
    for(int i=0;i<table->rowCount();++i) {
        auto* action=table->item(i,8);
        if(action) action->setForeground(QColor(BLUE));
    }
    l->addWidget(table,1);
    auto* foot = new QHBoxLayout;
    foot->addWidget(label(QString("Hiển thị %1 gia sư").arg(table->rowCount()),9,false,MUTED));
    foot->addStretch();
    foot->addWidget(label("‹   1   2   3   ›",9,true,BLUE));
    l->addLayout(foot);

    root->addWidget(content,1);
    return page;
}

QWidget* MainWindow::createMatchingPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    l->addWidget(titleBlock("SMART MATCHING","Tìm Gia Sư Thông Minh",
                            "Hệ thống tự động phân tích yêu cầu học sinh và đề xuất gia sư phù hợp nhất."));

    auto* chooseRow = new QHBoxLayout;
    chooseRow->setSpacing(10);

    auto* studentCard = card();
    auto* sc = new QVBoxLayout(studentCard);
    sc->setContentsMargins(14,12,14,12);
    auto* sh = new QHBoxLayout;
    sh->addWidget(label("BƯỚC 1",8,true,BLUE));
    sh->addStretch();
    sh->addWidget(badge("SV001","#E8F3FC",BLUE));
    sc->addLayout(sh);
    sc->addWidget(label("Chọn học sinh",10,true,MUTED));
    auto* studentCombo = combo({"SV001 - Nguyễn Minh Anh","SV002 - Trần Văn Bảo","SV003 - Lê Thị Lan"},260);
    sc->addWidget(studentCombo);
    chooseRow->addWidget(studentCard,1);

    auto* request = card();
    auto* rc = new QVBoxLayout(request);
    rc->setContentsMargins(14,12,14,12);
    auto* rh = new QHBoxLayout;
    rh->addWidget(label("YÊU CẦU HỌC SINH",8,true,BLUE));
    rh->addStretch();
    rh->addWidget(badge("Đã tải","#E8F8F1",GREEN));
    rc->addLayout(rh);
    auto* reqGrid = new QGridLayout;
    reqGrid->setHorizontalSpacing(22);
    reqGrid->setVerticalSpacing(5);
    reqGrid->addWidget(label("Môn học",8,false,MUTED),0,0);
    reqGrid->addWidget(label("Toán",10,true),1,0);
    reqGrid->addWidget(label("Khu vực",8,false,MUTED),0,1);
    reqGrid->addWidget(label("Hải Châu",10,true),1,1);
    reqGrid->addWidget(label("Ngân sách",8,false,MUTED),0,2);
    reqGrid->addWidget(label("200.000đ/buổi",10,true,BLUE),1,2);
    reqGrid->addWidget(label("Lịch học",8,false,MUTED),0,3);
    reqGrid->addWidget(label("T2, T4, T6",10,true),1,3);
    rc->addLayout(reqGrid);
    chooseRow->addWidget(request,2);
    l->addLayout(chooseRow);

    auto* find = button("✦  TÌM GIA SƯ PHÙ HỢP",true);
    find->setFixedSize(210,38);
    l->addWidget(find,0,Qt::AlignCenter);

    auto* result = new QHBoxLayout;
    result->setSpacing(10);

    auto* candidate = new QFrame;
    candidate->setStyleSheet("QFrame { background:#071426;border-radius:14px; }");
    candidate->setMinimumWidth(270);
    auto* cl = new QVBoxLayout(candidate);
    cl->setContentsMargins(18,16,18,16);
    cl->setSpacing(7);
    auto* top = new QHBoxLayout;
    top->addWidget(badge("TOP 1","#F7C948","#182033"));
    top->addStretch();
    top->addWidget(label("95%",19,true,"#5DB2FF"));
    cl->addLayout(top);
    cl->addWidget(avatar("TQH","#DDEEFF"),0,Qt::AlignCenter);
    auto* cn = label("Trần Quốc Huy",18,true,"#FFFFFF");
    cn->setAlignment(Qt::AlignCenter);
    cl->addWidget(cn);
    auto* cc = label("GS003  •  Toán, Lý  •  8 năm kinh nghiệm",8,false,"#9FB0C4");
    cc->setAlignment(Qt::AlignCenter);
    cl->addWidget(cc);
    cl->addSpacing(6);
    cl->addWidget(label("ĐỘ PHÙ HỢP",8,true,"#8FA2B8"));
    cl->addWidget(progress(95,"#2C94E9"));
    auto* perfect = label("95 / 100 điểm",9,true,"#5DB2FF");
    perfect->setAlignment(Qt::AlignRight);
    cl->addWidget(perfect);
    result->addWidget(candidate,1);

    auto* detail = card();
    auto* dl = new QVBoxLayout(detail);
    dl->setContentsMargins(15,14,15,14);
    dl->setSpacing(9);
    dl->addWidget(label("Chi tiết matching",14,true));
    auto* chips = new QHBoxLayout;
    chips->addWidget(infoTile("Kinh nghiệm","25/25",BLUE));
    chips->addWidget(infoTile("Khu vực","10/10",GREEN));
    chips->addWidget(infoTile("Học phí","40/45",ORANGE));
    chips->addWidget(infoTile("Lịch học","20/20",BLUE));
    dl->addLayout(chips);

    dl->addWidget(label("VÌ SAO GIA SƯ NÀY PHÙ HỢP?",10,true,BLUE));
    const QStringList reasons = {
        "✓ Kinh nghiệm chuyên môn đáp ứng tốt yêu cầu môn Toán.",
        "✓ Khu vực Hải Châu phù hợp với địa điểm học.",
        "✓ Học phí nằm trong ngân sách của học sinh.",
        "✓ Lịch T2, T4, T6 trùng khớp hoàn toàn."
    };
    for(const auto& r: reasons) dl->addWidget(label(r,9,false,MUTED));
    dl->addStretch();

    auto* actions = new QHBoxLayout;
    auto* detailBtn = button("Xem chi tiết",false);
    auto* chooseBtn = button("♥  Chọn gia sư này",true);
    actions->addStretch(); actions->addWidget(detailBtn); actions->addWidget(chooseBtn);
    dl->addLayout(actions);
    result->addWidget(detail,2);
    l->addLayout(result,1);

    root->addWidget(content,1);
    return page;
}

QWidget* MainWindow::createClassPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    QPushButton* add=nullptr;
    l->addWidget(titleBlock("QUẢN LÝ","Quản lý lớp học",
                            "Theo dõi lịch học, học sinh, gia sư và tiến độ lớp.",&add,
                            "＋ Tạo lớp học"));

    auto* summary = new QHBoxLayout;
    summary->setSpacing(9);
    summary->addWidget(infoTile("Tổng lớp","42",BLUE),1);
    summary->addWidget(infoTile("Đang học","31",GREEN),1);
    summary->addWidget(infoTile("Chờ xếp lịch","7",ORANGE),1);
    summary->addWidget(infoTile("Cần gia sư","4",RED),1);
    l->addLayout(summary);

    auto* filters=card();
    auto* fl=new QHBoxLayout(filters);
    fl->setContentsMargins(10,10,10,10);
    fl->setSpacing(8);
    addFilter(fl,"⌕  Tìm mã lớp, học sinh...",{},260);
    fl->addWidget(combo({"Tất cả môn","Toán","Lý","Hóa","Văn"},130));
    fl->addWidget(combo({"Tất cả trạng thái","Đang học","Chờ xử lý","Cần gia sư"},150));
    l->addWidget(filters);

    auto* table=new QTableWidget(4,10);
    table->setHorizontalHeaderLabels({"MÃ LỚP","HỌC SINH","GIA SƯ","MÔN","LỊCH","BẮT ĐẦU","KẾT THÚC","HỌC PHÍ","TRẠNG THÁI","THAO TÁC"});
    styleTable(table);
    const QString rows[4][10]={
        {"CL001","Nguyễn Minh Anh","Trần Quốc Huy","Toán","T2,T4,T6","01/09/26","30/06/27","200k","Đang học","Xem"},
        {"CL002","Trần Văn Bảo","Nguyễn Văn B","Lý","T3,T5","10/09/26","31/12/26","250k","Chờ xử lý","Xem"},
        {"CL003","Lê Thị Lan","Lê Minh C","Văn","T2,T5,T7","01/10/26","30/06/27","300k","Hoàn thành","Xem"},
        {"CL004","Phạm Quốc Huy","Phạm Văn D","Hóa","T4,T6","15/09/26","15/01/27","180k","Cần gia sư","Xem"}
    };
    for(int i=0;i<4;++i)for(int j=0;j<10;++j)table->setItem(i,j,new QTableWidgetItem(rows[i][j]));
    l->addWidget(table,1);
    root->addWidget(content,1);
    return page;
}

QWidget* MainWindow::createContractPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content = new QWidget;
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    QPushButton* add=nullptr;
    l->addWidget(titleBlock("QUẢN LÝ","Quản lý hợp đồng",
                            "Theo dõi toàn bộ hợp đồng của trung tâm.",&add,
                            "＋ Tạo hợp đồng"));

    auto* summary=new QHBoxLayout;
    summary->setSpacing(9);
    summary->addWidget(infoTile("Tổng hợp đồng","18",BLUE),1);
    summary->addWidget(infoTile("Đang hiệu lực","12",GREEN),1);
    summary->addWidget(infoTile("Chờ xử lý","4",ORANGE),1);
    summary->addWidget(infoTile("Hoàn thành","2","#7C5CDB"),1);
    l->addLayout(summary);

    auto* filters=card();
    auto* fl=new QHBoxLayout(filters);
    fl->setContentsMargins(10,10,10,10);
    fl->setSpacing(8);
    addFilter(fl,"⌕  Tìm mã hợp đồng, lớp, học sinh...",{},300);
    fl->addWidget(combo({"Tất cả trạng thái","Đang hiệu lực","Chờ xử lý","Hoàn thành"},170));
    fl->addWidget(combo({"Tất cả thời gian","Tháng này","Quý này"},145));
    l->addWidget(filters);

    auto* table=new QTableWidget(4,9);
    table->setHorizontalHeaderLabels({"MÃ HĐ","MÃ LỚP","HỌC SINH","GIA SƯ","BẮT ĐẦU","KẾT THÚC","TỔNG PHÍ","TRẠNG THÁI","THAO TÁC"});
    styleTable(table);
    const QString rows[4][9]={
        {"HD001","CL001","Nguyễn Minh Anh","Trần Quốc Huy","01/09/26","30/06/27","200.000đ","Đang hiệu lực","Xem"},
        {"HD002","CL002","Trần Văn Bảo","Nguyễn Văn B","10/09/26","31/12/26","250.000đ","Chờ xử lý","Xem"},
        {"HD003","CL003","Lê Thị Lan","Lê Minh C","01/10/26","30/06/27","300.000đ","Hoàn thành","Xem"},
        {"HD004","CL004","Phạm Quốc Huy","Phạm Văn D","15/09/26","15/01/27","180.000đ","Đang hiệu lực","Xem"}
    };
    for(int i=0;i<4;++i)for(int j=0;j<9;++j)table->setItem(i,j,new QTableWidgetItem(rows[i][j]));
    l->addWidget(table,1);

    auto* notice=card();
    auto* nl=new QHBoxLayout(notice);
    nl->setContentsMargins(12,9,12,9);
    nl->addWidget(label("▣  Lưu ý",9,true,ORANGE));
    nl->addWidget(label("4 hợp đồng đang chờ xử lý. Kiểm tra thời gian bắt đầu trước khi xác nhận.",9,false,MUTED),1);
    l->addWidget(notice);

    root->addWidget(content,1);
    return page;
}

QWidget* MainWindow::createStatisticsPage() {
    auto* page=new QWidget;
    auto* root=new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->addWidget(header(page));

    auto* content=new QWidget;
    auto* l=new QVBoxLayout(content);
    l->setContentsMargins(18,16,18,16);
    l->setSpacing(12);

    auto* titleRow=new QHBoxLayout;
    auto* texts=new QVBoxLayout;
    texts->setContentsMargins(0,0,0,0);
    texts->addWidget(label("BÁO CÁO VẬN HÀNH",8,true,"#6D7890"));
    texts->addWidget(label("Thống kê hệ thống",21,true,TEXT));
    texts->addWidget(label("Phân tích hiệu suất hoạt động của trung tâm.",9,false,MUTED));
    titleRow->addLayout(texts,1);
    titleRow->addWidget(combo({"30 ngày qua","7 ngày qua","Quý này"},125));
    titleRow->addWidget(button("⇩  Xuất báo cáo",true));
    l->addLayout(titleRow);

    auto* metrics=new QHBoxLayout;
    metrics->setSpacing(9);
    metrics->addWidget(makeMetric("HS","Tổng học sinh","2,543","+12%","#2C8BE0"),1);
    metrics->addWidget(makeMetric("GS","Tổng gia sư","842","+5%",BLUE),1);
    metrics->addWidget(makeMetric("●","Sẵn sàng nhận lớp","315","LIVE",GREEN),1);
    metrics->addWidget(makeMetric("LH","Lớp hoạt động","1,120","+18%","#7C5CDB"),1);
    metrics->addWidget(makeMetric("%","Tỷ lệ ghép","86%","Cao",ORANGE),1);
    l->addLayout(metrics);

    auto* body=new QHBoxLayout;
    body->setSpacing(10);

    auto* match=card();
    auto* ml=new QVBoxLayout(match);
    ml->setContentsMargins(15,14,15,14);
    ml->addWidget(label("Tình trạng ghép nối",13,true));
    auto* circle=new QLabel("86%\nĐÃ GHÉP NỐI");
    circle->setAlignment(Qt::AlignCenter);
    circle->setFixedHeight(150);
    circle->setStyleSheet(
        "background:#EEF5FC;color:#0D6DBB;border:16px solid #1677D2;"
        "border-radius:75px;font-size:18px;font-weight:800;");
    ml->addWidget(circle,0,Qt::AlignCenter);
    ml->addWidget(label("Lớp đã có gia sư                         1,420",9,false,MUTED));
    ml->addWidget(progress(86,BLUE));
    ml->addWidget(label("Đang chờ ghép                            231",9,false,MUTED));
    ml->addWidget(progress(14,"#AEB7C3"));
    body->addWidget(match,0);

    auto* chart=card();
    auto* cl=new QVBoxLayout(chart);
    cl->setContentsMargins(15,14,15,14);
    auto* ch=new QHBoxLayout;
    ch->addWidget(label("Phân bổ gia sư theo môn học",13,true));
    ch->addStretch();
    ch->addWidget(label("Xem chi tiết →",8,true,BLUE));
    cl->addLayout(ch);

    const QString subjects[5]={"Toán Học","Tiếng Anh","Vật Lý","Hóa Học","Ngữ Văn"};
    const int vals[5]={32,28,18,14,8};
    for(int i=0;i<5;++i){
        auto* row=new QHBoxLayout;
        row->addWidget(label(subjects[i],9,false,TEXT));
        row->addStretch();
        row->addWidget(label(QString("%1%").arg(vals[i]),9,true,BLUE));
        cl->addLayout(row);
        cl->addWidget(progress(vals[i], i==0?NAVY:(i==1?BLUE:(i==2?"#52708D":"#7FB8E9"))));
    }
    cl->addSpacing(8);
    cl->addWidget(label("Trạng thái lớp học",13,true));
    const QString stages[4]={"CHỜ BẮT ĐẦU","ĐANG HỌC","HOÀN THÀNH","HỦY"};
    const int stageVals[4]={46,72,64,18};
    auto* stagesRow=new QHBoxLayout;
    for(int i=0;i<4;++i){
        auto* col=new QVBoxLayout;
        col->addWidget(progress(stageVals[i],i==1?NAVY:BLUE));
        auto* s=label(stages[i],7,true,MUTED); s->setAlignment(Qt::AlignCenter);
        col->addWidget(s);
        stagesRow->addLayout(col,1);
    }
    cl->addLayout(stagesRow);
    body->addWidget(chart,1);
    l->addLayout(body,1);

    root->addWidget(content,1);
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
