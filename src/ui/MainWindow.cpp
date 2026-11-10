#include "MainWindow.h"
#include "LoginWindow.h"
#include "IdGenerator.h"
#include "ClassService.h"
#include "ContractService.h"
#include "ContractPage.h"
#include "TextUtils.h"
#include "Validator.h"
#include "Algorithms.h"
#include "ScheduleUtils.h"

#include <QAbstractItemView>
#include <QButtonGroup>
#include <QComboBox>
#include <QColor>
#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QStyle>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPaintEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QVector>
#include <QMap>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QPolygonF>
#include <QFontMetrics>

#include <algorithm>
#include <cmath>
#include <QResizeEvent>
#include <functional>
#include <utility>
#include <QTime>
#include <QTimer>

using namespace std;

namespace {

// ---------------------------------------------------------------------------
// Kiểm tra dữ liệu nhập: mỗi hàm trả về thông báo lỗi (rỗng nếu hợp lệ).
// Ô nhập sai được tô đỏ, con trỏ nhảy vào ô sai đầu tiên để người dùng nhập lại.
// ---------------------------------------------------------------------------
void markInvalid(QLineEdit* e, bool bad) {
    if (!e || e->property("invalid").toBool() == bad) return;
    e->setProperty("invalid", bad);
    e->style()->unpolish(e);
    e->style()->polish(e);
    e->update();
}

QString idError(const QString& id, const QString& prefix, const QString& what,
                const std::function<bool(const std::string&)>& exists) {
    if (id.isEmpty()) return what + QString::fromUtf8(" không được để trống.");
    if (!QRegularExpression("^" + prefix + "\\d{3,}$").match(id).hasMatch()) {
        return what + QString::fromUtf8(" phải có dạng %2 (%1 cộng ít nhất 3 chữ số).").arg(prefix, prefix + "001");
    }
    if (exists(id.toStdString())) {
        return what + " " + id + QString::fromUtf8(" đã tồn tại, vui lòng nhập mã khác.");
    }
    return QString();
}

QString requiredError(const QString& v, const QString& what) {
    return v.isEmpty() ? what + QString::fromUtf8(" không được để trống.") : QString();
}

QString nameError(const QString& n) {
    if (n.isEmpty()) return QString::fromUtf8("Họ và tên không được để trống.");
    if (n.contains(QRegularExpression("[0-9|]"))) {
        return QString::fromUtf8("Họ và tên không được chứa chữ số hoặc ký tự '|'.");
    }
    return QString();
}

QString phoneError(const QString& p) {
    if (p.isEmpty()) return QString::fromUtf8("Số điện thoại không được để trống.");
    if (!Validator::isValidPhone(p.toStdString())) {
        return QString::fromUtf8("Số điện thoại không hợp lệ (9-11 chữ số, bắt đầu bằng 0, ví dụ 0905123456).");
    }
    return QString();
}

QString dateError(const QString& d, const QString& what) {
    if (d.isEmpty()) return what + QString::fromUtf8(" không được để trống.");
    if (!Validator::isValidDate(d.toStdString())) {
        return what + QString::fromUtf8(" không hợp lệ (định dạng dd/MM/yyyy, ví dụ 25/12/2008).");
    }
    return QString();
}

QString dobError(const QString& d, int minAge, int maxAge, const QString& who) {
    QString e = dateError(d, QString::fromUtf8("Ngày sinh"));
    if (!e.isEmpty()) return e;
    QDate dt = QDate::fromString(d, "dd/MM/yyyy");
    QDate today = QDate::currentDate();
    if (dt > today) return QString::fromUtf8("Ngày sinh không được ở tương lai.");
    int age = today.year() - dt.year();
    if (today.month() < dt.month() || (today.month() == dt.month() && today.day() < dt.day())) --age;
    if (age < minAge || age > maxAge) {
        return QString::fromUtf8("Tuổi của %1 phải từ %2 đến %3 (ngày sinh nhập vào cho %4 tuổi).")
            .arg(who).arg(minAge).arg(maxAge).arg(age);
    }
    return QString();
}

QString subjectInputError(const QString& text, const QString& label) {
    const auto list = FileManager::splitString(text.toStdString(), ',');
    if (list.empty()) return label + QString::fromUtf8(" phải có ít nhất một môn hợp lệ.");
    // Reject blank items instead of silently discarding ",," or a trailing comma.
    const QStringList parts = text.split(',');
    for (const QString& part : parts)
        if (part.trimmed().isEmpty())
            return QString::fromUtf8("Danh sách môn không được có mục rỗng hoặc dấu phẩy thừa.");
    if (text.contains('|') || text.contains('\n') || text.contains('\r'))
        return QString::fromUtf8("Tên môn không được chứa ký tự | hoặc xuống dòng.");
    return {};
}

QString scheduleInputError(const QString& text) {
    return ScheduleUtils::valid(text.toStdString()) ? QString() :
        QString::fromUtf8("Lịch chỉ gồm T2, T3, T4, T5, T6, T7, CN (phân tách bằng dấu phẩy).");
}

QString moneyError(const QString& m, const QString& what) {
    if (!Validator::isNonNegativeNumber(m.toStdString())) {
        return what + QString::fromUtf8(" phải là số nguyên không âm (ví dụ 200000).");
    }
    return QString();
}

// Gom lỗi của cả form: tô đỏ mọi ô sai, đưa con trỏ vào ô sai đầu tiên, báo tất cả lỗi một lần.
class FormCheck {
    QVector<QLineEdit*> bad_;
public:
    void check(QLineEdit* f, const QString& err) {
        markInvalid(f, !err.isEmpty());
        // Each field owns its error label; place it immediately beneath the input.
        auto* error = qobject_cast<QLabel*>(f->property("validationLabel").value<QObject*>());
        if (!error) {
            error = new QLabel(f->parentWidget());
            error->setObjectName("inlineValidationError");
            error->setStyleSheet("color:#dc324f;font-size:11px;background:transparent;");
            error->setWordWrap(true);
            if (auto* form = qobject_cast<QFormLayout*>(f->parentWidget()->layout())) {
                // Position the error in its own row, immediately after the corresponding field row.
                int row = -1; QFormLayout::ItemRole role;
                form->getWidgetPosition(f, &row, &role);
                if (row >= 0) form->insertRow(row + 1, error);
            }
            error->setProperty("field", QVariant::fromValue(static_cast<QObject*>(f)));
            error->setParent(f->parentWidget());
            f->setProperty("validationLabel", QVariant::fromValue(static_cast<QObject*>(error)));
        }
        error->setText(err); error->setVisible(!err.isEmpty());
        if (!err.isEmpty()) bad_.push_back(f);
    }
    bool report(QWidget*) {
        if (bad_.isEmpty()) return true;
        bad_[0]->setFocus(); bad_[0]->selectAll(); return false;
    }
};

// Kiểm tra ngay khi gõ: ô chuyển đỏ nếu nội dung sai định dạng.
void attachLive(QLineEdit* e, std::function<QString(const QString&)> fn) {
    QObject::connect(e, &QLineEdit::textChanged, e, [e, fn](const QString& t) {
        const QString v = t.trimmed();
        const QString error = fn(v);
        markInvalid(e, !error.isEmpty());
        // Keep the message local to its input, without interrupting the form.
        auto* label = qobject_cast<QLabel*>(e->property("validationLabel").value<QObject*>());
        if (!label) {
            if (auto* form = qobject_cast<QFormLayout*>(e->parentWidget()->layout())) {
                label = new QLabel(e->parentWidget());
                label->setStyleSheet("color:#dc324f;font-size:11px;background:transparent;");
                label->setWordWrap(true);
                int row = -1; QFormLayout::ItemRole role;
                form->getWidgetPosition(e, &row, &role);
                if (row >= 0) form->insertRow(row + 1, label);
                e->setProperty("validationLabel", QVariant::fromValue(static_cast<QObject*>(label)));
            }
        }
        if (label) { label->setText(error); label->setVisible(!error.isEmpty()); }
    });
}

// Ngày sinh: chỉ hiển thị ví dụ, tuyệt đối không điền sẵn ngày mẫu.
// Regex chặn sai cấu trúc ngay lúc gõ; dobError kiểm tra ngày thực, năm nhuận và tuổi khi lưu.
QLineEdit* birthDateField(QWidget* parent, const QString& existing = QString()) {
    auto* edit = new QLineEdit(parent);
    edit->setObjectName("birthDateInput");
    edit->setPlaceholderText(QString::fromUtf8("Vd: 25/12/2008 (dd/MM/yyyy)"));
    edit->setToolTip(QString::fromUtf8("Bắt buộc nhập đúng dd/MM/yyyy, ví dụ 25/12/2008."));
    edit->setMaxLength(10);
    edit->setValidator(new QRegularExpressionValidator(
        QRegularExpression("(?:0[1-9]|[12][0-9]|3[01])/(?:0[1-9]|1[0-2])/[0-9]{4}"), edit));
    if (!existing.isEmpty()) edit->setText(existing);
    return edit;
}

}  // namespace

namespace {

// Color palette matching the modern dashboard theme
// Phong cách TutorHub: sidebar xanh đen, điểm nhấn gradient xanh pastel (teal -> xanh da trời).
const QString SIDEBAR_BG = "qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0B33B8,stop:0.55 #0A2793,stop:1 #081C6B)";
const QString SIDEBAR_ACTIVE = "#2F8CFF";
const QString SIDEBAR_HOVER = "rgba(255,255,255,45)";
const QString SIDEBAR_TEXT = "#E6EEFF";

const QString GRADIENT = "qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF)";
const QString GRADIENT_HOVER = "qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0)";
const QString ON_ACCENT = "#FFFFFF";      // chữ đậm đặt trên nền pastel để dễ đọc
const QString TABLE_HEAD = "#0A2E9E";     // header bảng xanh đen
const QString PASTEL_GREEN = "#C9D8FF";   // xanh lá pastel (thay cho xanh lá đậm)
const QString PASTEL_GREEN_TEXT = "#1E4FD0";

// Style chung cho các hộp thoại thêm/sửa
QString dialogStyle() {
    return "QDialog { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #EAF2FF,stop:1 #F1E9FF); } QLabel { background:transparent; color:#4A587F; } "
           "QLineEdit, QComboBox, QSpinBox { background:#FFFFFF; border:1px solid #D9E1F6; border-radius:8px; "
           "padding:7px 10px; color:#0F1E4A; } "
           "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border:2px solid #8AA9FF; } "
           "QLineEdit[invalid=\"true\"] { border:1px solid #D64545; background:#FEF2F2; }";
}

// Validate the class start date independently.
void attachStartDateLive(QLineEdit* start) {
    attachLive(start, [](const QString& v) {
        return dateError(v, QString::fromUtf8("Ngày bắt đầu"));
    });
}

const QString BG = "transparent";   // nền thật do CosmicBackdrop vẽ
const QString CARD_BG = "#FFFFFF";
const QString BORDER = "#E1E8F8";

const QString TEXT_MAIN = "#0F1E4A";
const QString TEXT_MUTED = "#7F8BB0";
const QString TEXT_SUBTLE = "#4A587F";

const QString BLUE = "#2458E6";
const QString BLUE_BG = "#E6EEFF";
const QString DARK_BTN = "#2F8CFF";
const QString DARK_HOVER = "#1E4FD0";

const QString RED = "#D63384";
const QString RED_BG = "#FFDFEE";
const QString GREEN = "#17896B";
const QString GREEN_BG = "#D9F7EC";
const QString PEACH_BG = "#FFEDD5";
const QString PEACH_TEXT = "#C2410C";
const QString GRAY_PILL_BG = "#EEF2FC";
const QString GRAY_PILL_TEXT = "#4A587F";

// Thanh tiêu đề gradient pastel đặt ở đầu mỗi hộp thoại thêm/sửa
QWidget* dialogHeader(const QString& title) {
    auto* h = new QFrame;
    h->setObjectName("dialogHeader");
    h->setFixedHeight(56);
    h->setStyleSheet(QString("QFrame#dialogHeader { background:%1; border:none; border-radius:12px; }").arg(GRADIENT));
    auto* l = new QHBoxLayout(h);
    l->setContentsMargins(18, 0, 18, 0);
    auto* t = new QLabel(QString::fromUtf8("🎓  ") + title, h);
    t->setStyleSheet(QString("background:transparent; color:%1; font-size:16px; font-weight:800;").arg(ON_ACCENT));
    l->addWidget(t);
    l->addStretch();
    return h;
}

QLabel* label(const QString& text, int size = 13, bool bold = false,
              const QString& color = TEXT_MAIN) {
    auto* l = new QLabel(text);
    l->setStyleSheet(QString("background:transparent;border:none;color:%1;font-size:%2px;%3")
                     .arg(color).arg(size).arg(bold ? "font-weight:700;" : "font-weight:400;"));
    return l;
}

// Bóng đổ mềm / ánh sáng (glow) cho thẻ và nút, tạo cảm giác "kính" hiện đại
void addGlow(QWidget* w, const QColor& c, int blur = 28, int dy = 8) {
    auto* e = new QGraphicsDropShadowEffect(w);
    e->setBlurRadius(blur);
    e->setOffset(0, dy);
    e->setColor(c);
    w->setGraphicsEffect(e);
}

QFrame* card(QWidget* parent = nullptr) {
    auto* f = new QFrame(parent);
    f->setObjectName("appCard");
    f->setStyleSheet(
        "QFrame#appCard { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(255,255,255,245),stop:0.55 rgba(240,246,255,228),stop:1 rgba(246,236,255,225)); "
        "border:2px solid rgba(255,255,255,235); border-radius:20px; }");
    addGlow(f, QColor(90, 120, 255, 70), 34, 8);
    return f;
}

QPushButton* button(const QString& text, bool primary = true, QWidget* parent = nullptr) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedHeight(38);
    if (primary) addGlow(b, QColor(40, 200, 255, 190), 26, 0);
    b->setStyleSheet(primary
        ? "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:1px solid rgba(255,255,255,160); border-radius:12px; "
          "padding:0 18px; font-size:12px; font-weight:700; }"
          "QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0); }"
        : "QPushButton { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #FFFFFF,stop:1 #EEF2FF); color:#2F4BB8; border:1px solid #DCE4FA; "
          "border-radius:12px; padding:0 18px; font-size:12px; font-weight:700; }"
          "QPushButton:hover { background:#E6EDFF; border:1px solid #AFC3FF; }");
    return b;
}

QPushButton* blueButton(const QString& text, QWidget* parent = nullptr) {
    auto* b = new QPushButton(text, parent);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedHeight(38);
    addGlow(b, QColor(150, 110, 255, 190), 26, 0);
    b->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #6C8CFF,stop:1 #8E5CFF); color:#FFFFFF; border:1px solid rgba(255,255,255,160); border-radius:12px; "
        "padding:0 18px; font-size:12px; font-weight:700; }"
        "QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #5878F0,stop:1 #7C48F0); }");
    return b;
}

QWidget* pillBadge(const QString& text, const QString& bg, const QString& fg, bool withDot = false) {
    auto* container = new QWidget;
    container->setAttribute(Qt::WA_TranslucentBackground, true);
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
        "QProgressBar { background:#E1E8F8; border:none; border-radius:%2px; }"
        "QProgressBar::chunk { background:%1; border-radius:%2px; }")
        .arg(color).arg(height / 2));
    return p;
}

// Avatar tròn vẽ bằng QPainter (khử răng cưa) nên không bao giờ bị cắt méo như khi dùng
// border-radius của stylesheet trong ô bảng.
class AvatarCircle : public QWidget {
    QString text_;
    QColor bg_;
    QColor fg_;
public:
    AvatarCircle(const QString& text, const QColor& bg, const QColor& fg, int size, QWidget* parent = nullptr)
        : QWidget(parent), text_(text), bg_(bg), fg_(fg) {
        setFixedSize(size, size);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(bg_);
        p.drawEllipse(QRectF(0.5, 0.5, width() - 1.0, height() - 1.0));
        QFont f = font();
        f.setPixelSize(13);
        f.setBold(true);
        p.setFont(f);
        p.setPen(fg_);
        p.drawText(rect(), Qt::AlignCenter, text_);
    }
};

QWidget* userAvatarItem(const QString& initials, const QString& name,
                       const QString& bg = "#E6EEFF", const QString& fg = "#1E4FD0",
                       const QString& sub = QString()) {
    auto* w = new QWidget;
    w->setAttribute(Qt::WA_TranslucentBackground, true);
    auto* l = new QHBoxLayout(w);
    l->setContentsMargins(2, 0, 2, 0);
    l->setSpacing(10);

    l->addWidget(new AvatarCircle(initials, QColor(bg), QColor(fg), 38, w), 0, Qt::AlignVCenter);

    auto* col = new QVBoxLayout;
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(1);
    col->addWidget(label(name, 13, true, TEXT_MAIN));
    if (!sub.isEmpty()) col->addWidget(label(sub, 11, false, TEXT_MUTED));
    l->addLayout(col);
    l->addStretch();
    return w;
}

void styleTable(QTableWidget* table, const QString& /*accent*/ = "#2458E6") {
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setStyleSheet(QString(
        "QTableWidget { background:#FFFFFF; alternate-background-color:#F6F8FF; border:none; "
        "color:#4A587F; font-size:12px; }"
        "QTableWidget::item { border-bottom:1px solid #EEF2FC; padding:0px 8px; }"
        "QTableWidget::item:hover { background:#EDF2FF; }"
        "QTableWidget::item:selected { background:#E6EEFF; color:#0F1E4A; }"
        "QHeaderView::section { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2A6BFF,stop:0.5 #1246D0,stop:1 #0A2E9E); color:#FFFFFF; border:none; border-top:1px solid rgba(190,230,255,200); "
        "padding:12px 8px; font-size:11px; font-weight:700; }"
        "QHeaderView::section:first { border-top-left-radius:14px; border-bottom-left-radius:14px; }"
        "QHeaderView::section:last { border-top-right-radius:14px; border-bottom-right-radius:14px; }"));
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(90);
    table->horizontalHeader()->setHighlightSections(false);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->verticalHeader()->setDefaultSectionSize(60);
    table->setTextElideMode(Qt::ElideNone);
    table->setWordWrap(false);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

QMap<QTableWidget*, std::function<void()>>& pagerHooks() {
    static QMap<QTableWidget*, std::function<void()>> m;
    return m;
}

// Nhảy tới trang chứa một dòng cụ thể (dùng sau khi thêm bản ghi mới).
QMap<QTableWidget*, std::function<void(int)>>& pagerJumpHooks() {
    static QMap<QTableWidget*, std::function<void(int)>> m;
    return m;
}

// Co cột vừa khít nội dung (kể cả widget trong ô) để tên không bị cắt chữ.
void fitColumns(QTableWidget* t) {
    if (!t) return;
    for (int r = 0; r < t->rowCount(); ++r) t->setRowHidden(r, false);
    t->resizeColumnsToContents();
    for (int c = 0; c < t->columnCount(); ++c) {
        t->setColumnWidth(c, qMax(t->columnWidth(c) + 24, 90));
    }
    auto& hooks = pagerHooks();
    auto it = hooks.find(t);
    if (it != hooks.end()) it.value()();
}

QString toQ(const std::string& v) { return QString::fromStdString(v); }

// Custom reusable Donut Chart Widget
class DonutChartWidget : public QWidget {
    Q_OBJECT
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

    void setPercent(int p, const QString& mainText) {
        percent = p;
        centerMain = mainText;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QRectF r(12, 12, width() - 24, height() - 24);
        int penWidth = 14;

        QPen bgPen(bgColor, penWidth, Qt::SolidLine, Qt::RoundCap);
        p.setPen(bgPen);
        p.drawArc(r, 0, 360 * 16);

        QPen arcPen(arcColor, penWidth, Qt::SolidLine, Qt::RoundCap);
        p.setPen(arcPen);
        p.drawArc(r, 90 * 16, -int((percent / 100.0) * 360 * 16));

        p.setPen(isDark ? QColor("#FFFFFF") : QColor("#0F1E4A"));
        QFont fMain = p.font();
        fMain.setPointSize(16);
        fMain.setBold(true);
        p.setFont(fMain);
        p.drawText(QRectF(0, height() / 2 - 20, width(), 24), Qt::AlignCenter, centerMain);

        p.setPen(isDark ? QColor("#4AA8FF") : QColor("#7F8BB0"));
        QFont fSub = p.font();
        fSub.setPointSize(8);
        fSub.setBold(true);
        p.setFont(fSub);
        p.drawText(QRectF(0, height() / 2 + 5, width(), 16), Qt::AlignCenter, centerSub);
    }
};


// ---------------------------------------------------------------------------
// Biểu đồ vẽ tay bằng QPainter (không cần Qt Charts)
// ---------------------------------------------------------------------------
class BarChartWidget : public QWidget {
    QStringList labels;
    QVector<double> values;
    QColor color;
public:
    explicit BarChartWidget(const QColor& c = QColor("#2F8CFF"), QWidget* parent = nullptr)
        : QWidget(parent), color(c) {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    void setData(const QStringList& l, const QVector<double>& v) {
        labels = l;
        values = v;
        update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QFont base = p.font();
        base.setPointSize(8);
        p.setFont(base);

        if (labels.isEmpty()) {
            p.setPen(QColor("#9AA6C8"));
            p.drawText(rect(), Qt::AlignCenter, QString::fromUtf8("Chưa có dữ liệu"));
            return;
        }
        double maxV = 1;
        for (double v : values) maxV = qMax(maxV, v);
        int top = static_cast<int>(maxV);
        if (top < maxV) ++top;
        if (top < 4) top = 4;
        while (top % 4 != 0) ++top;
        maxV = top;

        QRectF area(34, 18, width() - 46, height() - 52);
        p.setPen(QPen(QColor("#E1E8F8"), 1));
        for (int i = 0; i <= 4; ++i) {
            double y = area.bottom() - area.height() * i / 4.0;
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            p.setPen(QColor("#9AA6C8"));
            p.drawText(QRectF(0, y - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(maxV * i / 4.0, 'f', 0));
            p.setPen(QPen(QColor("#E1E8F8"), 1));
        }
        int n = labels.size();
        double slot = area.width() / n;
        double bw = qMin(46.0, slot * 0.55);
        QFontMetrics fm(base);
        for (int i = 0; i < n; ++i) {
            double h = area.height() * values[i] / maxV;
            QRectF bar(area.left() + slot * i + (slot - bw) / 2, area.bottom() - h, bw, h);
            QLinearGradient g(bar.topLeft(), bar.bottomLeft());
            // màu cột chuyển dần từ xanh lơ sang tím (giống mẫu thiết kế)
            const double tt = n > 1 ? double(i) / (n - 1) : 0.0;
            const QColor c1(34, 200, 255), c2(139, 108, 255);
            QColor cc(int(c1.red() + (c2.red() - c1.red()) * tt), int(c1.green() + (c2.green() - c1.green()) * tt),
                      int(c1.blue() + (c2.blue() - c1.blue()) * tt));
            g.setColorAt(0, cc);
            g.setColorAt(1, cc.lighter(125));
            p.setPen(Qt::NoPen);
            p.setBrush(g);
            p.drawRoundedRect(bar, 5, 5);
            p.setPen(QColor("#0F1E4A"));
            p.drawText(QRectF(bar.left() - 10, bar.top() - 18, bw + 20, 16), Qt::AlignCenter,
                       QString::number(values[i], 'f', 0));
            p.setPen(QColor("#7F8BB0"));
            QString lb = fm.elidedText(labels[i], Qt::ElideRight, static_cast<int>(slot) - 4);
            p.drawText(QRectF(area.left() + slot * i, area.bottom() + 6, slot, 18), Qt::AlignCenter, lb);
        }
    }
};

class SegmentDonutWidget : public QWidget {
    QStringList labels;
    QVector<double> values;
    QVector<QColor> colors;
    QString centerSub;
public:
    explicit SegmentDonutWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    void setData(const QStringList& l, const QVector<double>& v, const QVector<QColor>& c,
                 const QString& sub) {
        labels = l;
        values = v;
        colors = c;
        centerSub = sub;
        update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        double total = 0;
        for (double v : values) total += v;

        int side = qMin(height() - 16, 170);
        if (side < 80) side = 80;
        QRectF r(14 + 9, (height() - side) / 2.0 + 9, side - 18, side - 18);
        const int pw = 18;

        p.setPen(QPen(QColor("#EEF2FC"), pw, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(r, 0, 360 * 16);
        int start = 90 * 16;
        if (total > 0) {
            for (int i = 0; i < values.size() && i < colors.size(); ++i) {
                if (values[i] <= 0) continue;
                int span = -static_cast<int>(values[i] / total * 360.0 * 16.0);
                p.setPen(QPen(colors[i], pw, Qt::SolidLine, Qt::FlatCap));
                p.drawArc(r, start, span);
                start += span;
            }
        }
        QFont f = p.font();
        f.setPointSize(18);
        f.setBold(true);
        p.setFont(f);
        p.setPen(QColor("#0F1E4A"));
        p.drawText(QRectF(r.left(), r.center().y() - 22, r.width(), 28), Qt::AlignCenter,
                   QString::number(static_cast<int>(total)));
        f.setPointSize(8);
        f.setBold(false);
        p.setFont(f);
        p.setPen(QColor("#7F8BB0"));
        p.drawText(QRectF(r.left(), r.center().y() + 6, r.width(), 16), Qt::AlignCenter, centerSub);

        // Chú giải
        double lx = side + 34;
        double rowH = 30;
        double y0 = (height() - rowH * labels.size()) / 2.0;
        for (int i = 0; i < labels.size() && i < colors.size(); ++i) {
            double y = y0 + rowH * i;
            p.setPen(Qt::NoPen);
            p.setBrush(colors[i]);
            p.drawRoundedRect(QRectF(lx, y + 9, 12, 12), 3, 3);
            QFont lf = p.font();
            lf.setPointSize(9);
            lf.setBold(false);
            p.setFont(lf);
            p.setPen(QColor("#4A587F"));
            p.drawText(QRectF(lx + 20, y, width() - lx - 90, rowH), Qt::AlignLeft | Qt::AlignVCenter, labels[i]);
            lf.setBold(true);
            p.setFont(lf);
            p.setPen(QColor("#0F1E4A"));
            int pct = total > 0 ? static_cast<int>(values[i] * 100.0 / total + 0.5) : 0;
            p.drawText(QRectF(width() - 84, y, 76, rowH), Qt::AlignRight | Qt::AlignVCenter,
                       QString("%1  (%2%)").arg(static_cast<int>(values[i])).arg(pct));
        }
    }
};

class LineChartWidget : public QWidget {
    QStringList labels;
    QVector<double> values;
public:
    explicit LineChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    void setData(const QStringList& l, const QVector<double>& v) {
        labels = l;
        values = v;
        update();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QFont base = p.font();
        base.setPointSize(8);
        p.setFont(base);
        if (labels.isEmpty()) {
            p.setPen(QColor("#9AA6C8"));
            p.drawText(rect(), Qt::AlignCenter, QString::fromUtf8("Chưa có dữ liệu"));
            return;
        }
        double maxV = 4;
        for (double v : values) maxV = qMax(maxV, v);
        int top = static_cast<int>(maxV);
        if (top < maxV) ++top;
        while (top % 4 != 0) ++top;
        maxV = top;

        QRectF area(40, 22, width() - 62, height() - 56);
        for (int i = 0; i <= 4; ++i) {
            double y = area.bottom() - area.height() * i / 4.0;
            p.setPen(QPen(QColor("#E1E8F8"), 1, Qt::DashLine));
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            p.setPen(QColor("#9AA6C8"));
            p.drawText(QRectF(0, y - 8, 34, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(maxV * i / 4.0, 'f', 0));
        }
        int n = labels.size();
        QVector<QPointF> pts;
        for (int i = 0; i < n; ++i) {
            double x = n == 1 ? area.center().x() : area.left() + area.width() * i / (n - 1);
            double y = area.bottom() - area.height() * values[i] / maxV;
            pts.push_back(QPointF(x, y));
        }
        QPolygonF poly;
        poly << QPointF(pts.first().x(), area.bottom());
        for (const QPointF& pt : pts) poly << pt;
        poly << QPointF(pts.last().x(), area.bottom());
        QLinearGradient g(0, area.top(), 0, area.bottom());
        g.setColorAt(0, QColor(83, 183, 232, 110));
        g.setColorAt(1, QColor(83, 183, 232, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawPolygon(poly);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor("#2F8CFF"), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        for (int i = 0; i + 1 < pts.size(); ++i) p.drawLine(pts[i], pts[i + 1]);

        for (int i = 0; i < n; ++i) {
            p.setPen(QPen(QColor("#2F8CFF"), 2));
            p.setBrush(QColor("#FFFFFF"));
            p.drawEllipse(pts[i], 4.5, 4.5);
            p.setPen(QColor("#0F1E4A"));
            p.drawText(QRectF(pts[i].x() - 20, pts[i].y() - 22, 40, 16), Qt::AlignCenter,
                       QString::number(values[i], 'f', 0));
            p.setPen(QColor("#7F8BB0"));
            p.drawText(QRectF(pts[i].x() - 30, area.bottom() + 8, 60, 16), Qt::AlignCenter, labels[i]);
        }
    }
};

QFrame* chartCard(const QString& title, const QString& subtitle, QWidget* chart) {
    auto* f = card();
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(20, 16, 20, 16);
    l->setSpacing(4);
    l->addWidget(label(title, 15, true, TEXT_MAIN));
    l->addWidget(label(subtitle, 12, false, TEXT_MUTED));
    l->addSpacing(6);
    l->addWidget(chart, 1);
    return f;
}


// ---------------------------------------------------------------------------
// Thành phần giao diện dùng chung cho các trang quản lý
// ---------------------------------------------------------------------------
void avatarPalette(const QString& seed, QString& bg, QString& fg) {
    static const char* pal[][2] = {{"#E6EEFF", "#0B7A68"}, {"#FCE7F3", "#9D174D"}, {"#D3F4E3", "#2A7556"},
                                   {"#FEF3C7", "#92400E"}, {"#E8EBFD", "#3F4AA8"}, {"#CFFAFE", "#155E75"}};
    uint h = 0;
    for (QChar ch : seed) h = h * 31 + ch.unicode();
    bg = QString(pal[h % 6][0]);
    fg = QString(pal[h % 6][1]);
}

QString initialsOf(const QString& name) {
    QStringList parts = name.trimmed().split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return QString("?");
    return parts.last().left(1).toUpper();
}

QWidget* subjectPills(const QStringList& subs, const QString& note = QString()) {
    static const char* pal[][2] = {{"#E6EEFF", "#1E4FD0"}, {"#FCE7F3", "#BE185D"}, {"#FFEDD5", "#C2410C"},
                                   {"#D3F4E3", "#2A7556"}, {"#E8EBFD", "#4F5BC9"}, {"#CFFAFE", "#0E7490"}};
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(4, 2, 4, 2);
    v->setSpacing(3);
    if (!note.isEmpty()) v->addWidget(label(note, 10, true, TEXT_MUTED));
    auto* row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(5);
    for (const QString& sname : subs) {
        uint h = 0;
        for (QChar ch : sname) h = h * 31 + ch.unicode();
        auto* pl = new QLabel(sname, w);
        pl->setAlignment(Qt::AlignCenter);
        pl->setStyleSheet(QString(
            "background:%1; color:%2; border-radius:10px; padding:3px 11px; font-size:11px; font-weight:700;")
            .arg(QString(pal[h % 6][0]), QString(pal[h % 6][1])));
        row->addWidget(pl);
    }
    row->addStretch();
    v->addLayout(row);
    return w;
}

// Minh họa sách + mũ tốt nghiệp vẽ bằng QPainter
class BookIllustration : public QWidget {
public:
    explicit BookIllustration(QWidget* parent = nullptr) : QWidget(parent) { setFixedSize(150, 90); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(width() / 160.0, height() / 96.0);
        p.setPen(Qt::NoPen);
        struct Book { QRectF r; QColor c; };
        Book books[] = {{QRectF(14, 80, 132, 14), QColor("#8AA9FF")},
                        {QRectF(22, 66, 120, 14), QColor("#9FC4FF")},
                        {QRectF(10, 52, 124, 14), QColor("#C9D8FF")}};
        for (const Book& b : books) {
            p.setBrush(b.c);
            p.drawRoundedRect(b.r, 4, 4);
            p.setBrush(QColor(255, 255, 255, 170));
            p.drawRoundedRect(QRectF(b.r.right() - 10, b.r.top() + 3, 8, b.r.height() - 6), 2, 2);
        }
        // mũ tốt nghiệp
        QPolygonF base;
        base << QPointF(50, 32) << QPointF(50, 50) << QPointF(80, 60) << QPointF(110, 50) << QPointF(110, 32);
        p.setBrush(QColor("#2F6BFF"));
        p.drawPolygon(base);
        QPolygonF board;
        board << QPointF(80, 8) << QPointF(136, 28) << QPointF(80, 46) << QPointF(24, 28);
        p.setBrush(QColor("#2F8CFF"));
        p.drawPolygon(board);
        p.setPen(QPen(QColor("#F59E0B"), 2.2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(80, 28), QPointF(126, 33));
        p.drawLine(QPointF(126, 33), QPointF(126, 54));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#F59E0B"));
        p.drawEllipse(QPointF(126, 56), 3.5, 3.5);
    }
};

QFrame* welcomeStrip(QVector<QLabel*>& out) {
    auto* f = new QFrame;
    f->setObjectName("welcomeStrip");
    f->setStyleSheet(
        "QFrame#welcomeStrip { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #FFFFFF,stop:0.5 #E9F0FF,stop:1 #F2E6FF); border:2px solid rgba(255,255,255,230); border-radius:20px; }");
    auto* l = new QHBoxLayout(f);
    l->setContentsMargins(24, 16, 16, 16);
    l->setSpacing(12);

    int hr = QTime::currentTime().hour();
    QString greet = hr < 12 ? QString::fromUtf8("Chào buổi sáng,")
                  : hr < 18 ? QString::fromUtf8("Chào buổi chiều,")
                            : QString::fromUtf8("Chào buổi tối,");
    auto* col = new QVBoxLayout;
    col->setSpacing(2);
    col->addWidget(label(QString::fromUtf8("👋  ") + greet, 14, true, "#0F1E4A"));
    col->addWidget(label("Admin User!", 28, true, "#0F1E4A"));
    col->addWidget(label(QString::fromUtf8("Chúc bạn một ngày làm việc hiệu quả cùng TutorFlow!"), 12, false, "#7F8BB0"));
    col->addStretch();
    l->addLayout(col, 2);

    struct Tile { const char* icon; const char* title; const char* iconBg; const char* bg; };
    const Tile tiles[3] = {
        {"👥", "Tổng học sinh", "#E6EEFF", "#FFFFFF"},
        {"🎓", "Tổng gia sư", "#D3F4E3", "#F2FBF7"},
        {"📖", "Lớp học đang diễn ra", "#E8EBFD", "#F2F4FF"}};
    for (const Tile& t : tiles) {
        auto* c = new QFrame;
        c->setObjectName("miniStat");
        c->setStyleSheet(QString("QFrame#miniStat { background:%1; border:1px solid #E1E8F8; border-radius:12px; }")
                         .arg(QString(t.bg)));
        c->setMinimumWidth(130);
        auto* v = new QVBoxLayout(c);
        v->setContentsMargins(14, 10, 12, 10);
        v->setSpacing(4);
        auto* top = new QHBoxLayout;
        top->setSpacing(8);
        auto* ic = new QLabel(QString::fromUtf8(t.icon), c);
        ic->setAlignment(Qt::AlignCenter);
        ic->setFixedSize(32, 32);
        ic->setStyleSheet(QString("background:%1; border-radius:14px; font-size:13px;").arg(QString(t.iconBg)));
        top->addWidget(ic);
        top->addWidget(label(QString::fromUtf8(t.title), 11, false, "#4A587F"), 1);
        v->addLayout(top);
        auto* val = label("0", 24, true, "#0F1E4A");
        out.push_back(val);
        v->addWidget(val);
        l->addWidget(c, 1);
    }
    return f;
}

QVector<QPushButton*> statusTabs(QHBoxLayout* row, QComboBox* statusCombo, const QStringList& names) {
    QVector<QPushButton*> tabs;
    auto* group = new QButtonGroup(statusCombo);
    group->setExclusive(true);
    for (int i = 0; i < names.size(); ++i) {
        auto* b = new QPushButton(names[i]);
        b->setProperty("tabName", names[i]);
        b->setCheckable(true);
        b->setChecked(i == 0);
        b->setCursor(Qt::PointingHandCursor);
        b->setFixedHeight(38);
        b->setStyleSheet(
            "QPushButton { background:#FFFFFF; color:#4A587F; border:1px solid #E1E8F8; border-radius:10px; "
            "padding:0 18px; font-size:12px; font-weight:600; }"
            "QPushButton:hover { background:#F1F4FF; }"
            "QPushButton:checked { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:1px solid #2F8CFF; }");
        group->addButton(b, i);
        QObject::connect(b, &QPushButton::clicked, statusCombo, [statusCombo, i]() { statusCombo->setCurrentIndex(i); });
        row->addWidget(b);
        tabs.push_back(b);
    }
    QObject::connect(statusCombo, &QComboBox::currentIndexChanged, group, [tabs](int idx) {
        if (idx >= 0 && idx < tabs.size()) tabs[idx]->setChecked(true);
    });
    return tabs;
}

void setTabCounts(const QVector<QPushButton*>& tabs, const QVector<int>& counts) {
    for (int i = 0; i < tabs.size() && i < counts.size(); ++i) {
        if (tabs[i]) tabs[i]->setText(tabs[i]->property("tabName").toString() + QString("   %1").arg(counts[i]));
    }
}

// Thanh phân trang: hiển thị N dòng/trang, ẩn các dòng còn lại của bảng
class PagerBar : public QWidget {
    QTableWidget* table;
    QString noun;
    int page = 0;
    int pageSize = 10;
    QLabel* totalLabel = nullptr;
    QComboBox* sizeCombo = nullptr;
    QHBoxLayout* pagesLayout = nullptr;
public:
    PagerBar(QTableWidget* t, const QString& nounText, QWidget* parent = nullptr)
        : QWidget(parent), table(t), noun(nounText) {
        auto* l = new QHBoxLayout(this);
        l->setContentsMargins(0, 8, 0, 0);
        l->setSpacing(10);
        l->addWidget(label(QString::fromUtf8("Hiển thị"), 12, false, TEXT_MUTED));
        sizeCombo = new QComboBox(this);
        sizeCombo->addItems({"5", "10", "20", "50"});
        sizeCombo->setCurrentIndex(1);
        sizeCombo->setFixedSize(66, 34);
        sizeCombo->setStyleSheet(
            "QComboBox { background:white; border:1px solid #D9E1F6; border-radius:8px; padding:0 8px; font-size:12px; }");
        l->addWidget(sizeCombo);
        l->addWidget(label(QString::fromUtf8("/ trang"), 12, false, TEXT_MUTED));
        totalLabel = label("", 12, false, TEXT_SUBTLE);
        l->addSpacing(10);
        l->addWidget(totalLabel);
        l->addStretch();
        pagesLayout = new QHBoxLayout;
        pagesLayout->setSpacing(6);
        l->addLayout(pagesLayout);
        connect(sizeCombo, &QComboBox::currentIndexChanged, this, [this](int) {
            pageSize = sizeCombo->currentText().toInt();
            page = 0;
            refresh();
        });
        pagerHooks()[table] = [this]() { refresh(); };
        pagerJumpHooks()[table] = [this](int row) {
            page = row / qMax(1, pageSize);
            refresh();
        };
    }
    ~PagerBar() override {
        pagerHooks().remove(table);
        pagerJumpHooks().remove(table);
    }

    void refresh() {
        int rows = table->rowCount();
        int pages = qMax(1, (rows + pageSize - 1) / pageSize);
        if (page >= pages) page = pages - 1;
        if (page < 0) page = 0;
        for (int r = 0; r < rows; ++r) table->setRowHidden(r, r < page * pageSize || r >= (page + 1) * pageSize);
        totalLabel->setText(QString::fromUtf8("Tổng cộng %1 %2").arg(rows).arg(noun));

        while (QLayoutItem* it = pagesLayout->takeAt(0)) {
            if (QWidget* w = it->widget()) {
                w->hide();
                w->deleteLater();
            }
            delete it;
        }
        auto addBtn = [this](const QString& text, int target, bool current, bool enabled) {
            auto* b = new QPushButton(text, this);
            b->setFixedSize(34, 34);
            b->setEnabled(enabled);
            b->setCursor(Qt::PointingHandCursor);
            b->setStyleSheet(current
                ? "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:none; border-radius:8px; font-weight:700; }"
                : "QPushButton { background:white; color:#4A587F; border:1px solid #E1E8F8; border-radius:8px; }"
                  "QPushButton:hover { background:#F1F4FF; } QPushButton:disabled { color:#CBD5F0; }");
            connect(b, &QPushButton::clicked, this, [this, target]() {
                page = target;
                refresh();
            });
            pagesLayout->addWidget(b);
        };
        addBtn(QString::fromUtf8("‹"), page - 1, false, page > 0);
        int from = qMax(0, qMin(page - 2, pages - 5));
        int to = qMin(pages, from + 5);
        for (int i = from; i < to; ++i) addBtn(QString::number(i + 1), i, i == page, true);
        addBtn(QString::fromUtf8("›"), page + 1, false, page < pages - 1);
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

        p.setPen(QPen(QColor("#E1E8F8"), 1));
        p.setBrush(QColor("#F6F8FF"));
        p.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);

        struct Node { int x, y; QString symbol; QColor col; };
        Node nodes[] = {
            {30, 24, QString::fromUtf8("∑"), QColor("#2458E6")},
            {70, 18, QString::fromUtf8("🎓"), QColor("#2F8CFF")},
            {110, 26, QString::fromUtf8("∫"), QColor("#2458E6")},
            {45, 48, QString::fromUtf8("π"), QColor("#7F8BB0")},
            {95, 48, QString::fromUtf8("√"), QColor("#2458E6")}
        };

        p.setPen(QPen(QColor("#C9D8FF"), 1, Qt::DashLine));
        p.drawLine(30, 24, 70, 18);
        p.drawLine(70, 18, 110, 26);
        p.drawLine(30, 24, 45, 48);
        p.drawLine(70, 18, 95, 48);
        p.drawLine(45, 48, 95, 48);

        for (const auto& n : nodes) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#FFFFFF"));
            p.drawEllipse(QPoint(n.x, n.y), 10, 10);
            p.setPen(QPen(QColor("#E1E8F8"), 1));
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

QLineEdit* addFilter(QHBoxLayout* l, const QString& placeholder, int width = 200) {
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(42);
    edit->setMinimumWidth(width);
    edit->setStyleSheet(
        "QLineEdit { background:white; border:1px solid #D9E1F6; border-radius:10px; "
        "padding:0 12px; font-size:12px; color:#4A587F; }"
        "QLineEdit:focus { border:1px solid #2F8CFF; }");
    l->addWidget(edit, 1);
    return edit;
}

QComboBox* combo(const QStringList& items, int width = 150) {
    auto* c = new QComboBox;
    c->addItems(items);
    c->setFixedSize(width, 42);
    c->setStyleSheet(
        "QComboBox { background:white; border:1px solid #D9E1F6; border-radius:10px; "
        "padding:0 10px; font-size:12px; color:#4A587F; }"
        "QComboBox::drop-down { border:none; width:24px; }");
    return c;
}


// Reflow filter columns at narrow widths. Each label stays directly above
// its corresponding input; actions remain in their own row.
class AdaptiveSearchPanel final : public QFrame {
public:
    using QFrame::QFrame;
    QGridLayout* fieldGrid = nullptr;
    QVector<QPair<QLabel*, QWidget*>> fieldWidgets;
    int lastColumns = -1;
    void layoutFields(int availableWidth) {
        if (!fieldGrid || fieldWidgets.isEmpty()) return;
        const int count = fieldWidgets.size();
        const int columns = availableWidth < 720 ? 2 : (availableWidth < 1100 ? 3 : count);
        if (columns == lastColumns) return;
        for (const auto& pair : fieldWidgets) {
            fieldGrid->removeWidget(pair.first);
            fieldGrid->removeWidget(pair.second);
        }
        for (int col = 0; col < count; ++col)
            fieldGrid->setColumnStretch(col, col < columns ? 1 : 0);
        for (int index = 0; index < count; ++index) {
            const int row = (index / columns) * 2;
            const int column = index % columns;
            fieldGrid->addWidget(fieldWidgets[index].first, row, column);
            fieldGrid->addWidget(fieldWidgets[index].second, row + 1, column);
        }
        lastColumns = columns;
    }
protected:
    void resizeEvent(QResizeEvent* event) override {
        QFrame::resizeEvent(event);
        layoutFields(width());
    }
};

// Bố cục lọc thống nhất: nhãn phía trên, ô nhập ở giữa, hai nút dưới góc phải.
// Một QGridLayout dùng cho cả học sinh, gia sư và lớp, hỗ trợ phím Enter.
QFrame* createSearchPanel(const QVector<QPair<QString, QWidget*>>& fields,
                          const std::function<void()>& onSearch,
                          const std::function<void()>& onClear) {
    auto* panel = new AdaptiveSearchPanel;
    panel->setObjectName("managementSearchPanel");
    panel->setStyleSheet(
        "QFrame#managementSearchPanel{background:#FFFFFF;border:1px solid #E5EAF3;border-radius:18px;}"
        "QLineEdit,QComboBox{background:#FFFFFF;color:#172753;border:1px solid #CED7E6;"
        "border-radius:7px;padding:0 12px;font-size:13px;}"
        "QLineEdit:focus,QComboBox:focus{border:2px solid #5799F8;}"
        "QComboBox::drop-down{border:0;width:24px;}"
    );
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 14, 16, 13);
    layout->setSpacing(11);
    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(10);
    panel->fieldGrid = grid;
    for (int i = 0; i < fields.size(); ++i) {
        auto* fieldName = new QLabel(fields[i].first, panel);
        fieldName->setAlignment(Qt::AlignCenter);
        fieldName->setStyleSheet("border:0;background:transparent;color:#0F1E4A;font-size:13px;font-weight:500;");
        QWidget* field = fields[i].second;
        field->setParent(panel);
        field->setMinimumWidth(0);
        field->setFixedHeight(47);
        field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        panel->fieldWidgets.push_back({fieldName, field});
    }
    layout->addLayout(grid);
    panel->layoutFields(panel->width());
    auto* actions = new QHBoxLayout;
    actions->addStretch();
    auto* findButton = new QPushButton(QString::fromUtf8("🔍  Tìm kiếm"), panel);
    findButton->setObjectName("managementSearchButton");
    findButton->setFixedSize(158, 48);
    findButton->setCursor(Qt::PointingHandCursor);
    findButton->setStyleSheet("QPushButton{background:#04BFAE;color:white;border:0;border-radius:7px;"
                              "font-size:14px;font-weight:650;}QPushButton:hover{background:#00A997;}");
    auto* clearButton = new QPushButton(QString::fromUtf8("Xóa"), panel);
    clearButton->setObjectName("managementClearButton");
    clearButton->setFixedSize(124, 48);
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setStyleSheet("QPushButton{background:#707A85;color:white;border:0;border-radius:7px;"
                               "font-size:14px;font-weight:650;}QPushButton:hover{background:#5B6570;}");
    actions->addWidget(findButton);
    actions->addSpacing(2);
    actions->addWidget(clearButton);
    layout->addLayout(actions);
    QObject::connect(findButton, &QPushButton::clicked, panel, onSearch);
    QObject::connect(clearButton, &QPushButton::clicked, panel, onClear);
    for (const auto& field : fields) {
        if (auto* edit = qobject_cast<QLineEdit*>(field.second)) {
            QObject::connect(edit, &QLineEdit::returnPressed, findButton, &QPushButton::click);
        }
    }
    return panel;
}

// Giữ ID đã chọn khi danh sách thay đổi sau thao tác thêm/xóa.
void syncIdFilter(QComboBox* box, const QStringList& ids) {
    if (!box) return;
    const QString previous = box->currentData().toString();
    const bool blocked = box->blockSignals(true);
    box->clear();
    box->addItem(QString::fromUtf8("Tất cả"), QString());
    for (const QString& id : ids) box->addItem(id, id);
    const int idx = box->findData(previous);
    box->setCurrentIndex(idx >= 0 ? idx : 0);
    box->blockSignals(blocked);
}

bool matchesSearch(const QString& value, const QString& query) {
    if (query.trimmed().isEmpty()) return true;
    const string field = TextUtils::normalize(value.toStdString());
    const string key = TextUtils::normalize(query.toStdString());
    return field.find(key) != string::npos;
}

QLineEdit* searchField(const QString& placeholder) {
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setClearButtonEnabled(true);
    return edit;
}

void applyBanner(QFrame* f, const QString&, const QString&) {
    f->setObjectName("pageBanner");
    f->setStyleSheet(
        "QFrame#pageBanner { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #FFFFFF,stop:0.5 #E6EEFF,stop:1 #EBDDFF); "
        "border:2px solid rgba(255,255,255,230); border-radius:20px; }");
}

void styleBannerButton(QPushButton* b) {
    b->setFixedHeight(44);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:none; border-radius:10px; "
        "padding:0 22px; font-size:13px; font-weight:700; }"
        "QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0); }");
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

QFrame* infoTileWithLabel(const QString& titleText, QLabel*& valLabelOut, const QString& initialValue = "0", const QString& color = BLUE) {
    QColor c(color);
    auto mix = [&c](double t) {
        return QColor(int(255 - (255 - c.red()) * t), int(255 - (255 - c.green()) * t),
                      int(255 - (255 - c.blue()) * t)).name();
    };
    auto* f = new QFrame;
    f->setObjectName("infoTile");
    f->setMinimumHeight(84);
    f->setStyleSheet(QString(
        "QFrame#infoTile { background:%1; border:1px solid %2; border-left:5px solid %3; border-radius:12px; }")
        .arg(mix(0.09), mix(0.28), color));
    auto* l = new QVBoxLayout(f);
    l->setContentsMargins(16, 12, 14, 12);
    l->setSpacing(4);
    l->addWidget(label(titleText, 11, true, TEXT_MUTED));
    valLabelOut = label(initialValue, 24, true, QColor(color).darker(140).name());  // số đậm hơn viền để dễ đọc
    l->addWidget(valLabelOut);
    return f;
}

QWidget* header(QWidget* parent) {
    auto* h = new QWidget(parent);
    h->setFixedHeight(72);
    h->setStyleSheet("background:transparent; border:none;");
    auto* l = new QHBoxLayout(h);
    l->setContentsMargins(24, 11, 24, 11);
    l->setSpacing(12);

    l->addStretch();

    auto* notifBtn = new QPushButton(h);
    notifBtn->setFixedSize(36, 36);
    notifBtn->setCursor(Qt::PointingHandCursor);
    notifBtn->setStyleSheet(
        "QPushButton { background:transparent; border:none; border-radius:6px; } "
        "QPushButton:hover { background:#EEF2FC; }");
    auto* notifLayout = new QHBoxLayout(notifBtn);
    notifLayout->setContentsMargins(0, 0, 0, 0);
    auto* bellIcon = new QLabel(QString::fromUtf8("🔔"), notifBtn);
    bellIcon->setAlignment(Qt::AlignCenter);
    bellIcon->setStyleSheet("font-size:16px; color:#4A587F; background:transparent;");
    notifLayout->addWidget(bellIcon);

    auto* redDot = new QLabel(QString("3"), notifBtn);
    redDot->setFixedSize(15, 15);
    redDot->setAlignment(Qt::AlignCenter);
    redDot->setStyleSheet("background:#EF4444; color:white; border-radius:7px; font-size:9px; font-weight:700;");
    redDot->move(20, 1);
    l->addWidget(notifBtn);

    auto* sep = new QFrame(h);
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Plain);
    sep->setFixedSize(1, 26);
    sep->setStyleSheet("background:#E1E8F8; border:none;");
    l->addWidget(sep);

    auto* account = new QWidget(h);
    auto* al = new QHBoxLayout(account);
    al->setContentsMargins(4, 0, 0, 0);
    al->setSpacing(10);

    auto* textBlock = new QVBoxLayout;
    textBlock->setContentsMargins(0, 0, 0, 0);
    textBlock->setSpacing(1);
    auto* nameLabel = label("Admin User", 12, true, TEXT_MAIN);
    nameLabel->setAlignment(Qt::AlignRight);
    auto* statusLabel = label(QString::fromUtf8("Quản trị viên"), 11, false, TEXT_MUTED);
    statusLabel->setAlignment(Qt::AlignRight);
    textBlock->addWidget(nameLabel);
    textBlock->addWidget(statusLabel);
    al->addLayout(textBlock);

    auto* avatar = new QLabel(QString::fromUtf8("👤"), account);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(34, 34);
    avatar->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1FC8FF,stop:1 #7C5CFF); color:#FFFFFF; border:2px solid #FFFFFF; border-radius:17px; font-size:14px;");
    al->addWidget(avatar);

    l->addWidget(account);
    return h;
}

QFrame* makeMetricCardWithLabel(const QString& titleText, QLabel*& valLabelOut,
                               const QString& initialVal, const QString& trendVal,
                               const QString& trendText, const QString& iconEmoji,
                               const QString& iconBg, const QString& iconFg,
                               bool isTrendPositive = true) {
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
    icon->setFixedSize(40, 40);
    icon->setStyleSheet(QString(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #FFFFFF,stop:1 %1); border:1px solid #FFFFFF; color:%2; border-radius:12px; font-size:16px;")
        .arg(iconBg, iconFg));
    top->addWidget(icon);
    l->addLayout(top);

    valLabelOut = label(initialVal, 28, true, TEXT_MAIN);
    l->addWidget(valLabelOut);

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

    return f;
}

// Ngôi sao lấp lánh 4 cánh (kiểu "sparkle") + quầng sáng quanh nó
void drawSparkle(QPainter& p, QPointF c, qreal r, const QColor& col) {
    QRadialGradient halo(c, r * 2.4);
    QColor h = col;
    h.setAlpha(col.alpha() / 3);
    QColor h0 = col;
    h0.setAlpha(0);
    halo.setColorAt(0, h);
    halo.setColorAt(1, h0);
    p.setPen(Qt::NoPen);
    p.setBrush(halo);
    p.drawEllipse(c, r * 2.4, r * 2.4);

    QPainterPath star;
    const qreal k = r * 0.22;
    star.moveTo(c.x(), c.y() - r);
    star.quadTo(c.x() + k, c.y() - k, c.x() + r, c.y());
    star.quadTo(c.x() + k, c.y() + k, c.x(), c.y() + r);
    star.quadTo(c.x() - k, c.y() + k, c.x() - r, c.y());
    star.quadTo(c.x() - k, c.y() - k, c.x(), c.y() - r);
    p.setBrush(col);
    p.drawPath(star);
}

// Nền "vũ trụ pastel công nghệ": tinh vân màu, dải sáng cong, sao lấp lánh, hành tinh có viền sáng
class CosmicBackdrop : public QWidget {
public:
    explicit CosmicBackdrop(QWidget* parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const qreal W = width(), H = height();
        QLinearGradient base(0, 0, W, H);
        base.setColorAt(0.0, QColor("#E3EFFF"));
        base.setColorAt(0.5, QColor("#E4E8FF"));
        base.setColorAt(1.0, QColor("#F3E6FF"));
        p.fillRect(rect(), base);

        auto glow = [&](QPointF c, qreal r, QColor col) {
            QRadialGradient g(c, r);
            QColor c1 = col;
            c1.setAlpha(0);
            g.setColorAt(0, col);
            g.setColorAt(1, c1);
            p.setPen(Qt::NoPen);
            p.setBrush(g);
            p.drawEllipse(c, r, r);
        };
        // tinh vân: xanh lơ - tím - hồng
        glow(QPointF(W * 0.30, H * 0.05), W * 0.42, QColor(90, 200, 255, 120));
        glow(QPointF(W * 0.88, H * 0.38), W * 0.34, QColor(170, 130, 255, 110));
        glow(QPointF(W * 0.55, H * 1.02), W * 0.40, QColor(255, 150, 215, 105));
        glow(QPointF(W * 0.05, H * 0.85), W * 0.30, QColor(120, 150, 255, 90));

        // dải ánh sáng cong (light ribbons) ở nửa dưới
        for (int i = 0; i < 4; ++i) {
            QPainterPath ribbon;
            qreal y0 = H * (0.70 + 0.07 * i);
            ribbon.moveTo(-20, y0);
            ribbon.cubicTo(W * 0.30, y0 - H * 0.16, W * 0.60, y0 + H * 0.12, W + 20, y0 - H * 0.10 - i * 6);
            QLinearGradient lg(0, 0, W, 0);
            lg.setColorAt(0.0, QColor(255, 255, 255, 0));
            lg.setColorAt(0.5, QColor(255, 255, 255, 120 - i * 18));
            lg.setColorAt(1.0, QColor(190, 160, 255, 140 - i * 20));
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QBrush(lg), 2.2 - i * 0.4));
            p.drawPath(ribbon);
        }

        // hành tinh góc phải dưới: thân + viền sáng + khí quyển
        QPointF pc(W * 0.99, H * 1.04);
        qreal pr = qMin(W, H) * 0.30;
        QRadialGradient atm(pc, pr * 1.35);
        atm.setColorAt(0.70, QColor(200, 170, 255, 0));
        atm.setColorAt(0.78, QColor(200, 190, 255, 120));
        atm.setColorAt(1.0, QColor(200, 190, 255, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(atm);
        p.drawEllipse(pc, pr * 1.35, pr * 1.35);
        QRadialGradient pg(pc + QPointF(-pr * 0.35, -pr * 0.4), pr * 1.3);
        pg.setColorAt(0, QColor(255, 255, 255, 235));
        pg.setColorAt(0.45, QColor(190, 205, 255, 170));
        pg.setColorAt(1, QColor(150, 130, 255, 120));
        p.setBrush(pg);
        p.setPen(QPen(QColor(255, 255, 255, 230), 2.4));
        p.drawEllipse(pc, pr, pr);

        // sao lấp lánh (vị trí cố định theo tỉ lệ cửa sổ)
        struct S { double x, y, r; int tint; };
        static const S stars[] = {{0.34, 0.07, 7, 0}, {0.66, 0.04, 5, 1}, {0.94, 0.17, 8, 2}, {0.47, 0.36, 4, 0},
                                  {0.78, 0.70, 6, 1}, {0.24, 0.90, 5, 2}, {0.71, 0.93, 7, 0}, {0.54, 0.60, 4, 1},
                                  {0.12, 0.30, 5, 2}, {0.87, 0.88, 4, 1}, {0.40, 0.78, 3, 0}, {0.58, 0.22, 3, 2}};
        for (const S& st : stars) {
            QColor col = st.tint == 0 ? QColor(255, 255, 255, 245)
                       : st.tint == 1 ? QColor(160, 225, 255, 240) : QColor(255, 190, 235, 240);
            drawSparkle(p, QPointF(W * st.x, H * st.y), st.r, col);
        }
    }
};

// Thanh menu trái: xanh sapphire sâu, tinh vân phát sáng, sao lấp lánh, viền phải sáng
class StarrySidebar : public QFrame {
public:
    explicit StarrySidebar(QWidget* parent = nullptr) : QFrame(parent) {}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const qreal W = width(), H = height();
        QLinearGradient base(0, 0, 0, H);
        base.setColorAt(0.0, QColor("#0A36C4"));
        base.setColorAt(0.5, QColor("#0A2793"));
        base.setColorAt(1.0, QColor("#07185F"));
        p.fillRect(rect(), base);
        auto glow = [&](QPointF c, qreal r, QColor col) {
            QRadialGradient g(c, r);
            QColor c1 = col;
            c1.setAlpha(0);
            g.setColorAt(0, col);
            g.setColorAt(1, c1);
            p.setPen(Qt::NoPen);
            p.setBrush(g);
            p.drawEllipse(c, r, r);
        };
        glow(QPointF(W * 0.1, H * 0.05), W * 1.2, QColor(60, 190, 255, 110));
        glow(QPointF(W * 1.0, H * 0.45), W * 1.1, QColor(140, 90, 255, 100));
        glow(QPointF(W * 0.2, H * 0.98), W * 1.2, QColor(255, 120, 210, 80));
        // sao
        struct S { double x, y, r; };
        static const S stars[] = {{0.88, 0.07, 3.2}, {0.55, 0.20, 2.2}, {0.92, 0.33, 3.0}, {0.10, 0.47, 2.2},
                                  {0.85, 0.58, 2.6}, {0.30, 0.70, 2.0}, {0.93, 0.80, 3.2}, {0.06, 0.92, 2.4},
                                  {0.62, 0.95, 2.0}, {0.42, 0.12, 1.8}};
        for (const S& st : stars) drawSparkle(p, QPointF(W * st.x, H * st.y), st.r, QColor(255, 255, 255, 230));
        // viền phải phát sáng
        QLinearGradient edge(0, 0, 0, H);
        edge.setColorAt(0, QColor(120, 220, 255, 220));
        edge.setColorAt(0.5, QColor(160, 140, 255, 200));
        edge.setColorAt(1, QColor(255, 150, 220, 200));
        p.setPen(QPen(QBrush(edge), 2));
        p.drawLine(QPointF(W - 1, 0), QPointF(W - 1, H));
    }
};

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();
    classes = FileManager::loadClasses();
    ClassService classService(students, tutors, classes);
    classService.synchronize();
    const int brokenReferences = classService.countBrokenReferences();
    const int duplicateAssignments = classService.countDuplicateAssignments();
    const int invalidClasses = classService.countInvalidClasses();
    if (!FileManager::storageError().empty()) {
        const QString issue = QString::fromStdString(FileManager::storageError());
        QTimer::singleShot(0, this, [this, issue]() {
            QMessageBox::critical(this, QString::fromUtf8("Lỗi đọc dữ liệu — đã khóa lưu"),
                issue + QString::fromUtf8("\nDữ liệu gốc được giữ nguyên. Hãy sao lưu và sửa file TXT trước khi mở lại ứng dụng."));
        });
    }
    if (!FileManager::recoveryError().empty()) {
        const QString issue = QString::fromStdString(FileManager::recoveryError());
        QTimer::singleShot(0, this, [this, issue]() {
            QMessageBox::critical(this, QString::fromUtf8("Không thể khôi phục dữ liệu"),
                issue + QString::fromUtf8("\nMọi thao tác lưu sẽ bị từ chối cho đến khi dữ liệu được sửa."));
        });
    }
    if (brokenReferences > 0 || duplicateAssignments > 0 || invalidClasses > 0) {
        QTimer::singleShot(0, this, [this, brokenReferences, duplicateAssignments, invalidClasses]() {
            QMessageBox::warning(this, QString::fromUtf8("Cảnh báo dữ liệu cũ"),
                QString::fromUtf8("Phát hiện %1 lớp có mã học sinh/gia sư không tồn tại "
                                  ", %2 lần ghép trùng học sinh–gia sư và %3 lớp không hợp lệ. "
                                  "Dữ liệu vẫn được giữ nguyên; vui lòng kiểm tra và xử lý "
                                  "các lớp trùng trong trang quản lý lớp. "
                                  "Số học sinh của gia sư được tính theo học sinh duy nhất.")
                    .arg(brokenReferences).arg(duplicateAssignments).arg(invalidClasses));
        });
    }

    setWindowTitle(QString::fromUtf8("QUẢN LÝ GIA SƯ - Hệ thống quản trị TutorFlow"));
    resize(1360, 860);
    setMinimumSize(1100, 720);

    auto* central = new CosmicBackdrop(this);
    central->setObjectName("centralWidget");
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createSidebar(), 0);

    stackedWidget = new QStackedWidget(central);
    stackedWidget->setObjectName("contentStack");
    stackedWidget->setStyleSheet(QString("QStackedWidget#contentStack { background:%1; border:none; }").arg(BG));

    stackedWidget->addWidget(createDashboardPage());     // 0
    stackedWidget->addWidget(createStudentPage());       // 1
    stackedWidget->addWidget(createTutorPage());         // 2
    stackedWidget->addWidget(createMatchingPage());      // 3
    stackedWidget->addWidget(createClassPage());         // 4
    contractPage = new ContractPage(students, tutors, classes, stackedWidget);
    stackedWidget->addWidget(contractPage);              // 5
    stackedWidget->addWidget(createStatisticsPage());    // 6

    mainLayout->addWidget(stackedWidget, 1);
    setCentralWidget(central);

    refreshAll();
}

MainWindow::~MainWindow() {
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
}

QString MainWindow::joinStrings(Vector<string> items) {
    QStringList parts;
    for (size_t i = 0; i < items.size(); ++i) {
        if (!items[i].empty()) parts << QString::fromStdString(items[i]);
    }
    return parts.join(", ");
}

Student* MainWindow::findStudent(const string& id) {
    for (size_t i = 0; i < students.size(); ++i) {
        if (students[i]->getPersonID() == id) return students[i];
    }
    return nullptr;
}

Tutor* MainWindow::findTutor(const string& id) {
    for (size_t i = 0; i < tutors.size(); ++i) {
        if (tutors[i]->getPersonID() == id) return tutors[i];
    }
    return nullptr;
}

TutoringClass* MainWindow::findClass(const string& id) {
    for (size_t i = 0; i < classes.size(); ++i) {
        if (classes[i]->getClassID() == id) return classes[i];
    }
    return nullptr;
}

void MainWindow::switchPage(int index) {
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(index);
        if (index == 5 && contractPage) contractPage->refresh();
        if (index == 6 && contractPage) {
            contractPage->refresh();
            refreshStatisticsMetrics();
        }
    }
    // Giữ nút trên thanh điều hướng đồng bộ với trang đang mở.
    if (navGroup && navGroup->button(index)) navGroup->button(index)->setChecked(true);
}

void MainWindow::resetStudentFilters() {
    if (studentIdCombo) studentIdCombo->setCurrentIndex(0);
    for (auto* e : {studentNameEdit, studentPhoneEdit, studentSubjectEdit, studentAddressEdit}) if (e) e->clear();
    if (studentStatusCombo) studentStatusCombo->setCurrentIndex(0);
    populateStudentTable();
}

void MainWindow::resetTutorFilters() {
    if (tutorIdCombo) tutorIdCombo->setCurrentIndex(0);
    for (auto* e : {tutorNameEdit, tutorPhoneEdit, tutorSubjectEdit, tutorAddressEdit}) if (e) e->clear();
    if (tutorExperienceCombo) tutorExperienceCombo->setCurrentIndex(0);
    if (tutorStatusCombo) tutorStatusCombo->setCurrentIndex(0);
    populateTutorTable();
}

void MainWindow::resetClassFilters() {
    if (classIdCombo) classIdCombo->setCurrentIndex(0);
    for (auto* e : {classStudentEdit, classTutorEdit, classSubjectEdit, classScheduleEdit}) if (e) e->clear();
    if (classStatusCombo) classStatusCombo->setCurrentIndex(0);
    populateClassTable();
}

void MainWindow::focusNewRecord(int pageIndex, QTableWidget* table, const QString& id) {
    // Khôi phục bộ lọc để bản ghi mới không bị ẩn.
    if (table == studentTable) resetStudentFilters();
    else if (table == tutorTable) resetTutorFilters();
    else if (table == classTable) resetClassFilters();

    switchPage(pageIndex);
    if (!table) return;

    for (int r = 0; r < table->rowCount(); ++r) {
        auto* lb = qobject_cast<QLabel*>(table->cellWidget(r, 0));
        if (!lb || lb->text() != id) continue;
        auto it = pagerJumpHooks().find(table);
        if (it != pagerJumpHooks().end()) it.value()(r);
        table->selectRow(r);
        table->scrollTo(table->model()->index(r, 0), QAbstractItemView::PositionAtCenter);
        break;
    }
}

QWidget* MainWindow::createSidebar() {
    auto* panel = new StarrySidebar;
    panel->setFrameShape(QFrame::NoFrame);
    panel->setFixedWidth(240);
    panel->setObjectName("appSidebar");

    auto* l = new QVBoxLayout(panel);
    l->setContentsMargins(14, 20, 14, 16);
    l->setSpacing(4);

    // Logo + tên hệ thống
    auto* brandWidget = new QFrame(panel);
    brandWidget->setObjectName("brandCard");
    brandWidget->setAttribute(Qt::WA_StyledBackground, true);
    brandWidget->setStyleSheet("QFrame#brandCard { background:transparent; border:none; }");
    auto* bl = new QHBoxLayout(brandWidget);
    bl->setContentsMargins(10, 8, 10, 8);
    bl->setSpacing(10);

    auto* logoIcon = new QLabel(QString::fromUtf8("🎓"), brandWidget);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setFixedSize(44, 44);
    logoIcon->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #FFFFFF,stop:1 #CFE0FF); "
                            "border:1px solid rgba(255,255,255,200); border-radius:12px; font-size:20px; color:#1E4FD0;");
    bl->addWidget(logoIcon);

    auto* brandCol = new QVBoxLayout;
    brandCol->setContentsMargins(0, 0, 0, 0);
    brandCol->setSpacing(2);
    auto* brandTitle = new QLabel(QString::fromUtf8("TutorFlow"), brandWidget);
    brandTitle->setStyleSheet(
        "color:#FFFFFF; font-size:17px; font-weight:800; background:transparent;");
    auto* brandSub = new QLabel(QString::fromUtf8("Quản lý gia sư"), brandWidget);
    brandSub->setStyleSheet("color:#C5D7FF; font-size:11px; font-weight:500; background:transparent;");
    brandCol->addWidget(brandTitle);
    brandCol->addWidget(brandSub);
    bl->addLayout(brandCol);
    bl->addStretch();
    l->addWidget(brandWidget);

    auto* brandLine = new QFrame(panel);
    brandLine->setFixedHeight(1);
    brandLine->setStyleSheet("background:rgba(255,255,255,60); border:none;");
    l->addWidget(brandLine);
    l->addSpacing(8);

    // Nút điều hướng
    auto* group = new QButtonGroup(panel);
    group->setExclusive(true);
    navGroup = group;

    struct NavItem { QString icon; QString text; QString section; };
    const QVector<NavItem> items = {
        {QString::fromUtf8("⊞"), QString::fromUtf8("Tổng quan"), QString::fromUtf8("DASHBOARD")},
        {QString::fromUtf8("👥"), QString::fromUtf8("Học sinh"), QString::fromUtf8("NGHIỆP VỤ")},
        {QString::fromUtf8("👨‍🏫"), QString::fromUtf8("Gia sư"), QString()},
        {QString::fromUtf8("👤⁺"), QString::fromUtf8("Tìm gia sư"), QString()},
        {QString::fromUtf8("📖"), QString::fromUtf8("Lớp học"), QString()},
        {QString::fromUtf8("🧾"), QString::fromUtf8("Hợp đồng"), QString()},
        {QString::fromUtf8("📊"), QString::fromUtf8("Thống kê"), QString::fromUtf8("BÁO CÁO")}
    };

    for (int i = 0; i < items.size(); ++i) {
        if (!items[i].section.isEmpty()) {
            auto* sec = new QLabel(items[i].section, panel);
            sec->setStyleSheet("color:#B9D0FF; font-size:10px; font-weight:800; letter-spacing:1px; "
                               "background:transparent; padding:10px 0 4px 6px;");
            l->addWidget(sec);
        }
        auto* b = new QPushButton(QString("  %1   %2").arg(items[i].icon, items[i].text), panel);
        b->setCheckable(true);
        b->setFixedHeight(42);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString(
            "QPushButton { text-align:left; padding-left:14px; background:transparent; "
            "color:%1; border:none; border-radius:10px; font-size:13px; font-weight:500; } "
            "QPushButton:hover { background:%2; color:#FFFFFF; } "
            "QPushButton:checked { background:%3; color:%4; font-weight:800; border:1px solid #BFF6FF; }")
            .arg(SIDEBAR_TEXT, SIDEBAR_HOVER, GRADIENT, ON_ACCENT));

        {
            auto* neon = new QGraphicsDropShadowEffect(b);
            neon->setBlurRadius(26);
            neon->setOffset(0, 0);
            neon->setColor(QColor(60, 220, 255, 230));
            neon->setEnabled(false);
            b->setGraphicsEffect(neon);
            connect(b, &QPushButton::toggled, neon, &QGraphicsDropShadowEffect::setEnabled);
        }
        if (i == 0) b->setChecked(true);
        group->addButton(b, i);
        connect(b, &QPushButton::clicked, this, [this, i]() { switchPage(i); });
        l->addWidget(b);
    }

    l->addStretch();

    // Thẻ giới thiệu (nền tối, chữ pastel)
    auto* promo = new QFrame(panel);
    promo->setObjectName("promoCard");
    promo->setStyleSheet(
        "QFrame#promoCard { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 rgba(150,120,255,170),stop:1 rgba(40,90,255,110)); "
        "border:1px solid rgba(255,255,255,130); border-radius:14px; }");
    auto* pv = new QVBoxLayout(promo);
    pv->setContentsMargins(14, 14, 14, 8);
    pv->setSpacing(4);
    pv->addWidget(label(QString::fromUtf8("Kết nối tri thức\nKiến tạo tương lai"), 13, true, "#C9D8FF"));
    auto* promoDesc = label(QString::fromUtf8("TutorFlow – Đồng hành cùng bạn trên hành trình học tập và phát triển."), 10, false, "#DCE8FF");
    promoDesc->setWordWrap(true);
    pv->addWidget(promoDesc);
    pv->addWidget(new BookIllustration(promo), 0, Qt::AlignHCenter);
    l->addWidget(promo);
    l->addSpacing(6);

    // Thông tin tài khoản
    auto* profile = new QWidget(panel);
    profile->setStyleSheet("background:transparent;");
    auto* pl = new QHBoxLayout(profile);
    pl->setContentsMargins(6, 6, 6, 6);
    pl->setSpacing(10);

    auto* pa = new QLabel(QString::fromUtf8("👤"), profile);
    pa->setAlignment(Qt::AlignCenter);
    pa->setFixedSize(36, 36);
    pa->setStyleSheet(QString("background:%1; color:%2; border-radius:18px; font-size:16px;")
                      .arg(GRADIENT, ON_ACCENT));
    pl->addWidget(pa);

    auto* userTexts = new QVBoxLayout;
    userTexts->setContentsMargins(0, 0, 0, 0);
    userTexts->setSpacing(1);
    auto* userName = new QLabel("Admin", profile);
    userName->setStyleSheet("color:#FFFFFF; font-size:13px; font-weight:700; background:transparent;");
    auto* userRole = new QLabel(QString::fromUtf8("Quản trị viên"), profile);
    userRole->setStyleSheet("color:#C5D7FF; font-size:11px; background:transparent;");
    userTexts->addWidget(userName);
    userTexts->addWidget(userRole);
    pl->addLayout(userTexts);
    pl->addStretch();
    l->addWidget(profile);

    // Nút đăng xuất: quay về màn hình đăng nhập
    auto* logoutBtn = new QPushButton(QString::fromUtf8("⏻   Đăng xuất"), panel);
    logoutBtn->setFixedHeight(44);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #FFFFFF,stop:1 #D9E4FF); color:#1B2F7A; border:1px solid rgba(255,255,255,200); border-radius:14px; "
        "font-size:13px; font-weight:700; } QPushButton:hover { background:#FFFFFF; }");
    connect(logoutBtn, &QPushButton::clicked, this, [this]() {
        auto* login = new LoginWindow();
        login->setAttribute(Qt::WA_DeleteOnClose, true);
        login->resize(420, 520);
        login->show();
        close();
    });
    l->addWidget(logoutBtn);

    return panel;
}

// -------------------------------------------------------------
// 0. DASHBOARD PAGE
// -------------------------------------------------------------
QWidget* MainWindow::createDashboardPage() {
    auto* page = new QWidget;
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    root->addWidget(header(page));

    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setObjectName("pageContent");
    content->setStyleSheet(QString("QWidget#pageContent { background:%1; }").arg(BG));
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
    auto* title = label(QString::fromUtf8("Tổng quan hệ thống <span style='color:#B48CFF'>✦</span>"), 24, true, TEXT_MAIN);
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

    connect(btnAddStudent, &QPushButton::clicked, this, &MainWindow::onAddStudent);
    connect(btnAddTutor, &QPushButton::clicked, this, &MainWindow::onAddTutor);
    connect(btnFindTutor, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(btnCreateClass, &QPushButton::clicked, this, &MainWindow::onAddClass);

    l->addLayout(headerRow);

    // Grid of 4 Metric Cards
    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Tổng học sinh"), dashTotalStudentsLabel, "0", "", QString::fromUtf8(" theo dữ liệu"), QString::fromUtf8("👥"), "#EEF2FC", "#4A587F", true), 1);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Tổng gia sư"), dashTotalTutorsLabel, "0", "", QString::fromUtf8(" theo dữ liệu"), QString::fromUtf8("🎓"), "#E6EEFF", "#2458E6", true), 1);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Lớp đang hoạt động"), dashActiveClassesLabel, "0", "Thời gian thực", QString::fromUtf8(" hệ thống"), QString::fromUtf8("📥"), "#EEF2FC", "#4A587F", true), 1);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Tỷ lệ ghép thành công"), dashMatchRateLabel, "0%", "Tự động", QString::fromUtf8(" từ dữ liệu"), QString::fromUtf8("🤝"), "#E6EEFF", "#2458E6", true), 1);
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
    dashMatchedStudentsLabel = label("0", 15, true, TEXT_MAIN);
    row1->addWidget(dot1);
    row1->addStretch();
    row1->addWidget(dashMatchedStudentsLabel);
    scLayout->addLayout(row1);

    auto* row2 = new QHBoxLayout;
    auto* dot2 = new QLabel(QString::fromUtf8("●  Chưa ghép"), studentStatusCard);
    dot2->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(TEXT_MUTED));
    dashUnmatchedStudentsLabel = label("0", 15, true, TEXT_MAIN);
    row2->addWidget(dot2);
    row2->addStretch();
    row2->addWidget(dashUnmatchedStudentsLabel);
    scLayout->addLayout(row2);

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
    dashAvailTutorsLabel = label("0", 14, true, TEXT_MAIN);
    tRow1->addWidget(tDot1);
    tRow1->addStretch();
    tRow1->addWidget(dashAvailTutorsLabel);
    tcLayout->addLayout(tRow1);

    auto* tRow2 = new QHBoxLayout;
    auto* tDot2 = new QLabel(QString::fromUtf8("●  Bận / Đã nhận lớp"), tutorStatusCard);
    tDot2->setStyleSheet(QString("color:%1; font-size:13px; font-weight:500;").arg(TEXT_MAIN));
    dashBusyTutorsLabel = label("0", 14, true, TEXT_MAIN);
    tRow2->addWidget(tDot2);
    tRow2->addStretch();
    tRow2->addWidget(dashBusyTutorsLabel);
    tcLayout->addLayout(tRow2);

    leftCol->addWidget(tutorStatusCard);
    // Card 3: Biểu đồ gia sư theo môn
    dashChartTutorSubject = new BarChartWidget(QColor("#2F8CFF"));
    dashChartTutorSubject->setMinimumHeight(200);
    auto* dashChartCard = chartCard(QString::fromUtf8("Gia sư theo môn dạy"),
                                    QString::fromUtf8("Số lượng gia sư phụ trách từng môn"),
                                    dashChartTutorSubject);
    dashChartCard->setFixedWidth(330);
    leftCol->addWidget(dashChartCard);
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
        "QPushButton { background:transparent; color:#2458E6; border:none; "
        "font-size:11px; font-weight:700; } QPushButton:hover { color:#1E4FD0; }");
    connect(seeAll, &QPushButton::clicked, this, [this]() { switchPage(4); });
    rHead->addWidget(seeAll);
    rcLayout->addLayout(rHead);

    dashboardRecentTable = new QTableWidget(0, 6, recentCard);
    dashboardRecentTable->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ LỚP"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("MÔN HỌC"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(dashboardRecentTable);
    dashboardRecentTable->setFixedHeight(46 + 5 * 60 + 12);
    dashboardRecentTable->setColumnWidth(0, 85);
    dashboardRecentTable->setColumnWidth(1, 190);
    dashboardRecentTable->setColumnWidth(2, 130);
    dashboardRecentTable->setColumnWidth(3, 140);
    dashboardRecentTable->setColumnWidth(4, 110);
    dashboardRecentTable->setColumnWidth(5, 70);

    rcLayout->addWidget(dashboardRecentTable);
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
        QString::fromUtf8("Hệ thống tự động đồng bộ và lưu trữ dữ liệu thời gian thực giữa giao diện và các tệp văn bản. "
                          "Mọi thay đổi trên giao diện đều được cập nhật bền vững vào cơ sở dữ liệu."), 12, false, TEXT_MUTED);
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
    content->setAutoFillBackground(false);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateDashboardRecentTable() {
    if (!dashboardRecentTable) return;
    int count = min(5, static_cast<int>(classes.size()));
    dashboardRecentTable->setRowCount(count);

    for (int i = 0; i < count; ++i) {
        TutoringClass* c = classes[classes.size() - 1 - i]; // Most recent first
        dashboardRecentTable->setCellWidget(i, 0, label(QString::fromStdString(c->getClassID()), 12, true, BLUE));

        Student* s = findStudent(c->getStudentID());
        QString sName = s ? QString::fromStdString(s->getFullName()) : QString::fromStdString(c->getStudentID());
        QString sInit = sName.isEmpty() ? "S" : sName.left(1).toUpper();
        QString abg, afg;
        avatarPalette(sName, abg, afg);
        dashboardRecentTable->setCellWidget(i, 1, userAvatarItem(sInit, sName, abg, afg));

        Tutor* t = findTutor(c->getTutorID());
        QString tName = t ? QString::fromStdString(t->getFullName()) : (c->getTutorID().empty() ? QString::fromUtf8("Chưa có") : QString::fromStdString(c->getTutorID()));
        dashboardRecentTable->setCellWidget(i, 2, label(tName, 12, false, TEXT_SUBTLE));

        dashboardRecentTable->setCellWidget(i, 3, label(QString::fromStdString(c->getSubject()), 12, false, TEXT_MAIN));

        QString st = QString::fromStdString(c->getStatus());
        QString bg = st == "Đang học" ? GREEN_BG : (st == "Chờ xếp lịch" ? GRAY_PILL_BG : RED_BG);
        QString fg = st == "Đang học" ? GREEN : (st == "Chờ xếp lịch" ? GRAY_PILL_TEXT : RED);
        dashboardRecentTable->setCellWidget(i, 4, pillBadge(st, bg, fg));

        auto* btnAction = new QPushButton(QString::fromUtf8("•••"), dashboardRecentTable);
        btnAction->setFixedSize(30, 26);
        btnAction->setCursor(Qt::PointingHandCursor);
        btnAction->setStyleSheet(
            "QPushButton { background:transparent; color:#7F8BB0; border:none; font-size:14px; font-weight:bold; } "
            "QPushButton:hover { color:#0F1E4A; background:#EEF2FC; border-radius:4px; }");
        connect(btnAction, &QPushButton::clicked, this, [this, c]() { onEditClass(c); });
        dashboardRecentTable->setCellWidget(i, 5, btnAction);
    }
    fitColumns(dashboardRecentTable);
}

// -------------------------------------------------------------
// 1. QUẢN LÝ HỌC SINH
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
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setObjectName("pageContent");
    content->setStyleSheet(QString("QWidget#pageContent { background:%1; }").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#4AA8FF", "#7C8CF0");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý học sinh"), 22, true, "#0F1E4A");
    auto* pageDesc = label(QString::fromUtf8("Quản lý danh sách, tìm kiếm và phân bổ học sinh cho gia sư."), 13, false, "#7F8BB0");
    tCol->addWidget(pageTitle);
    tCol->addWidget(pageDesc);
    topLayout->addLayout(tCol, 1);

    auto* btnAddStudent = new QPushButton(QString::fromUtf8("＋  Thêm học sinh"), topCard);
    btnAddStudent->setFixedHeight(40);
    btnAddStudent->setCursor(Qt::PointingHandCursor);
    btnAddStudent->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:none; border-radius:6px; "
        "padding:0 20px; font-size:13px; font-weight:600; } QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0); }");
    connect(btnAddStudent, &QPushButton::clicked, this, &MainWindow::onAddStudent);
    styleBannerButton(btnAddStudent);
    topLayout->addWidget(btnAddStudent);
    topLayout->addWidget(new BookIllustration(topCard));
    l->addWidget(topCard);

    auto* stuSummary = new QHBoxLayout;
    stuSummary->setSpacing(14);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Tổng học sinh"), stuSumTotal, "0", "#4AA8FF"), 1);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Đã ghép gia sư"), stuSumMatched, "0", GREEN), 1);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Chưa ghép"), stuSumUnmatched, "0", "#F59E0B"), 1);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Ngân sách TB/giờ"), stuSumAvgBudget, "0đ", "#7C8CF0"), 1);
    l->addLayout(stuSummary);

    // Các bộ lọc độc lập theo thiết kế: ID, tên, số điện thoại, môn học, địa chỉ.
    studentIdCombo = new QComboBox;
    studentNameEdit = searchField(QString::fromUtf8("Họ tên"));
    studentPhoneEdit = searchField(QString::fromUtf8("SĐT"));
    studentSubjectEdit = searchField(QString::fromUtf8("Môn học"));
    studentAddressEdit = searchField(QString::fromUtf8("Địa chỉ"));
    studentStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")}, 150);
    connect(studentStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateStudentTable(); });
    studentStatusCombo->setParent(content);
    studentStatusCombo->hide();  // trạng thái dùng bộ tab phía dưới
    l->addWidget(createSearchPanel({
        {"ID", studentIdCombo}, {QString::fromUtf8("Họ và tên"), studentNameEdit},
        {QString::fromUtf8("Số điện thoại"), studentPhoneEdit},
        {QString::fromUtf8("Môn học"), studentSubjectEdit},
        {QString::fromUtf8("Địa chỉ"), studentAddressEdit}
    }, [this]() { populateStudentTable(); }, [this]() { resetStudentFilters(); }));

    {
        auto* tabRow = new QHBoxLayout;
        tabRow->setSpacing(10);
        stuTabs = statusTabs(tabRow, studentStatusCombo, QStringList{QString::fromUtf8("Tất cả"), QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")});
        tabRow->addStretch();
        l->addLayout(tabRow);
    }

    // Table Card
    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(18, 14, 18, 14);

    studentTable = new QTableWidget(0, 7, tableCard);
    studentTable->setHorizontalHeaderLabels({
        QString::fromUtf8("Mã HS"),
        QString::fromUtf8("Học sinh"),
        QString::fromUtf8("Lớp & Môn"),
        QString::fromUtf8("Lịch rảnh"),
        QString::fromUtf8("Ngân sách"),
        QString::fromUtf8("Trạng thái"),
        QString::fromUtf8("Thao tác")
    });
    styleTable(studentTable, "#4AA8FF");
    studentTable->setMinimumHeight(350);

    tl->addWidget(studentTable);
    tl->addWidget(new PagerBar(studentTable, QString::fromUtf8("học sinh"), tableCard));
    l->addWidget(tableCard);

    l->addStretch();
    scroll->setWidget(content);
    content->setAutoFillBackground(false);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateStudentTable() {
    if (!studentTable) return;
    QStringList ids;
    for (size_t i = 0; i < students.size(); ++i) ids << toQ(students[i]->getPersonID());
    syncIdFilter(studentIdCombo, ids);
    const QString idFilter = studentIdCombo ? studentIdCombo->currentData().toString() : QString();
    const QString nameFilter = studentNameEdit ? studentNameEdit->text().trimmed() : QString();
    const QString phoneFilter = studentPhoneEdit ? studentPhoneEdit->text().trimmed() : QString();
    const QString subjectFilter = studentSubjectEdit ? studentSubjectEdit->text().trimmed() : QString();
    const QString addressFilter = studentAddressEdit ? studentAddressEdit->text().trimmed() : QString();
    const QString statusFilter = studentStatusCombo ? studentStatusCombo->currentText() : QString();

    Vector<Student*> filtered;
    for (size_t i = 0; i < students.size(); ++i) {
        Student* s = students[i];
        if (!idFilter.isEmpty() && toQ(s->getPersonID()) != idFilter) continue;
        if (!nameFilter.isEmpty() && !matchesSearch(toQ(s->getFullName()), nameFilter)) continue;
        if (!phoneFilter.isEmpty() && !matchesSearch(toQ(s->getPhone()), phoneFilter)) continue;
        if (!subjectFilter.isEmpty() && !matchesSearch(joinStrings(s->getNeedSubjects()), subjectFilter)) continue;
        if (!addressFilter.isEmpty() && !matchesSearch(toQ(s->getAddress()), addressFilter)) continue;
        if (statusFilter == QString::fromUtf8("Đã ghép") && !s->getIsMatched()) continue;
        if (statusFilter == QString::fromUtf8("Chưa ghép") && s->getIsMatched()) continue;

        filtered.push_back(s);
    }

    studentTable->setRowCount(static_cast<int>(filtered.size()));
    for (size_t i = 0; i < filtered.size(); ++i) {
        Student* s = filtered[i];
        int row = static_cast<int>(i);

        studentTable->setCellWidget(row, 0, label(QString::fromStdString(s->getPersonID()), 12, true, BLUE));

        QString initial = initialsOf(toQ(s->getFullName()));
        {
            QString abg, afg;
            avatarPalette(toQ(s->getFullName()), abg, afg);
            studentTable->setCellWidget(row, 1, userAvatarItem(initial, toQ(s->getFullName()), abg, afg,
                                                               QString::fromUtf8("📞  ") + toQ(s->getPhone())));
        }

        {
            QStringList subs;
            Vector<string>& sv = s->getNeedSubjects();
            for (size_t k = 0; k < sv.size(); ++k) if (!sv[k].empty()) subs << toQ(sv[k]);
            studentTable->setCellWidget(row, 2, subjectPills(subs, QString::fromUtf8("Lớp ") + toQ(s->getGrade())));
        }

        // Schedule
        auto* schW = new QWidget;
        auto* schl = new QHBoxLayout(schW);
        schl->setContentsMargins(0, 0, 0, 0);
        schl->setSpacing(4);
        schl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QStringList dList = QString::fromStdString(s->getAvailableSchedule()).split(',', Qt::SkipEmptyParts);
        for (const auto& d : dList) {
            auto* p = new QLabel(d.trimmed(), schW);
            p->setAlignment(Qt::AlignCenter);
            p->setFixedSize(30, 24);
            p->setStyleSheet("background:#D3F4E3; color:#2A7556; border-radius:12px; font-size:11px; font-weight:700;");
            schl->addWidget(p);
        }
        studentTable->setCellWidget(row, 3, schW);

        // Budget
        studentTable->setCellWidget(row, 4, label(QString::fromStdString(s->getBudgetPerHour()) + QString::fromUtf8("đ/giờ"), 12, true, BLUE));

        // Status
        studentTable->setCellWidget(row, 5, pillBadge(s->getIsMatched() ? QString::fromUtf8("Đã ghép") : QString::fromUtf8("Chưa ghép"),
                                                       s->getIsMatched() ? GREEN_BG : PEACH_BG,
                                                       s->getIsMatched() ? GREEN : PEACH_TEXT, true));

        // Action buttons
        auto* actW = new QWidget;
        auto* actL = new QHBoxLayout(actW);
        actL->setContentsMargins(4, 2, 4, 2);
        actL->setSpacing(4);

        auto* btnView = new QPushButton(QString::fromUtf8("👁"), actW);
        btnView->setToolTip(QString::fromUtf8("Xem chi tiết"));
        btnView->setFixedSize(32, 32);
        btnView->setStyleSheet("QPushButton { background:#E6EEFF; border:none; border-radius:16px; color:#2F8CFF; font-size:12px; } QPushButton:hover { background:#E1E8F8; }");

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#F3F6FF; border:none; border-radius:16px; color:#2458E6; font-size:12px; } QPushButton:hover { background:#E6EEFF; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#D64545; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

        actL->addWidget(btnView);
        actL->addWidget(btnEdit);
        actL->addWidget(btnDel);
        actL->addStretch();
        studentTable->setCellWidget(row, 6, actW);

        connect(btnView, &QPushButton::clicked, this, [this, s]() { onViewStudent(s); });
        connect(btnEdit, &QPushButton::clicked, this, [this, s]() { onEditStudent(s); });
        connect(btnDel, &QPushButton::clicked, this, [this, s]() { onDeleteStudent(s); });
    }
    fitColumns(studentTable);
}

// -------------------------------------------------------------
// 2. QUẢN LÝ GIA SƯ
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
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setObjectName("pageContent");
    content->setStyleSheet(QString("QWidget#pageContent { background:%1; }").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#6572E3", "#DB2777");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý gia sư"), 22, true, "#0F1E4A");
    auto* pageDesc = label(QString::fromUtf8("Danh sách gia sư, chuyên môn và lịch nhận lớp."), 13, false, "#7F8BB0");
    tCol->addWidget(pageTitle);
    tCol->addWidget(pageDesc);
    topLayout->addLayout(tCol, 1);

    auto* btnAddTutor = blueButton(QString::fromUtf8("👨‍🏫⁺  Thêm gia sư"), topCard);
    connect(btnAddTutor, &QPushButton::clicked, this, &MainWindow::onAddTutor);
    styleBannerButton(btnAddTutor);
    topLayout->addWidget(btnAddTutor);
    topLayout->addWidget(new BookIllustration(topCard));
    l->addWidget(topCard);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Tổng gia sư"), tutorSumTotal, "0", BLUE), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Sẵn sàng"), tutorSumAvail, "0", GREEN), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Đang bận"), tutorSumBusy, "0", RED), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Mức phí TB"), tutorSumAvgRate, "0đ", BLUE), 1);
    l->addLayout(summary);

    tutorIdCombo = new QComboBox;
    tutorNameEdit = searchField(QString::fromUtf8("Họ tên"));
    tutorPhoneEdit = searchField(QString::fromUtf8("SĐT"));
    tutorSubjectEdit = searchField(QString::fromUtf8("Môn"));
    tutorExperienceCombo = new QComboBox;
    tutorExperienceCombo->addItems({QString::fromUtf8("Tất cả"), QString::fromUtf8("Dưới 1 năm"),
                                     QString::fromUtf8("1–2 năm"), QString::fromUtf8("3–4 năm"),
                                     QString::fromUtf8("5–9 năm"), QString::fromUtf8("Từ 10 năm")});
    tutorAddressEdit = searchField(QString::fromUtf8("Địa chỉ"));
    tutorStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Sẵn sàng"), QString::fromUtf8("Bận")}, 140);
    connect(tutorStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateTutorTable(); });
    tutorStatusCombo->setParent(content);
    tutorStatusCombo->hide();
    l->addWidget(createSearchPanel({
        {"ID", tutorIdCombo}, {QString::fromUtf8("Họ và tên"), tutorNameEdit},
        {QString::fromUtf8("Số điện thoại"), tutorPhoneEdit},
        {QString::fromUtf8("Môn dạy"), tutorSubjectEdit},
        {QString::fromUtf8("Kinh nghiệm"), tutorExperienceCombo},
        {QString::fromUtf8("Địa chỉ"), tutorAddressEdit}
    }, [this]() { populateTutorTable(); }, [this]() { resetTutorFilters(); }));

    {
        auto* tabRow = new QHBoxLayout;
        tabRow->setSpacing(10);
        tutorTabs = statusTabs(tabRow, tutorStatusCombo, QStringList{QString::fromUtf8("Tất cả"), QString::fromUtf8("Sẵn sàng"), QString::fromUtf8("Bận")});
        tabRow->addStretch();
        l->addLayout(tabRow);
    }

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    tutorTable = new QTableWidget(0, 9, tableCard);
    tutorTable->setHorizontalHeaderLabels({
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
    styleTable(tutorTable, "#6572E3");
    tutorTable->setMinimumHeight(350);

    tl->addWidget(tutorTable);
    tl->addWidget(new PagerBar(tutorTable, QString::fromUtf8("gia sư"), tableCard));
    l->addWidget(tableCard, 1);

    l->addStretch();
    scroll->setWidget(content);
    content->setAutoFillBackground(false);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateTutorTable() {
    if (!tutorTable) return;
    QStringList ids;
    for (size_t i = 0; i < tutors.size(); ++i) ids << toQ(tutors[i]->getPersonID());
    syncIdFilter(tutorIdCombo, ids);
    const QString idFilter = tutorIdCombo ? tutorIdCombo->currentData().toString() : QString();
    const QString nameFilter = tutorNameEdit ? tutorNameEdit->text().trimmed() : QString();
    const QString phoneFilter = tutorPhoneEdit ? tutorPhoneEdit->text().trimmed() : QString();
    const QString subjFilter = tutorSubjectEdit ? tutorSubjectEdit->text().trimmed() : QString();
    const QString areaFilter = tutorAddressEdit ? tutorAddressEdit->text().trimmed() : QString();
    const int expFilter = tutorExperienceCombo ? tutorExperienceCombo->currentIndex() : 0;
    const QString statusFilter = tutorStatusCombo ? tutorStatusCombo->currentText() : QString();

    Vector<Tutor*> filtered;
    for (size_t i = 0; i < tutors.size(); ++i) {
        Tutor* t = tutors[i];
        if (!idFilter.isEmpty() && toQ(t->getPersonID()) != idFilter) continue;
        if (!nameFilter.isEmpty() && !matchesSearch(toQ(t->getFullName()), nameFilter)) continue;
        if (!phoneFilter.isEmpty() && !matchesSearch(toQ(t->getPhone()), phoneFilter)) continue;
        if (!subjFilter.isEmpty() && !matchesSearch(joinStrings(t->getSubjects()), subjFilter)) continue;
        // Địa chỉ là địa chỉ cá nhân hoặc khu vực nhận lớp.
        if (!areaFilter.isEmpty() && !matchesSearch(toQ(t->getAddress()), areaFilter) &&
            !matchesSearch(joinStrings(t->getTeachingAreas()), areaFilter)) continue;
        const int years = t->getYearsOfExperience();
        if ((expFilter == 1 && years >= 1) || (expFilter == 2 && (years < 1 || years > 2)) ||
            (expFilter == 3 && (years < 3 || years > 4)) ||
            (expFilter == 4 && (years < 5 || years > 9)) ||
            (expFilter == 5 && years < 10)) continue;
        if (statusFilter == QString::fromUtf8("Sẵn sàng") && !ClassService::canReceiveStudents(*t)) continue;
        if (statusFilter == QString::fromUtf8("Bận") && ClassService::canReceiveStudents(*t)) continue;

        filtered.push_back(t);
    }

    tutorTable->setRowCount(static_cast<int>(filtered.size()));
    for (size_t i = 0; i < filtered.size(); ++i) {
        Tutor* t = filtered[i];
        int row = static_cast<int>(i);

        tutorTable->setCellWidget(row, 0, label(QString::fromStdString(t->getPersonID()), 12, true, BLUE));
        {
            QString abg, afg;
            avatarPalette(toQ(t->getFullName()), abg, afg);
            tutorTable->setCellWidget(row, 1, userAvatarItem(initialsOf(toQ(t->getFullName())), toQ(t->getFullName()),
                                                             abg, afg, QString::fromUtf8("📞  ") + toQ(t->getPhone())));
        }
        {
            QStringList subs;
            Vector<string>& tv = t->getSubjects();
            for (size_t k = 0; k < tv.size(); ++k) if (!tv[k].empty()) subs << toQ(tv[k]);
            tutorTable->setCellWidget(row, 2, subjectPills(subs));
        }
        tutorTable->setCellWidget(row, 3, label(QString::number(t->getYearsOfExperience()) + QString::fromUtf8(" năm"), 12, false, TEXT_MUTED));
        tutorTable->setCellWidget(row, 4, label(QString::fromStdString(t->getRatePerHour()) + QString::fromUtf8("đ/giờ"), 12, true, BLUE));
        tutorTable->setCellWidget(row, 5, label(QString::fromStdString(t->getAvailability()), 12, false, TEXT_MAIN));
        tutorTable->setCellWidget(row, 6, label(joinStrings(t->getTeachingAreas()), 12, false, TEXT_MUTED));

        tutorTable->setCellWidget(row, 7, pillBadge(
            ClassService::canReceiveStudents(*t) ? QString::fromUtf8("Sẵn sàng") : QString::fromUtf8("Bận"),
            ClassService::canReceiveStudents(*t) ? GREEN_BG : RED_BG,
            ClassService::canReceiveStudents(*t) ? GREEN : RED, true));

        auto* actW = new QWidget;
        auto* actL = new QHBoxLayout(actW);
        actL->setContentsMargins(4, 2, 4, 2);
        actL->setSpacing(4);

        auto* btnView = new QPushButton(QString::fromUtf8("👁"), actW);
        btnView->setToolTip(QString::fromUtf8("Xem chi tiết"));
        btnView->setFixedSize(32, 32);
        btnView->setStyleSheet("QPushButton { background:#E6EEFF; border:none; border-radius:16px; color:#2F8CFF; font-size:12px; } QPushButton:hover { background:#E1E8F8; }");

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#F3F6FF; border:none; border-radius:16px; color:#2458E6; font-size:12px; } QPushButton:hover { background:#E6EEFF; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#D64545; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

        auto* btnAvailability = new QPushButton(QString::fromUtf8("⇄"), actW);
        btnAvailability->setToolTip(t->getIsAvailable()
            ? QString::fromUtf8("Tạm dừng nhận học sinh mới")
            : QString::fromUtf8("Mở lại nhận học sinh"));
        btnAvailability->setFixedSize(32, 32);
        btnAvailability->setStyleSheet("QPushButton { background:#E6F8F2;border:none;"
            "border-radius:16px;color:#008B70;font-size:14px; }"
            "QPushButton:hover {background:#C8EFE3;}");
        actL->addWidget(btnView);
        actL->addWidget(btnEdit);
        actL->addWidget(btnAvailability);
        actL->addWidget(btnDel);
        actL->addStretch();
        tutorTable->setCellWidget(row, 8, actW);
        connect(btnAvailability, &QPushButton::clicked, this, [this,t]() {
            Tutor proposed = *t;
            proposed.setIsAvailable(!t->getIsAvailable());
            string error;
            if (!ClassService(students,tutors,classes).updateTutor(*t,proposed,error)) {
                QMessageBox::warning(this,QString::fromUtf8("Không thể đổi trạng thái"),QString::fromStdString(error));
                return;
            }
            currentMatchedStudent=nullptr;
            currentMatchedTutor=nullptr;
            refreshAll();
        });
        connect(btnView, &QPushButton::clicked, this, [this, t]() { onViewTutor(t); });
        connect(btnEdit, &QPushButton::clicked, this, [this, t]() { onEditTutor(t); });
        connect(btnDel, &QPushButton::clicked, this, [this, t]() { onDeleteTutor(t); });
    }
    fitColumns(tutorTable);
}

// -------------------------------------------------------------
// 3. TÌM GIA SƯ THÔNG MINH (SMART MATCHING)
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
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setObjectName("pageContent");
    content->setStyleSheet(QString("QWidget#pageContent { background:%1; }").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(3);
    tCol->addWidget(label(QString::fromUtf8("Tìm Gia Sư Thông Minh <span style='color:#B48CFF'>✦</span>"), 24, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Hệ thống tự động phân tích và đề xuất gia sư phù hợp nhất dựa trên yêu cầu của học sinh và hồ sơ năng lực của gia sư."), 13, false, TEXT_MUTED));
    l->addLayout(tCol);

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
    scHead->addWidget(scTitle);
    scHead->addStretch();
    scLayout->addLayout(scHead);

    matchStudentCombo = new QComboBox(studentCard);
    matchStudentCombo->setFixedHeight(40);
    matchStudentCombo->setStyleSheet(
        "QComboBox { background:#F6F8FF; border:1px solid #CBD5F0; border-radius:8px; "
        "padding:0 12px; font-size:13px; font-weight:600; color:#0F1E4A; } "
        "QComboBox::drop-down { border:none; width:28px; }");
    connect(matchStudentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateMatchingStudentReqs);
    scLayout->addWidget(matchStudentCombo);
    scLayout->addWidget(label(QString::fromUtf8("Môn cần tìm gia sư:"), 11, true, TEXT_MUTED));
    matchSubjectCombo = new QComboBox(studentCard);
    matchSubjectCombo->setFixedHeight(38);
    matchSubjectCombo->setStyleSheet(matchStudentCombo->styleSheet());
    connect(matchSubjectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) {
        clearMatchingCandidates();
        if (matchReqSubject && matchSubjectCombo)
            matchReqSubject->setText(matchSubjectCombo->currentData().toString().isEmpty()
                ? matchSubjectCombo->currentText()
                : matchSubjectCombo->currentData().toString());
    });
    scLayout->addWidget(matchSubjectCombo);

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
    t1->setStyleSheet("background:#F6F8FF; border:1px solid #E1E8F8; border-radius:8px; QLabel { background:transparent; border:none; }");
    auto* t1l = new QVBoxLayout(t1);
    t1l->setContentsMargins(12, 10, 12, 10);
    t1l->addWidget(label(QString::fromUtf8("MÔN HỌC & LỚP"), 10, true, TEXT_MUTED));
    auto* t1v = new QHBoxLayout;
    matchReqSubject = label(QString::fromUtf8("Toán Học"), 13, true, TEXT_MAIN);
    matchReqGrade = new QLabel(QString::fromUtf8("Lớp 10"), t1);
    matchReqGrade->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border-radius:4px; padding:2px 6px; font-size:10px; font-weight:700;");
    t1v->addWidget(matchReqSubject);
    t1v->addWidget(matchReqGrade);
    t1v->addStretch();
    t1l->addLayout(t1v);
    tilesRow->addWidget(t1, 1);

    // Tile 2: Khu vực
    auto* t2 = new QFrame(reqCard);
    t2->setStyleSheet("background:#F6F8FF; border:1px solid #E1E8F8; border-radius:8px; QLabel { background:transparent; border:none; }");
    auto* t2l = new QVBoxLayout(t2);
    t2l->setContentsMargins(12, 10, 12, 10);
    t2l->addWidget(label(QString::fromUtf8("KHU VỰC"), 10, true, TEXT_MUTED));
    auto* t2v = new QHBoxLayout;
    auto* pin = new QLabel(QString::fromUtf8("📍"), t2);
    pin->setStyleSheet("color:#D64545; font-size:12px;");
    matchReqArea = label(QString::fromUtf8("Hải Châu"), 13, true, TEXT_MAIN);
    t2v->addWidget(pin);
    t2v->addWidget(matchReqArea);
    t2v->addStretch();
    t2l->addLayout(t2v);
    tilesRow->addWidget(t2, 1);

    // Tile 3: Ngân sách
    auto* t3 = new QFrame(reqCard);
    t3->setStyleSheet("background:#F6F8FF; border:1px solid #E1E8F8; border-radius:8px; QLabel { background:transparent; border:none; }");
    auto* t3l = new QVBoxLayout(t3);
    t3l->setContentsMargins(12, 10, 12, 10);
    t3l->addWidget(label(QString::fromUtf8("NGÂN SÁCH (BUỔI)"), 10, true, TEXT_MUTED));
    matchReqBudget = label(QString::fromUtf8("200,000đ"), 14, true, BLUE);
    t3l->addWidget(matchReqBudget);
    tilesRow->addWidget(t3, 1);

    // Tile 4: Lịch học yêu cầu
    auto* t4 = new QFrame(reqCard);
    t4->setStyleSheet("background:#F6F8FF; border:1px solid #E1E8F8; border-radius:8px; QLabel { background:transparent; border:none; }");
    auto* t4l = new QVBoxLayout(t4);
    t4l->setContentsMargins(12, 10, 12, 10);
    t4l->addWidget(label(QString::fromUtf8("LỊCH HỌC YÊU CẦU"), 10, true, TEXT_MUTED));

    matchReqScheduleContainer = new QWidget(t4);
    auto* daysLayout = new QHBoxLayout(matchReqScheduleContainer);
    daysLayout->setSpacing(4);
    daysLayout->setContentsMargins(0, 0, 0, 0);

    const QStringList dArr = {"T2", "T3", "T4", "T5", "T6", "T7", "CN"};
    for (const auto& item : dArr) {
        auto* dl = new QLabel(item, matchReqScheduleContainer);
        dl->setAlignment(Qt::AlignCenter);
        dl->setFixedSize(24, 20);
        dl->setObjectName("day_" + item);
        dl->setStyleSheet("background:#E1E8F8; color:#9AA6C8; border-radius:3px; font-size:10px; font-weight:600;");
        daysLayout->addWidget(dl);
    }
    daysLayout->addStretch();
    t4l->addWidget(matchReqScheduleContainer);
    tilesRow->addWidget(t4, 1);

    rcLayout->addLayout(tilesRow);
    reqRow->addWidget(reqCard, 1);
    l->addLayout(reqRow);

    // Center Action Button
    auto* findBtn = new QPushButton(QString::fromUtf8("↺   TÌM GIA SƯ PHÙ HỢP"), content);
    findBtn->setFixedSize(250, 44);
    findBtn->setCursor(Qt::PointingHandCursor);
    findBtn->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:none; border-radius:22px; "
        "font-size:13px; font-weight:700; letter-spacing:0.5px; } QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0); }");
    connect(findBtn, &QPushButton::clicked, this, &MainWindow::runSmartMatching);
    l->addWidget(findBtn, 0, Qt::AlignCenter);

    // Matching Results Section Header
    auto* resHeader = new QHBoxLayout;
    resHeader->addWidget(label(QString::fromUtf8("💫  Kết Quả Matching"), 16, true, TEXT_MAIN));
    resHeader->addStretch();
    resHeader->addWidget(pillBadge(QString::fromUtf8("Đề xuất tối ưu theo thuật toán PBL2"), "#EEF2FC", "#7F8BB0"));
    l->addLayout(resHeader);

    // All eligible candidates are shown in descending score order. The table
    // intentionally cannot be re-sorted: the displayed rank is always reliable.
    matchCandidatesSummary = label(QString::fromUtf8("Chọn học sinh, môn học và nhấn Tìm gia sư để xem danh sách."),
                                   12, false, TEXT_MUTED);
    l->addWidget(matchCandidatesSummary);
    matchCandidatesTable = new QTableWidget(content);
    matchCandidatesTable->setObjectName("matchCandidatesTable");
    matchCandidatesTable->setColumnCount(6);
    matchCandidatesTable->setHorizontalHeaderLabels({
        QString::fromUtf8("Hạng"), QString::fromUtf8("Gia sư"),
        QString::fromUtf8("Điểm /100"), QString::fromUtf8("Học phí/giờ"),
        QString::fromUtf8("Kinh nghiệm"), QString::fromUtf8("Khu vực dạy")});
    matchCandidatesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    matchCandidatesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    matchCandidatesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    matchCandidatesTable->setSortingEnabled(false);
    matchCandidatesTable->setShowGrid(false);
    matchCandidatesTable->setAlternatingRowColors(true);
    matchCandidatesTable->setFocusPolicy(Qt::StrongFocus);
    matchCandidatesTable->verticalHeader()->setVisible(false);
    matchCandidatesTable->verticalHeader()->setDefaultSectionSize(42);
    matchCandidatesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    matchCandidatesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    matchCandidatesTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    matchCandidatesTable->setStyleSheet(
        "QTableWidget { background:#FFFFFF; alternate-background-color:#F5F8FF; "
        "border:1px solid #DBE5F7; border-radius:9px; color:#172554; } "
        "QTableWidget::item { padding:4px 8px; } "
        "QTableWidget::item:selected { background:#DDEAFF; color:#0F2880; } "
        "QHeaderView::section { background:#EAF1FF; color:#183B8C; border:0; "
        "padding:9px; font-weight:700; }");
    matchCandidatesTable->setVisible(false);
    connect(matchCandidatesTable, &QTableWidget::currentCellChanged, this,
            [this](int row, int, int, int) { selectMatchingCandidate(row); });
    l->addWidget(matchCandidatesTable);

    // Result Details: Two Columns
    auto* matchBody = new QHBoxLayout;
    matchBody->setSpacing(14);

    // Left Candidate Card (Dark Theme)
    auto* candidateCard = new QFrame(content);
    candidateCard->setFixedWidth(330);
    candidateCard->setStyleSheet("background:#F3F6FF; border:1px solid #C9D8FF; border-radius:12px; QLabel { background:transparent; border:none; }");
    auto* candLayout = new QVBoxLayout(candidateCard);
    candLayout->setContentsMargins(20, 20, 20, 20);
    candLayout->setSpacing(10);
    candLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    candidateRankBadge = new QLabel(QString::fromUtf8("CHƯA CHỌN"), candidateCard);
    candidateRankBadge->setAlignment(Qt::AlignCenter);
    candidateRankBadge->setMinimumSize(86, 26);
    candidateRankBadge->setStyleSheet("background:#EAB308; color:#0F1E4A; font-size:11px; font-weight:800; border-radius:4px; padding:0 8px;");
    candLayout->addWidget(candidateRankBadge, 0, Qt::AlignLeft);

    auto* tutorPhoto = new QLabel(QString::fromUtf8("👨‍🏫"), candidateCard);
    tutorPhoto->setAlignment(Qt::AlignCenter);
    tutorPhoto->setFixedSize(90, 90);
    tutorPhoto->setStyleSheet(
        "background:#FFFFFF; border:2px solid #9FC4FF; border-radius:12px; font-size:44px;");
    candLayout->addWidget(tutorPhoto, 0, Qt::AlignCenter);

    candidateId = new QLabel("--", candidateCard);
    candidateId->setAlignment(Qt::AlignCenter);
    candidateId->setFixedSize(65, 22);
    candidateId->setStyleSheet("background:#E6EEFF; color:#1E4FD0; border-radius:11px; font-size:10px; font-weight:700;");
    candLayout->addWidget(candidateId, 0, Qt::AlignCenter);

    candidateName = new QLabel(QString::fromUtf8("Chưa chọn gia sư"), candidateCard);
    candidateName->setAlignment(Qt::AlignCenter);
    candidateName->setStyleSheet("color:#0F1E4A; font-size:18px; font-weight:800; background:transparent;");
    candLayout->addWidget(candidateName);

    candidateDegree = new QLabel(QString::fromUtf8("🎓 Đang chờ tìm kiếm"), candidateCard);
    candidateDegree->setAlignment(Qt::AlignCenter);
    candidateDegree->setStyleSheet("color:#2458E6; font-size:12px; font-weight:500; background:transparent;");
    candLayout->addWidget(candidateDegree);

    candLayout->addSpacing(8);

    candidateDonutContainer = new QWidget(candidateCard);
    auto* donL = new QVBoxLayout(candidateDonutContainer);
    donL->setContentsMargins(0, 0, 0, 0);
    auto* donut = new DonutChartWidget(0, QColor("#2F8CFF"), QColor("#E6EEFF"), "0%", QString::fromUtf8("ĐỘ PHÙ HỢP"), false, candidateDonutContainer);
    donL->addWidget(donut, 0, Qt::AlignCenter);
    candLayout->addWidget(candidateDonutContainer, 0, Qt::AlignCenter);

    candLayout->addSpacing(6);
    auto* botNote = new QLabel(QString::fromUtf8("Được đề xuất tự động từ thuật toán"), candidateCard);
    botNote->setAlignment(Qt::AlignCenter);
    botNote->setStyleSheet("color:#7F8BB0; font-size:11px; background:transparent;");
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

    auto makeStatBox = [](const QString& t, QLabel*& valOut, const QString& initialVal, const QString& valColor = TEXT_MAIN) {
        auto* b = new QFrame;
        b->setStyleSheet("background:#F6F8FF; border:1px solid #E1E8F8; border-radius:8px; QLabel { background:transparent; border:none; }");
        auto* l = new QVBoxLayout(b);
        l->setContentsMargins(14, 10, 14, 10);
        l->addWidget(label(t, 10, true, TEXT_MUTED));
        valOut = label(initialVal, 15, true, valColor);
        l->addWidget(valOut);
        return b;
    };

    grid4->addWidget(makeStatBox(QString::fromUtf8("KINH NGHIỆM"), candidateExp, "-"));
    grid4->addWidget(makeStatBox(QString::fromUtf8("KHU VỰC"), candidateArea, "-"));
    grid4->addWidget(makeStatBox(QString::fromUtf8("HỌC PHÍ"), candidateFee, "-", BLUE));
    grid4->addWidget(makeStatBox(QString::fromUtf8("LỊCH TRỐNG"), candidateSchedule, "-"));
    mrLayout->addLayout(grid4);

    mrLayout->addWidget(label(QString::fromUtf8("CHI TIẾT ĐIỂM MATCHING"), 12, true, TEXT_MUTED));

    candidateScoreBreakdownContainer = new QWidget(matchRight);
    auto* matchGrid = new QGridLayout(candidateScoreBreakdownContainer);
    matchGrid->setContentsMargins(0, 0, 0, 0);
    matchGrid->setHorizontalSpacing(24);
    matchGrid->setVerticalSpacing(10);

    auto addScoreRow = [matchGrid](int row, int col, const QString& title, const QString& scoreId, const QString& score, int val) {
        auto* block = new QVBoxLayout;
        block->setSpacing(4);
        auto* top = new QHBoxLayout;
        top->addWidget(label(title, 12, false, TEXT_MAIN));
        top->addStretch();
        auto* scLbl = label(score, 12, true, BLUE);
        scLbl->setObjectName(scoreId);
        top->addWidget(scLbl);
        block->addLayout(top);
        auto* pb = progress(val, BLUE, 6);
        pb->setObjectName("pb_" + scoreId);
        block->addWidget(pb);
        matchGrid->addLayout(block, row, col);
    };

    addScoreRow(0, 0, QString::fromUtf8("Kinh nghiệm chuyên môn"), "score_exp", "0/25", 0);
    addScoreRow(0, 1, QString::fromUtf8("Khu vực địa lý"), "score_loc", "0/10", 0);
    addScoreRow(1, 0, QString::fromUtf8("Học phí yêu cầu"), "score_price", "0/45", 0);
    addScoreRow(1, 1, QString::fromUtf8("Lịch học trùng khớp"), "score_sched", "0/20", 0);

    mrLayout->addWidget(candidateScoreBreakdownContainer);

    // "VÌ SAO GIA SƯ NÀY PHÙ HỢP?" Card
    candidateWhyContainer = new QFrame(matchRight);
    candidateWhyContainer->setStyleSheet("background:#F6F8FF; border:1px solid #E1E8F8; border-radius:8px; QLabel { background:transparent; border:none; }");
    auto* wl = new QVBoxLayout(candidateWhyContainer);
    wl->setContentsMargins(14, 12, 14, 12);
    wl->setSpacing(8);

    auto* whyTitle = new QHBoxLayout;
    auto* bulb = new QLabel(QString::fromUtf8("💡"), candidateWhyContainer);
    bulb->setStyleSheet("color:#2458E6; font-size:14px;");
    whyTitle->addWidget(bulb);
    whyTitle->addWidget(label(QString::fromUtf8("VÌ SAO GIA SƯ NÀY PHÙ HỢP?"), 11, true, BLUE));
    whyTitle->addStretch();
    wl->addLayout(whyTitle);

    auto* why1 = label(QString(), 12, false, TEXT_MAIN);
    why1->setObjectName("why_1");
    auto* why2 = label(QString(), 12, false, TEXT_MAIN);
    why2->setObjectName("why_2");
    auto* why3 = label(QString(), 12, false, TEXT_MAIN);
    why3->setObjectName("why_3");
    wl->addWidget(why1);
    wl->addWidget(why2);
    wl->addWidget(why3);

    mrLayout->addWidget(candidateWhyContainer);

    // Action buttons
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();

    btnAcceptCandidate = new QPushButton(QString::fromUtf8("🤝  CHỌN GIA SƯ NÀY & TẠO LỚP"), matchRight);
    btnAcceptCandidate->setFixedHeight(40);
    btnAcceptCandidate->setCursor(Qt::PointingHandCursor);
    btnAcceptCandidate->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:none; border-radius:6px; "
        "padding:0 24px; font-size:12px; font-weight:700; } QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0); }");
    connect(btnAcceptCandidate, &QPushButton::clicked, this, &MainWindow::acceptCandidateMatch);
    btnAcceptCandidate->setEnabled(false);
    btnRow->addWidget(btnAcceptCandidate);
    mrLayout->addLayout(btnRow);

    matchBody->addWidget(matchRight, 1);
    l->addLayout(matchBody);

    l->addStretch();
    scroll->setWidget(content);
    content->setAutoFillBackground(false);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::updateMatchingStudentReqs(int studentIndex) {
    clearMatchingCandidates();
    if (studentIndex < 0 || studentIndex >= static_cast<int>(students.size())) {
        if (matchSubjectCombo) matchSubjectCombo->clear();
        return;
    }
    Student* s = students[studentIndex];
    if (matchSubjectCombo) {
        matchSubjectCombo->blockSignals(true);
        matchSubjectCombo->clear();
        matchSubjectCombo->addItem(QString::fromUtf8("Tất cả môn đã đăng ký"), QString());
        Vector<string>& subjects = s->getNeedSubjects();
        for (size_t i = 0; i < subjects.size(); ++i) {
            QString subject = QString::fromStdString(subjects[i]);
            matchSubjectCombo->addItem(subject, subject);
        }
        matchSubjectCombo->setCurrentIndex(0);
        matchSubjectCombo->blockSignals(false);
    }
    if (matchReqSubject) matchReqSubject->setText(joinStrings(s->getNeedSubjects()));
    if (matchReqGrade) matchReqGrade->setText(QString::fromStdString(s->getGrade()));
    if (matchReqArea) matchReqArea->setText(QString::fromStdString(s->getAddress()));
    if (matchReqBudget) matchReqBudget->setText(QString::fromStdString(s->getBudgetPerHour()) + QString::fromUtf8("đ/giờ"));

    if (matchReqScheduleContainer) {
        QString sched = QString::fromStdString(s->getAvailableSchedule());
        const QStringList dArr = {"T2", "T3", "T4", "T5", "T6", "T7", "CN"};
        for (const auto& item : dArr) {
            auto* dl = matchReqScheduleContainer->findChild<QLabel*>("day_" + item);
            if (dl) {
                bool act = sched.contains(item);
                dl->setStyleSheet(act
                    ? "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border-radius:3px; font-size:10px; font-weight:700;"
                    : "background:#E1E8F8; color:#9AA6C8; border-radius:3px; font-size:10px; font-weight:600;");
            }
        }
    }
}

void MainWindow::clearMatchingCandidates() {
    currentMatchedStudent = nullptr;
    currentMatchedTutor = nullptr;
    currentMatchedSubject.clear();
    matchCandidateResults.clear();
    if (btnAcceptCandidate) btnAcceptCandidate->setEnabled(false);
    if (matchCandidatesTable) {
        matchCandidatesTable->blockSignals(true);
        matchCandidatesTable->clearSelection();
        matchCandidatesTable->setRowCount(0);
        matchCandidatesTable->setVisible(false);
        matchCandidatesTable->blockSignals(false);
    }
    if (matchCandidatesSummary) matchCandidatesSummary->setText(
        QString::fromUtf8("Nhấn Tìm gia sư để xem danh sách được sắp xếp theo độ phù hợp."));
    if (candidateRankBadge) candidateRankBadge->setText(QString::fromUtf8("CHƯA CHỌN"));
    if (candidateName) candidateName->setText(QString::fromUtf8("Chưa chọn gia sư"));
    if (candidateId) candidateId->setText("--");
    if (candidateDegree) candidateDegree->setText(QString::fromUtf8("🎓 Chọn gia sư ở danh sách phía trên"));
    if (candidateExp) candidateExp->setText("-");
    if (candidateArea) candidateArea->setText("-");
    if (candidateFee) candidateFee->setText("-");
    if (candidateSchedule) candidateSchedule->setText("-");
    if (candidateDonutContainer) {
        if (auto* donut = candidateDonutContainer->findChild<DonutChartWidget*>())
            donut->setPercent(0, "0/100");
    }
    if (candidateScoreBreakdownContainer) {
        const char* names[] = {"exp", "loc", "price", "sched"};
        const int denominators[] = {25, 10, 45, 20};
        for (int i = 0; i < 4; ++i) {
            if (auto* text = candidateScoreBreakdownContainer->findChild<QLabel*>(
                    QString("score_%1").arg(names[i])))
                text->setText(QString("0/%1").arg(denominators[i]));
            if (auto* bar = candidateScoreBreakdownContainer->findChild<QProgressBar*>(
                    QString("pb_score_%1").arg(names[i]))) bar->setValue(0);
        }
    }
    if (candidateWhyContainer) {
        for (int i = 1; i <= 3; ++i) {
            if (auto* why = candidateWhyContainer->findChild<QLabel*>(QString("why_%1").arg(i)))
                why->clear();
        }
    }
}

// Select ANY row, not only the best candidate. No class is created until confirmed.
void MainWindow::selectMatchingCandidate(int row) {
    currentMatchedTutor = nullptr;
    if (btnAcceptCandidate) btnAcceptCandidate->setEnabled(false);
    if (!currentMatchedStudent || row < 0 ||
        static_cast<size_t>(row) >= matchCandidateResults.size()) return;

    const MatchResult& candidate = matchCandidateResults[static_cast<size_t>(row)];
    if (!candidate.tutor) return;
    currentMatchedTutor = candidate.tutor;
    if (btnAcceptCandidate) btnAcceptCandidate->setEnabled(true);

    if (candidateRankBadge) candidateRankBadge->setText(QString("TOP %1").arg(row + 1));
    if (candidateName) candidateName->setText(QString::fromStdString(currentMatchedTutor->getFullName()));
    if (candidateName) candidateName->setToolTip(
        QString::fromUtf8("Số điện thoại: %1")
            .arg(QString::fromStdString(currentMatchedTutor->getPhone())));
    if (candidateId) candidateId->setText(QString::fromStdString(currentMatchedTutor->getPersonID()));
    if (candidateDegree) candidateDegree->setText(
        QString::fromUtf8("🎓 ") + QString::fromStdString(currentMatchedTutor->getUniversityOrDegree()));

    const int percent = min(100, max(0, static_cast<int>(candidate.totalScore + 0.5)));
    if (candidateDonutContainer) {
        if (auto* donut = candidateDonutContainer->findChild<DonutChartWidget*>())
            donut->setPercent(percent, QString("%1/100").arg(percent));
    }
    if (candidateExp) candidateExp->setText(
        QString::number(currentMatchedTutor->getYearsOfExperience()) + QString::fromUtf8(" năm"));
    if (candidateArea) candidateArea->setText(joinStrings(currentMatchedTutor->getTeachingAreas()));
    if (candidateFee) candidateFee->setText(
        QString::fromStdString(currentMatchedTutor->getRatePerHour()) + QString::fromUtf8("đ/giờ"));
    if (candidateSchedule) candidateSchedule->setText(
        QString::fromStdString(currentMatchedTutor->getAvailability()));

    if (candidateScoreBreakdownContainer) {
        const double scores[] = {candidate.experienceScore, candidate.locationScore,
                                 candidate.priceScore, candidate.scheduleScore};
        const int maxScores[] = {25, 10, 45, 20};
        const char* keys[] = {"exp", "loc", "price", "sched"};
        for (int i = 0; i < 4; ++i) {
            if (auto* text = candidateScoreBreakdownContainer->findChild<QLabel*>(
                    QString("score_%1").arg(keys[i])))
                text->setText(QString("%1/%2").arg(scores[i], 0, 'f', 1).arg(maxScores[i]));
            if (auto* bar = candidateScoreBreakdownContainer->findChild<QProgressBar*>(
                    QString("pb_score_%1").arg(keys[i])))
                bar->setValue(min(100, max(0, static_cast<int>(
                    scores[i] * 100 / maxScores[i] + 0.5))));
        }
    }
    if (candidateWhyContainer) {
        for (int i = 0; i < 3; ++i) {
            if (auto* reason = candidateWhyContainer->findChild<QLabel*>(
                    QString("why_%1").arg(i + 1)))
                reason->setText(static_cast<size_t>(i) < candidate.reasons.size()
                    ? QString::fromUtf8("• ") + QString::fromStdString(candidate.reasons[static_cast<size_t>(i)])
                    : QString());
        }
    }
}

void MainWindow::runSmartMatching() {
    // Invalidate old search (also when an invalid request or 0 results is returned).
    clearMatchingCandidates();
    if (!matchStudentCombo || matchStudentCombo->currentIndex() < 0 ||
        matchStudentCombo->currentIndex() >= static_cast<int>(students.size())) {
        QMessageBox::warning(this, QString::fromUtf8("Thông báo"),
            QString::fromUtf8("Vui lòng chọn học sinh cần tìm gia sư!"));
        return;
    }

    Student* student = students[matchStudentCombo->currentIndex()];
    ClassService service(students, tutors, classes);
    service.synchronize();
    const string subject = matchSubjectCombo ? matchSubjectCombo->currentData().toString().toStdString() : "";
    Vector<string>& registered = student->getNeedSubjects();
    if (!subject.empty() && service.hasAssignedSubject(student->getPersonID(), subject)) {
        QMessageBox::information(this, QString::fromUtf8("Môn đã được ghép"),
            QString::fromUtf8("Học sinh đã có lớp hiệu lực cho môn %1. Chọn môn khác hoặc xử lý lớp hiện tại.")
                .arg(QString::fromStdString(subject)));
        return;
    }
    if (subject.empty()) {
        for (size_t i = 0; i < registered.size(); ++i) {
            if (service.hasAssignedSubject(student->getPersonID(), registered[i])) {
                QMessageBox::information(this, QString::fromUtf8("Đã ghép một phần môn học"),
                    QString::fromUtf8("Học sinh đã được ghép ít nhất một môn. Vui lòng chọn riêng môn chưa ghép."));
                return;
            }
        }
    }

    Student request = *student;
    if (!subject.empty()) request.setNeedSubjects({subject});
    const Vector<MatchResult> found = MatchingService::matchStudentsToTutors(request, tutors);
    // The backend has already sorted the FULL eligible list by descending score.
    // Keep only tutors without an existing active assignment to this student.
    for (size_t i = 0; i < found.size(); ++i) {
        if (!found[i].tutor || service.isAssignedTo(student->getPersonID(), found[i].tutor->getPersonID()))
            continue;
        const auto freeDays = service.availableSlots(*student, *found[i].tutor);
        if (freeDays.empty()) continue;
        // The existing rank included days already occupied by another class.
        // Recalculate the schedule component from actually free days.
        std::string freeDaysCsv;
        for (size_t k = 0; k < freeDays.size(); ++k) {
            if (k) freeDaysCsv += ",";
            freeDaysCsv += freeDays[k];
        }
        MatchResult adjusted = found[i];
        const double newSchedule = MatchingService::calculateScheduleScore(
            student->getAvailableSchedule(), freeDaysCsv);
        adjusted.totalScore = std::max(0.0, std::min(100.0,
            adjusted.totalScore + (newSchedule - adjusted.scheduleScore) *
                                  (0.85 + 0.15 * adjusted.subjectCoverage)));
        adjusted.scheduleScore = newSchedule;
        adjusted.totalScore = std::round(adjusted.totalScore * 10.0) / 10.0;
        bool replacedScheduleReason = false;
        for (size_t r = 0; r < adjusted.reasons.size(); ++r) {
            if (adjusted.reasons[r].rfind("Trùng ", 0) == 0) {
                adjusted.reasons[r] = "Còn ngày trống: " + freeDaysCsv;
                replacedScheduleReason = true;
            }
        }
        if (!replacedScheduleReason) adjusted.reasons.push_back("Còn ngày trống: " + freeDaysCsv);
        matchCandidateResults.push_back(adjusted);
    }
    mergeSort(matchCandidateResults, [](const MatchResult& a, const MatchResult& b) {
        if (a.totalScore != b.totalScore) return a.totalScore > b.totalScore;
        const auto rateA = TextUtils::toLongLong(a.tutor->getRatePerHour(), 0);
        const auto rateB = TextUtils::toLongLong(b.tutor->getRatePerHour(), 0);
        if (rateA != rateB) return rateA < rateB;
        return a.tutor->getPersonID() < b.tutor->getPersonID();
    });

    if (matchCandidateResults.empty()) {
        if (matchCandidatesSummary) matchCandidatesSummary->setText(
            QString::fromUtf8("Không có gia sư đáp ứng môn học, ngày trống thực tế, học phí và sức chứa."));
        QMessageBox::information(this, QString::fromUtf8("Kết quả tìm kiếm"),
            QString::fromUtf8("Không tìm thấy gia sư phù hợp cho học sinh %1.")
                .arg(QString::fromStdString(student->getFullName())));
        return;
    }

    currentMatchedStudent = student;
    currentMatchedSubject = subject;
    if (matchCandidatesSummary) matchCandidatesSummary->setText(
        QString::fromUtf8("Tìm thấy %1 gia sư phù hợp, xếp từ cao xuống thấp. "
                              "Điểm /100 là điểm xếp hạng, không phải xác suất thành công. Nhấn một dòng để chọn.")
            .arg(static_cast<qulonglong>(matchCandidateResults.size())));
    if (matchCandidatesTable) {
        matchCandidatesTable->blockSignals(true);
        matchCandidatesTable->setRowCount(static_cast<int>(matchCandidateResults.size()));
        for (size_t i = 0; i < matchCandidateResults.size(); ++i) {
            const MatchResult& result = matchCandidateResults[i];
            Tutor* tutor = result.tutor;
            const QString cells[] = {
                QString("#%1").arg(static_cast<qulonglong>(i + 1)),
                QString::fromStdString(tutor->getFullName()) + " (" +
                    QString::fromStdString(tutor->getPersonID()) + ")",
                QString("%1/100").arg(result.totalScore, 0, 'f', 1),
                QString::fromStdString(tutor->getRatePerHour()) + QString::fromUtf8("đ"),
                QString::fromUtf8("%1 năm").arg(tutor->getYearsOfExperience()),
                joinStrings(tutor->getTeachingAreas())
            };
            for (int col = 0; col < 6; ++col) {
                auto* item = new QTableWidgetItem(cells[col]);
                if (col == 0 || col == 2) item->setTextAlignment(Qt::AlignCenter);
                if (col == 2) item->setForeground(QColor("#1769B1"));
                matchCandidatesTable->setItem(static_cast<int>(i), col, item);
            }
        }
        matchCandidatesTable->setFixedHeight(
            47 + 42 * min(static_cast<int>(matchCandidateResults.size()), 6));
        matchCandidatesTable->setVisible(true);
        matchCandidatesTable->blockSignals(false);
        matchCandidatesTable->setCurrentCell(0, 1);
        matchCandidatesTable->selectRow(0);
    }
    // Also update details if currentCellChanged was not emitted by the widget.
    selectMatchingCandidate(0);
}

void MainWindow::acceptCandidateMatch() {
    if (!currentMatchedStudent || !currentMatchedTutor) {
        QMessageBox::warning(this, QString::fromUtf8("Thông báo"),
            QString::fromUtf8("Vui lòng tìm gia sư phù hợp trước khi xác nhận."));
        return;
    }

    ClassService service(students, tutors, classes);
    service.synchronize();
    Student matchingRequest = *currentMatchedStudent;
    if (!currentMatchedSubject.empty()) matchingRequest.setNeedSubjects({currentMatchedSubject});
    if (service.isAssignedTo(currentMatchedStudent->getPersonID(), currentMatchedTutor->getPersonID()) ||
        !MatchingService::meetsMandatoryRequirements(matchingRequest, *currentMatchedTutor)) {
        clearMatchingCandidates();
        QMessageBox::warning(this, QString::fromUtf8("Kết quả không còn hợp lệ"),
            QString::fromUtf8("Gia sư đã được ghép, hết chỗ hoặc điều kiện đã thay đổi. Hãy tìm lại."));
        return;
    }

    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận ghép gia sư"),
        QString::fromUtf8("Ghép gia sư %1 cho học sinh %2 và tạo một lớp học?")
            .arg(QString::fromStdString(currentMatchedTutor->getFullName()),
                 QString::fromStdString(currentMatchedStudent->getFullName())),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    // Choose a genuinely unoccupied day. The class model has only weekdays,
    // not hour ranges, so one day is reserved conservatively.
    const auto available = service.availableSlots(*currentMatchedStudent, *currentMatchedTutor);
    if (available.empty()) {
        clearMatchingCandidates();
        QMessageBox::warning(this, QString::fromUtf8("Lịch đã kín"),
            QString::fromUtf8("Học sinh hoặc gia sư không còn ngày học trống. Hãy tìm lại."));
        return;
    }
    const string schedule = available[0];
    string classID = IdGenerator::nextFor("LH", classes, [](TutoringClass* x) { return x->getClassID(); });
    string today = QDate::currentDate().toString("dd/MM/yyyy").toStdString();
    string subject = currentMatchedStudent->getGrade() + " - " +
        (currentMatchedSubject.empty() ? joinStrings(currentMatchedStudent->getNeedSubjects()).toStdString()
                                       : currentMatchedSubject);
    TutoringClass candidate(classID, currentMatchedStudent->getPersonID(),
                            currentMatchedTutor->getPersonID(), subject, schedule,
                            today, "Đang học", currentMatchedTutor->getRatePerHour(), today, today);
    const QString studentName = QString::fromStdString(currentMatchedStudent->getFullName());
    const QString tutorName = QString::fromStdString(currentMatchedTutor->getFullName());
    string error;
    if (!service.create(candidate, error)) {
        QMessageBox::warning(this, QString::fromUtf8("Không thể ghép gia sư"), QString::fromStdString(error));
        return;
    }
    currentMatchedStudent = nullptr;
    currentMatchedTutor = nullptr;
    currentMatchedSubject.clear();
    if (btnAcceptCandidate) btnAcceptCandidate->setEnabled(false);
    refreshAll();
    QMessageBox::information(this, QString::fromUtf8("Ghép nối thành công"),
        QString::fromUtf8("Đã ghép gia sư %1 cho học sinh %2 và tạo lớp %3.")
            .arg(tutorName, studentName, QString::fromStdString(classID)));
    switchPage(4);
}

// -------------------------------------------------------------
// 4. QUẢN LÝ LỚP HỌC
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
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setObjectName("pageContent");
    content->setStyleSheet(QString("QWidget#pageContent { background:%1; }").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#5CC4A0", "#0891B2");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý lớp học"), 22, true, "#0F1E4A");
    auto* pageDesc = label(QString::fromUtf8("Theo dõi lịch học, học sinh, gia sư và tiến độ lớp."), 13, false, "#7F8BB0");
    tCol->addWidget(pageTitle);
    tCol->addWidget(pageDesc);
    topLayout->addLayout(tCol, 1);
    auto* btnCreateClass = button(QString::fromUtf8("📖⁺  Tạo lớp học"), false, topCard);
    connect(btnCreateClass, &QPushButton::clicked, this, &MainWindow::onAddClass);
    styleBannerButton(btnCreateClass);
    topLayout->addWidget(btnCreateClass);
    topLayout->addWidget(new BookIllustration(topCard));
    l->addWidget(topCard);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Tổng lớp"), classSumTotal, "0", BLUE), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Đang học"), classSumActive, "0", GREEN), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Chờ xếp lịch"), classSumPending, "0", "#F59E0B"), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Cần gia sư"), classSumNeedTutor, "0", RED), 1);
    l->addLayout(summary);

    classIdCombo = new QComboBox;
    classStudentEdit = searchField(QString::fromUtf8("Tên hoặc mã học sinh"));
    classTutorEdit = searchField(QString::fromUtf8("Tên hoặc mã gia sư"));
    classSubjectEdit = searchField(QString::fromUtf8("Môn học"));
    classScheduleEdit = searchField(QString::fromUtf8("Lịch học"));
    classStatusCombo = new QComboBox;
    classStatusCombo->addItems({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đang học"),
                                QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư"),
                                QString::fromUtf8("Hoàn thành")});
    connect(classStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateClassTable(); });
    l->addWidget(createSearchPanel({
        {"ID", classIdCombo}, {QString::fromUtf8("Học sinh"), classStudentEdit},
        {QString::fromUtf8("Gia sư"), classTutorEdit},
        {QString::fromUtf8("Môn học"), classSubjectEdit},
        {QString::fromUtf8("Lịch học"), classScheduleEdit},
        {QString::fromUtf8("Trạng thái"), classStatusCombo}
    }, [this]() { populateClassTable(); }, [this]() { resetClassFilters(); }));

    {
        auto* tabRow = new QHBoxLayout;
        tabRow->setSpacing(10);
        classTabs = statusTabs(tabRow, classStatusCombo, QStringList{QString::fromUtf8("Tất cả"), QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư"), QString::fromUtf8("Hoàn thành")});
        tabRow->addStretch();
        l->addLayout(tabRow);
    }

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    classTable = new QTableWidget(0, 8, tableCard);
    classTable->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ LỚP"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("MÔN"),
        QString::fromUtf8("LỊCH HỌC"),
        QString::fromUtf8("HỌC PHÍ"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(classTable, "#5CC4A0");
    classTable->setMinimumHeight(350);

    tl->addWidget(classTable);
    tl->addWidget(new PagerBar(classTable, QString::fromUtf8("lớp học"), tableCard));
    l->addWidget(tableCard, 1);

    l->addStretch();
    scroll->setWidget(content);
    content->setAutoFillBackground(false);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateClassTable() {
    if (!classTable) return;
    QStringList ids;
    for (size_t i = 0; i < classes.size(); ++i) ids << toQ(classes[i]->getClassID());
    syncIdFilter(classIdCombo, ids);
    const QString idFilter = classIdCombo ? classIdCombo->currentData().toString() : QString();
    const QString studentFilter = classStudentEdit ? classStudentEdit->text().trimmed() : QString();
    const QString tutorFilter = classTutorEdit ? classTutorEdit->text().trimmed() : QString();
    const QString subjFilter = classSubjectEdit ? classSubjectEdit->text().trimmed() : QString();
    const QString scheduleFilter = classScheduleEdit ? classScheduleEdit->text().trimmed() : QString();
    const QString statusFilter = classStatusCombo ? classStatusCombo->currentText() : QString();

    Vector<TutoringClass*> filtered;
    for (size_t i = 0; i < classes.size(); ++i) {
        TutoringClass* c = classes[i];
        if (!idFilter.isEmpty() && toQ(c->getClassID()) != idFilter) continue;
        Student* student = findStudent(c->getStudentID());
        Tutor* tutor = findTutor(c->getTutorID());
        const QString studentInfo = toQ(c->getStudentID()) + " " + (student ? toQ(student->getFullName()) : QString());
        const QString tutorInfo = toQ(c->getTutorID()) + " " + (tutor ? toQ(tutor->getFullName()) : QString());
        if (!studentFilter.isEmpty() && !matchesSearch(studentInfo, studentFilter)) continue;
        if (!tutorFilter.isEmpty() && !matchesSearch(tutorInfo, tutorFilter)) continue;
        if (!subjFilter.isEmpty() && !matchesSearch(toQ(c->getSubject()), subjFilter)) continue;
        if (!scheduleFilter.isEmpty() && !matchesSearch(toQ(c->getSchedule()), scheduleFilter)) continue;
        if (!statusFilter.isEmpty() && statusFilter != QString::fromUtf8("Tất cả trạng thái")) {
            if (QString::fromStdString(c->getStatus()) != statusFilter) continue;
        }

        filtered.push_back(c);
    }

    classTable->setRowCount(static_cast<int>(filtered.size()));
    for (size_t i = 0; i < filtered.size(); ++i) {
        TutoringClass* c = filtered[i];
        int row = static_cast<int>(i);

        classTable->setCellWidget(row, 0, label(QString::fromStdString(c->getClassID()), 12, true, BLUE));

        Student* s = findStudent(c->getStudentID());
        QString sName = s ? QString::fromStdString(s->getFullName()) : QString::fromStdString(c->getStudentID());
        classTable->setCellWidget(row, 1, label(sName, 12, true, TEXT_MAIN));

        Tutor* t = findTutor(c->getTutorID());
        QString tName = t ? QString::fromStdString(t->getFullName()) : (c->getTutorID().empty() ? QString::fromUtf8("Chưa có") : QString::fromStdString(c->getTutorID()));
        classTable->setCellWidget(row, 2, label(tName, 12, false, TEXT_SUBTLE));

        classTable->setCellWidget(row, 3, label(QString::fromStdString(c->getSubject()), 12, false, TEXT_MAIN));
        classTable->setCellWidget(row, 4, label(c->getSchedule().empty() ?
            QString::fromUtf8("Chưa xếp lịch") : QString::fromStdString(c->getSchedule()),
            12, false, TEXT_MUTED));
        classTable->setCellWidget(row, 5, label(QString::fromStdString(c->getRatePerHour()) + QString::fromUtf8("đ/giờ"), 12, true, BLUE));

        QString st = QString::fromStdString(c->getStatus());
        QString bg = st == "Đang học" ? GREEN_BG : (st == "Chờ xếp lịch" ? GRAY_PILL_BG : RED_BG);
        QString fg = st == "Đang học" ? GREEN : (st == "Chờ xếp lịch" ? GRAY_PILL_TEXT : RED);
        classTable->setCellWidget(row, 6, pillBadge(st, bg, fg));

        auto* actW = new QWidget;
        auto* actL = new QHBoxLayout(actW);
        actL->setContentsMargins(4, 2, 4, 2);
        actL->setSpacing(4);

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#F3F6FF; border:none; border-radius:16px; color:#2458E6; font-size:12px; } QPushButton:hover { background:#E6EEFF; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#D64545; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

        actL->addWidget(btnEdit);
        actL->addWidget(btnDel);
        actL->addStretch();
        classTable->setCellWidget(row, 7, actW);

        connect(btnEdit, &QPushButton::clicked, this, [this, c]() { onEditClass(c); });
        connect(btnDel, &QPushButton::clicked, this, [this, c]() { onDeleteClass(c); });
    }
    fitColumns(classTable);
}

// -------------------------------------------------------------
// 6. THỐNG KÊ HỆ THỐNG
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
    scroll->viewport()->setAutoFillBackground(false);
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setObjectName("pageContent");
    content->setStyleSheet(QString("QWidget#pageContent { background:%1; }").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(18);

    auto* titleRow = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("Thống kê hệ thống <span style='color:#B48CFF'>✦</span>"), 24, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Tổng quan dữ liệu hoạt động của nền tảng quản lý gia sư. Các chỉ số được cập nhật theo thời gian thực để hỗ trợ ra quyết định nhanh chóng."), 13, false, TEXT_MUTED));
    titleRow->addLayout(tCol, 1);

    auto* refreshBtn = new QPushButton(QString::fromUtf8("🔄  Làm mới"), content);
    refreshBtn->setFixedHeight(38);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1FC8FF,stop:1 #2F6BFF); color:#FFFFFF; border:none; border-radius:6px; "
        "padding:0 18px; font-size:12px; font-weight:600; } QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #12B4EE,stop:1 #2459F0); }");
    connect(refreshBtn, &QPushButton::clicked, this, [this]() { refreshAll(); });
    titleRow->addWidget(refreshBtn);
    l->addLayout(titleRow);

    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(12);

    auto makeStatCardWithLbl = [](const QString& iconEmoji, const QString& trend, QLabel*& valOut,
                                   const QString& initVal, const QString& sub, const QString& iconBg = "#E6EEFF",
                                   const QString& iconFg = "#2458E6", bool isDark = false) {
        auto* f = new QFrame;
        f->setStyleSheet(isDark
            ? QString("background:#F3F6FF; border:1px solid #C9D8FF; border-radius:12px;")
            : "background:#FFFFFF; border:1px solid #E1E8F8; border-radius:12px;");
        auto* l = new QVBoxLayout(f);
        l->setContentsMargins(16, 14, 16, 14);
        l->setSpacing(8);

        auto* top = new QHBoxLayout;
        auto* i = new QLabel(iconEmoji, f);
        i->setAlignment(Qt::AlignCenter);
        i->setFixedSize(40, 40);
        i->setStyleSheet(QString(
            "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #FFFFFF,stop:1 %1); border:1px solid #FFFFFF; color:%2; border-radius:12px; font-size:16px;").arg(iconBg, iconFg));
        top->addWidget(i);
        top->addStretch();

        if (!trend.isEmpty()) {
            auto* tr = new QLabel(trend, f);
            tr->setAlignment(Qt::AlignCenter);
            tr->setStyleSheet(trend.contains("LIVE")
                ? "background:#D3F4E3; color:#4DB892; border-radius:10px; padding:2px 8px; font-size:10px; font-weight:800;"
                : "background:#EEF2FC; color:#4A587F; border-radius:10px; padding:2px 8px; font-size:10px; font-weight:700;");
            top->addWidget(tr);
        }
        l->addLayout(top);

        valOut = label(initVal, 24, true, TEXT_MAIN);
        auto* desc = label(sub, 12, false, TEXT_MUTED);
        l->addWidget(valOut);
        l->addWidget(desc);
        return f;
    };

    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("👥"), "Dữ liệu", statTotalStudents, "0", QString::fromUtf8("Tổng học sinh"), "#E6EEFF", "#2458E6"), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("👨‍🏫"), "Dữ liệu", statTotalTutors, "0", QString::fromUtf8("Tổng gia sư"), "#E8EBFD", "#6572E3"), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("👤"), QString::fromUtf8("● LIVE"), statAvailTutors, "0", QString::fromUtf8("Gia sư sẵn sàng nhận lớp"), "#D3F4E3", "#5CC4A0", true), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("📖"), "Dữ liệu", statActiveClasses, "0", QString::fromUtf8("Lớp đang hoạt động"), "#E6EEFF", "#2458E6"), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("⚙"), "Tự động", statMatchRate, "0%", QString::fromUtf8("Tỷ lệ ghép thành công"), "#FEF3C7", "#D97706"), 1);

    l->addLayout(metrics);
    auto* contractMetrics = new QHBoxLayout;
    contractMetrics->setSpacing(12);
    contractMetrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("🧾"), "Dữ liệu",
        statContractsCount, "0", QString::fromUtf8("Tổng hợp đồng"), "#E6EEFF", "#2458E6"),1);
    contractMetrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("💰"), "Đã ghi nhận",
        statMonthlyRevenue, "0 đ", QString::fromUtf8("Thực thu tháng hiện tại"), "#D3F4E3", "#259B70"),1);
    l->addLayout(contractMetrics);

    auto* statsBody = new QHBoxLayout;
    statsBody->setSpacing(14);

    auto* matchCard = card();
    matchCard->setFixedWidth(340);
    auto* mcLayout = new QVBoxLayout(matchCard);
    mcLayout->setContentsMargins(20, 18, 20, 18);
    mcLayout->setSpacing(14);

    auto* mcHead = new QHBoxLayout;
    mcHead->addWidget(label(QString::fromUtf8("Tình trạng ghép nối"), 15, true, TEXT_MAIN));
    mcHead->addStretch();
    mcLayout->addLayout(mcHead);

    auto* sRow1 = new QHBoxLayout;
    sRow1->addWidget(label(QString::fromUtf8("●  Lớp đang học"), 13, false, TEXT_MAIN));
    sRow1->addStretch();
    statMatchedClasses = label("0", 14, true, TEXT_MAIN);
    sRow1->addWidget(statMatchedClasses);
    mcLayout->addLayout(sRow1);

    auto* sRow2 = new QHBoxLayout;
    sRow2->addWidget(label(QString::fromUtf8("●  Đang chờ ghép / xếp lịch"), 13, false, TEXT_MUTED));
    sRow2->addStretch();
    statPendingClasses = label("0", 14, true, TEXT_MAIN);
    sRow2->addWidget(statPendingClasses);
    mcLayout->addLayout(sRow2);

    statsBody->addWidget(matchCard);

    // Right Column: Summary note card
    auto* rightCard = card();
    auto* rcL = new QVBoxLayout(rightCard);
    rcL->setContentsMargins(20, 18, 20, 18);
    rcL->setSpacing(10);
    rcL->addWidget(label(QString::fromUtf8("Phân tích hoạt động trung tâm"), 15, true, TEXT_MAIN));
    auto* descNote = label(QString::fromUtf8(
        "Tất cả chỉ số đều được đồng bộ trực tiếp từ các thực thể trong hệ thống (học sinh, gia sư và lớp học). "
        "Khi thực hiện thêm, sửa, xóa hoặc ghép nối ở bất kỳ màn hình nào, dữ liệu được cập nhật tức thời và lưu trữ vĩnh viễn vào các file TXT."), 13, false, TEXT_MUTED);
    descNote->setWordWrap(true);
    rcL->addWidget(descNote);
    rcL->addStretch();
    statsBody->addWidget(rightCard, 1);

    l->addLayout(statsBody);

    auto* chartRow1 = new QHBoxLayout;
    chartRow1->setSpacing(14);
    statChartMatch = new SegmentDonutWidget;
    statChartClass = new SegmentDonutWidget;
    statChartGrade = new BarChartWidget(QColor("#2F8CFF"));
    chartRow1->addWidget(chartCard(QString::fromUtf8("Tình trạng ghép học sinh"),
                                   QString::fromUtf8("Đã ghép so với chưa ghép"), statChartMatch), 1);
    chartRow1->addWidget(chartCard(QString::fromUtf8("Trạng thái lớp học"),
                                   QString::fromUtf8("Phân bổ lớp theo tiến độ"), statChartClass), 1);
    chartRow1->addWidget(chartCard(QString::fromUtf8("Học sinh theo khối lớp"),
                                   QString::fromUtf8("Số học sinh mỗi khối"), statChartGrade), 1);
    l->addLayout(chartRow1);

    auto* chartRow2 = new QHBoxLayout;
    chartRow2->setSpacing(14);
    statChartSubject = new BarChartWidget(QColor("#4AA8FF"));
    statChartMonthly = new LineChartWidget;
    chartRow2->addWidget(chartCard(QString::fromUtf8("Gia sư theo môn dạy"),
                                   QString::fromUtf8("Các môn có nhiều gia sư nhất"), statChartSubject), 1);
    chartRow2->addWidget(chartCard(QString::fromUtf8("Lớp học mới theo tháng"),
                                   QString::fromUtf8("Xu hướng mở lớp theo ngày bắt đầu"), statChartMonthly), 1);
    l->addLayout(chartRow2);
    statChartStudentSubject = new BarChartWidget(QColor("#6572E3"));
    l->addWidget(chartCard(QString::fromUtf8("Học sinh theo môn đăng ký"),
                            QString::fromUtf8("Đếm số học sinh đăng ký mỗi môn"), statChartStudentSubject));
    statChartRevenue = new BarChartWidget(QColor("#2F8CFF"));
    l->addWidget(chartCard(QString::fromUtf8("Doanh thu thực thu theo tháng (triệu VNĐ)"),
                            QString::fromUtf8("Chỉ ghi nhận khoản tiền đã thu, không tính học phí dự kiến"),
                            statChartRevenue));
    l->addStretch();

    scroll->setWidget(content);
    content->setAutoFillBackground(false);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// CẬP NHẬT SỐ LIỆU & BIỂU ĐỒ
// -------------------------------------------------------------
namespace {
void topCounts(const QMap<QString, int>& m, int limit, QStringList& labels, QVector<double>& values) {
    QVector<std::pair<QString, int>> items;
    for (auto it = m.constBegin(); it != m.constEnd(); ++it) items.push_back({it.key(), it.value()});
    std::stable_sort(items.begin(), items.end(),
                     [](const std::pair<QString, int>& a, const std::pair<QString, int>& b) {
                         return a.second > b.second;
                     });
    for (int i = 0; i < items.size() && i < limit; ++i) {
        labels << items[i].first;
        values.push_back(items[i].second);
    }
}
}  // namespace

void MainWindow::refreshDashboardMetrics() {
    int totalS = static_cast<int>(students.size());
    int totalT = static_cast<int>(tutors.size());
    int matched = 0, avail = 0, active = 0;
    for (size_t i = 0; i < students.size(); ++i) if (students[i]->getIsMatched()) ++matched;
    for (size_t i = 0; i < tutors.size(); ++i) if (ClassService::canReceiveStudents(*tutors[i])) ++avail;
    for (size_t i = 0; i < classes.size(); ++i)
        if (toQ(classes[i]->getStatus()) == QString::fromUtf8("Đang học")) ++active;
    int rate = totalS > 0 ? matched * 100 / totalS : 0;

    if (dashTotalStudentsLabel) dashTotalStudentsLabel->setText(QString::number(totalS));
    if (dashTotalTutorsLabel) dashTotalTutorsLabel->setText(QString::number(totalT));
    if (dashActiveClassesLabel) dashActiveClassesLabel->setText(QString::number(active));
    if (dashMatchRateLabel) dashMatchRateLabel->setText(QString("%1%").arg(rate));
    if (dashMatchedStudentsLabel) dashMatchedStudentsLabel->setText(QString::number(matched));
    if (dashUnmatchedStudentsLabel) dashUnmatchedStudentsLabel->setText(QString::number(totalS - matched));
    if (dashAvailTutorsLabel) dashAvailTutorsLabel->setText(QString::number(avail));
    if (dashBusyTutorsLabel) dashBusyTutorsLabel->setText(QString::number(totalT - avail));

    if (dashChartTutorSubject) {
        QMap<QString, int> m;
        for (size_t i = 0; i < tutors.size(); ++i) {
            Vector<string>& subs = tutors[i]->getSubjects();
            for (size_t j = 0; j < subs.size(); ++j) if (!subs[j].empty()) m[toQ(subs[j])]++;
        }
        QStringList lb;
        QVector<double> vl;
        topCounts(m, 5, lb, vl);
        static_cast<BarChartWidget*>(dashChartTutorSubject)->setData(lb, vl);
    }
}

void MainWindow::refreshStatisticsMetrics() {
    int totalS = static_cast<int>(students.size());
    int totalT = static_cast<int>(tutors.size());
    int matched = 0, avail = 0, active = 0, pending = 0, needTutor = 0, other = 0;
    for (size_t i = 0; i < students.size(); ++i) if (students[i]->getIsMatched()) ++matched;
    for (size_t i = 0; i < tutors.size(); ++i) if (ClassService::canReceiveStudents(*tutors[i])) ++avail;
    for (size_t i = 0; i < classes.size(); ++i) {
        QString st = toQ(classes[i]->getStatus());
        if (st == QString::fromUtf8("Đang học")) ++active;
        else if (st == QString::fromUtf8("Chờ xếp lịch")) ++pending;
        else if (st == QString::fromUtf8("Cần gia sư") || classes[i]->getTutorID().empty()) ++needTutor;
        else ++other;
    }
    int rate = totalS > 0 ? matched * 100 / totalS : 0;

    if (statTotalStudents) statTotalStudents->setText(QString::number(totalS));
    if (statTotalTutors) statTotalTutors->setText(QString::number(totalT));
    if (statAvailTutors) statAvailTutors->setText(QString::number(avail));
    if (statActiveClasses) statActiveClasses->setText(QString::number(active));
    if (statMatchRate) statMatchRate->setText(QString("%1%").arg(rate));
    if (statMatchedClasses) statMatchedClasses->setText(QString::number(active));
    if (statPendingClasses) statPendingClasses->setText(QString::number(pending + needTutor));
    if (contractPage) {
        if (statContractsCount) statContractsCount->setText(QString::number(contractPage->contractCount()));
        const QDate today = QDate::currentDate();
        if (statMonthlyRevenue) statMonthlyRevenue->setText(
            QString::number(contractPage->paidThisMonth(today.year(), today.month()))+QString::fromUtf8(" đ"));
        if (statChartRevenue) {
            QStringList labels;
            QVector<double> vals;
            for (int i=11;i>=0;--i) {
                const QDate month=today.addMonths(-i);
                labels << month.toString("MM/yy");
                vals.push_back(contractPage->monthlyRevenue(month.year(),month.month())/1000000.0);
            }
            static_cast<BarChartWidget*>(statChartRevenue)->setData(labels, vals);
        }
    }

    if (statChartMatch) {
        static_cast<SegmentDonutWidget*>(statChartMatch)->setData(
            QStringList{QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")},
            QVector<double>{double(matched), double(totalS - matched)},
            QVector<QColor>{QColor("#2F8CFF"), QColor("#D9E1F6")}, QString::fromUtf8("học sinh"));
    }
    if (statChartClass) {
        static_cast<SegmentDonutWidget*>(statChartClass)->setData(
            QStringList{QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"),
                        QString::fromUtf8("Cần gia sư"), QString::fromUtf8("Khác")},
            QVector<double>{double(active), double(pending), double(needTutor), double(other)},
            QVector<QColor>{QColor("#7FD6B0"), QColor("#F7C768"), QColor("#F29B94"), QColor("#C0CBE8")},
            QString::fromUtf8("lớp học"));
    }
    if (statChartGrade) {
        QMap<QString, int> m;
        for (size_t i = 0; i < students.size(); ++i) m[toQ(students[i]->getGrade()).trimmed()]++;
        QVector<std::pair<QString, int>> items;
        for (auto it = m.constBegin(); it != m.constEnd(); ++it) items.push_back({it.key(), it.value()});
        std::sort(items.begin(), items.end(), [](const std::pair<QString, int>& a, const std::pair<QString, int>& b) {
            return a.first.toInt() < b.first.toInt();
        });
        QStringList lb;
        QVector<double> vl;
        for (int i = 0; i < items.size(); ++i) {
            lb << QString::fromUtf8("Lớp ") + items[i].first;
            vl.push_back(items[i].second);
        }
        static_cast<BarChartWidget*>(statChartGrade)->setData(lb, vl);
    }
    if (statChartStudentSubject) {
        QMap<QString,int> counts;
        for (size_t i=0;i<students.size();++i) {
            const auto& subjects=students[i]->getNeedSubjects();
            for (size_t j=0;j<subjects.size();++j)
                if (!subjects[j].empty()) counts[toQ(subjects[j])]++;
        }
        QStringList labels;
        QVector<double> values;
        topCounts(counts,7,labels,values);
        static_cast<BarChartWidget*>(statChartStudentSubject)->setData(labels,values);
    }
    if (statChartSubject) {
        QMap<QString, int> m;
        for (size_t i = 0; i < tutors.size(); ++i) {
            Vector<string>& subs = tutors[i]->getSubjects();
            for (size_t j = 0; j < subs.size(); ++j) if (!subs[j].empty()) m[toQ(subs[j])]++;
        }
        QStringList lb;
        QVector<double> vl;
        topCounts(m, 6, lb, vl);
        static_cast<BarChartWidget*>(statChartSubject)->setData(lb, vl);
    }
    if (statChartMonthly) {
        QMap<QDate, int> m;
        for (size_t i = 0; i < classes.size(); ++i) {
            QDate d = QDate::fromString(toQ(classes[i]->getStartDate()), "dd/MM/yyyy");
            if (d.isValid()) m[QDate(d.year(), d.month(), 1)]++;
        }
        QStringList lb;
        QVector<double> vl;
        for (auto it = m.constBegin(); it != m.constEnd(); ++it) {
            lb << it.key().toString("MM/yyyy");
            vl.push_back(it.value());
        }
        while (lb.size() > 8) { lb.removeFirst(); vl.remove(0); }
        static_cast<LineChartWidget*>(statChartMonthly)->setData(lb, vl);
    }
}

void MainWindow::refreshTutorSummary() {
    int total = static_cast<int>(tutors.size());
    int avail = 0;
    long long sum = 0;
    int counted = 0;
    for (size_t i = 0; i < tutors.size(); ++i) {
        if (ClassService::canReceiveStudents(*tutors[i])) ++avail;
        bool ok = false;
        long long r = toQ(tutors[i]->getRatePerHour()).trimmed().toLongLong(&ok);
        if (ok) { sum += r; ++counted; }
    }
    if (tutorSumTotal) tutorSumTotal->setText(QString::number(total));
    if (tutorSumAvail) tutorSumAvail->setText(QString::number(avail));
    if (tutorSumBusy) tutorSumBusy->setText(QString::number(total - avail));
    if (tutorSumAvgRate) tutorSumAvgRate->setText(QString::number(counted > 0 ? sum / counted : 0) + QString::fromUtf8("đ"));
}

void MainWindow::refreshClassSummary() {
    int active = 0, pending = 0, need = 0;
    for (size_t i = 0; i < classes.size(); ++i) {
        QString st = toQ(classes[i]->getStatus());
        if (st == QString::fromUtf8("Đang học")) ++active;
        else if (st == QString::fromUtf8("Chờ xếp lịch")) ++pending;
        if (st == QString::fromUtf8("Cần gia sư") || classes[i]->getTutorID().empty()) ++need;
    }
    if (classSumTotal) classSumTotal->setText(QString::number(static_cast<int>(classes.size())));
    if (classSumActive) classSumActive->setText(QString::number(active));
    if (classSumPending) classSumPending->setText(QString::number(pending));
    if (classSumNeedTutor) classSumNeedTutor->setText(QString::number(need));
}

void MainWindow::refreshStudentSummary() {
    int total = static_cast<int>(students.size()), matched = 0, counted = 0;
    long long sum = 0;
    for (size_t i = 0; i < students.size(); ++i) {
        if (students[i]->getIsMatched()) ++matched;
        bool ok = false;
        long long v = toQ(students[i]->getBudgetPerHour()).trimmed().toLongLong(&ok);
        if (ok) { sum += v; ++counted; }
    }
    if (stuSumTotal) stuSumTotal->setText(QString::number(total));
    if (stuSumMatched) stuSumMatched->setText(QString::number(matched));
    if (stuSumUnmatched) stuSumUnmatched->setText(QString::number(total - matched));
    if (stuSumAvgBudget) stuSumAvgBudget->setText(QString::number(counted > 0 ? sum / counted : 0) + QString::fromUtf8("đ"));
}

void MainWindow::refreshOverviewWidgets() {
    int totalS = static_cast<int>(students.size());
    int totalT = static_cast<int>(tutors.size());
    int matched = 0, avail = 0;
    int cActive = 0, cPending = 0, cNeed = 0;
    for (size_t i = 0; i < students.size(); ++i) if (students[i]->getIsMatched()) ++matched;
    for (size_t i = 0; i < tutors.size(); ++i) if (ClassService::canReceiveStudents(*tutors[i])) ++avail;
    for (size_t i = 0; i < classes.size(); ++i) {
        QString st = toQ(classes[i]->getStatus());
        if (st == QString::fromUtf8("Đang học")) ++cActive;
        else if (st == QString::fromUtf8("Chờ xếp lịch")) ++cPending;
        if (st == QString::fromUtf8("Cần gia sư") || classes[i]->getTutorID().empty()) ++cNeed;
    }
    const int vals[3] = {totalS, totalT, cActive};
    for (int i = 0; i < welcomeLabels.size(); ++i) {
        if (welcomeLabels[i]) welcomeLabels[i]->setText(QString::number(vals[i % 3]));
    }
    setTabCounts(stuTabs, {totalS, matched, totalS - matched});
    setTabCounts(tutorTabs, {totalT, avail, totalT - avail});
    int cFinished = 0;
    for (size_t i = 0; i < classes.size(); ++i)
        if (classes[i]->getStatus() == "Hoàn thành") ++cFinished;
    setTabCounts(classTabs, {static_cast<int>(classes.size()), cActive, cPending, cNeed, cFinished});
}

void MainWindow::refreshAll() {
    ClassService(students, tutors, classes).synchronize();
    populateStudentTable();
    populateTutorTable();
    populateClassTable();
    populateDashboardRecentTable();
    if (contractPage) contractPage->refresh();
    refreshDashboardMetrics();
    refreshStatisticsMetrics();
    refreshTutorSummary();
    refreshClassSummary();
    refreshStudentSummary();
    refreshOverviewWidgets();

    if (matchStudentCombo) {
        int prevIdx = matchStudentCombo->currentIndex();
        matchStudentCombo->blockSignals(true);
        matchStudentCombo->clear();
        for (size_t i = 0; i < students.size(); ++i) {
            matchStudentCombo->addItem(QString("%1 - %2").arg(
                QString::fromStdString(students[i]->getPersonID()),
                QString::fromStdString(students[i]->getFullName())));
        }
        if (prevIdx >= 0 && prevIdx < static_cast<int>(students.size())) {
            matchStudentCombo->setCurrentIndex(prevIdx);
        } else if (students.size() > 0) {
            matchStudentCombo->setCurrentIndex(0);
        }
        matchStudentCombo->blockSignals(false);
        updateMatchingStudentReqs(matchStudentCombo->currentIndex());
    }
}

// -------------------------------------------------------------
// CRUD DIALOGS: HỌC SINH
// -------------------------------------------------------------
void MainWindow::onAddStudent() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Thêm học sinh mới"));
    dialog.resize(500, 560);
    dialog.setStyleSheet(dialogStyle());

    auto* form = new QFormLayout(&dialog);
    form->addRow(dialogHeader(dialog.windowTitle()));
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("SV", students, [](Student* x) { return x->getPersonID(); });
    auto* txtId = new QLineEdit(QString::fromStdString(nextId), &dialog);
    txtId->setReadOnly(true);  // mã tự sinh, cố định, không cho sửa
    auto* txtName = new QLineEdit(&dialog);
    auto* txtPhone = new QLineEdit(&dialog);
    auto* txtAddress = new QLineEdit(&dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    auto* txtDob = birthDateField(&dialog);
    auto* cbGrade = new QComboBox(&dialog);
    cbGrade->addItems({"10", "11", "12", "9", "8", "7", "6", "5"});
    auto* txtSubjects = new QLineEdit(&dialog);
    txtSubjects->setPlaceholderText(QString::fromUtf8("Vd: Toán, Lý"));
    auto* txtSchedule = new QLineEdit("T2,T4,T6", &dialog);
    auto* cbAcademic = new QComboBox(&dialog);
    cbAcademic->addItems({QString::fromUtf8("Trung Bình"), QString::fromUtf8("Khá"), QString::fromUtf8("Giỏi")});
    auto* txtBudget = new QLineEdit("200000", &dialog);
    auto* txtNotes = new QLineEdit(&dialog);

    form->addRow(label(QString::fromUtf8("Mã học sinh:")), txtId);
    form->addRow(label(QString::fromUtf8("Họ và tên *:")), txtName);
    form->addRow(label(QString::fromUtf8("Số điện thoại *:")), txtPhone);
    form->addRow(label(QString::fromUtf8("Khu vực / Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh *:")), txtDob);
    form->addRow(label(QString::fromUtf8("Khối lớp:")), cbGrade);
    form->addRow(label(QString::fromUtf8("Môn cần học *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Học lực:")), cbAcademic);
    form->addRow(label(QString::fromUtf8("Ngân sách / giờ:")), txtBudget);
    form->addRow(label(QString::fromUtf8("Ghi chú:")), txtNotes);

    attachLive(txtId, [this](const QString& v) {
        return idError(v.toUpper(), "SV", QString::fromUtf8("Mã học sinh"), [this](const std::string& x) { return findStudent(x) != nullptr; });
    });
    attachLive(txtName, nameError);
    attachLive(txtPhone, phoneError);
    attachLive(txtDob, [](const QString& v) { return dobError(v, 5, 25, QString::fromUtf8("học sinh")); });
    attachLive(txtBudget, [](const QString& v) { return moneyError(v, QString::fromUtf8("Ngân sách")); });
    attachLive(txtSubjects, [](const QString& v) { return requiredError(v, QString::fromUtf8("Môn cần học")); });

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("QPushButton { background:#5B7188; color:white; padding:9px 18px; border:none; border-radius:6px; font-weight:600; } QPushButton:hover { background:#4A5F75; }");btnCancel->setCursor(Qt::PointingHandCursor);
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu học sinh"), &dialog);
    btnSave->setStyleSheet(QString("QPushButton { background:%1; color:%2; padding:9px 18px; border:none; border-radius:6px; font-weight:700; } QPushButton:hover { background:%3; }").arg(GRADIENT, ON_ACCENT, GRADIENT_HOVER));btnSave->setCursor(Qt::PointingHandCursor);
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {

        FormCheck fc;
        fc.check(txtId, idError(txtId->text().trimmed().toUpper(), "SV", QString::fromUtf8("Mã học sinh"),
                                [this](const std::string& x) { return findStudent(x) != nullptr; }));
        fc.check(txtName, nameError(txtName->text().trimmed()));
        fc.check(txtPhone, phoneError(txtPhone->text().trimmed()));
        fc.check(txtDob, dobError(txtDob->text().trimmed(), 5, 25, QString::fromUtf8("học sinh")));
        fc.check(txtSubjects, subjectInputError(txtSubjects->text().trimmed(), QString::fromUtf8("Môn cần học")));
        fc.check(txtSchedule, scheduleInputError(txtSchedule->text().trimmed()));
        fc.check(txtBudget, moneyError(txtBudget->text().trimmed(), QString::fromUtf8("Ngân sách")));
        if (!fc.report(&dialog)) return;
        txtId->setText(txtId->text().trimmed().toUpper());

        Vector<string> subList = FileManager::splitString(txtSubjects->text().toStdString(), ',');
        auto* s = new Student(
            txtId->text().trimmed().toStdString(),
            txtName->text().trimmed().toStdString(),
            txtPhone->text().trimmed().toStdString(),
            txtAddress->text().trimmed().toStdString(),
            cbGender->currentText().toStdString(),
            txtDob->text().trimmed().toStdString(),
            cbGrade->currentText().toStdString(),
            subList,
            txtSchedule->text().trimmed().toStdString(),
            cbAcademic->currentText().toStdString(),
            "Khong",
            txtBudget->text().trimmed().toStdString(),
            txtNotes->text().trimmed().toStdString(),
            false
        );
        students.push_back(s);
        if (!FileManager::saveAll(students, tutors, classes)) {
            students.pop_back();
            delete s;
            QMessageBox::critical(&dialog, QString::fromUtf8("Lỗi lưu dữ liệu"),
                QString::fromUtf8("Không thể lưu học sinh. Dữ liệu cũ không bị thay đổi."));
            return;
        }
        refreshAll();
        auto answer = QMessageBox::question(&dialog, QString::fromUtf8("Lưu thành công"),
            QString::fromUtf8("Đã lưu học sinh. Bạn có muốn nhập tiếp không?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        dialog.done(answer == QMessageBox::Yes ? 2 : QDialog::Accepted);
    });

    const int result = dialog.exec();
    if (result == 2) { onAddStudent(); return; }
    if (result == QDialog::Accepted) focusNewRecord(1, studentTable, txtId->text().trimmed());
}

void MainWindow::onEditStudent(Student* s) {
    if (!s) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa học sinh: ") + QString::fromStdString(s->getFullName()));
    dialog.resize(500, 560);
    dialog.setStyleSheet(dialogStyle());

    auto* form = new QFormLayout(&dialog);
    form->addRow(dialogHeader(dialog.windowTitle()));
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(s->getPersonID()), &dialog);
    txtId->setEnabled(false);
    auto* txtName = new QLineEdit(QString::fromStdString(s->getFullName()), &dialog);
    auto* txtPhone = new QLineEdit(QString::fromStdString(s->getPhoneNumber()), &dialog);
    auto* txtAddress = new QLineEdit(QString::fromStdString(s->getAddress()), &dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    cbGender->setCurrentText(QString::fromStdString(s->getGender()));
    auto* txtDob = birthDateField(&dialog, QString::fromStdString(s->getDateOfBirth()));
    auto* cbGrade = new QComboBox(&dialog);
    cbGrade->addItems({"10", "11", "12", "9", "8", "7", "6", "5"});
    cbGrade->setCurrentText(QString::fromStdString(s->getGrade()));
    auto* txtSubjects = new QLineEdit(joinStrings(s->getNeedSubjects()), &dialog);
    auto* txtSchedule = new QLineEdit(QString::fromStdString(s->getAvailableSchedule()), &dialog);
    auto* cbAcademic = new QComboBox(&dialog);
    cbAcademic->addItems({QString::fromUtf8("Trung Bình"), QString::fromUtf8("Khá"), QString::fromUtf8("Giỏi")});
    cbAcademic->setCurrentText(QString::fromStdString(s->getAcademicPerformance()));
    auto* txtBudget = new QLineEdit(QString::fromStdString(s->getBudgetPerHour()), &dialog);
    auto* txtNotes = new QLineEdit(QString::fromStdString(s->getNotes()), &dialog);

    form->addRow(label(QString::fromUtf8("Mã học sinh:")), txtId);
    form->addRow(label(QString::fromUtf8("Họ và tên *:")), txtName);
    form->addRow(label(QString::fromUtf8("Số điện thoại *:")), txtPhone);
    form->addRow(label(QString::fromUtf8("Khu vực / Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh *:")), txtDob);
    form->addRow(label(QString::fromUtf8("Khối lớp:")), cbGrade);
    form->addRow(label(QString::fromUtf8("Môn cần học *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Học lực:")), cbAcademic);
    form->addRow(label(QString::fromUtf8("Ngân sách / giờ:")), txtBudget);
    form->addRow(label(QString::fromUtf8("Ghi chú:")), txtNotes);

    attachLive(txtName, nameError);
    attachLive(txtPhone, phoneError);
    attachLive(txtDob, [](const QString& v) { return dobError(v, 5, 25, QString::fromUtf8("học sinh")); });
    attachLive(txtBudget, [](const QString& v) { return moneyError(v, QString::fromUtf8("Ngân sách")); });
    attachLive(txtSubjects, [](const QString& v) { return requiredError(v, QString::fromUtf8("Môn cần học")); });

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("QPushButton { background:#5B7188; color:white; padding:9px 18px; border:none; border-radius:6px; font-weight:600; } QPushButton:hover { background:#4A5F75; }");btnCancel->setCursor(Qt::PointingHandCursor);
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet(QString("QPushButton { background:%1; color:%2; padding:9px 18px; border:none; border-radius:6px; font-weight:700; } QPushButton:hover { background:%3; }").arg(GRADIENT, ON_ACCENT, GRADIENT_HOVER));btnSave->setCursor(Qt::PointingHandCursor);
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        FormCheck fc;
        fc.check(txtName, nameError(txtName->text().trimmed()));
        fc.check(txtPhone, phoneError(txtPhone->text().trimmed()));
        fc.check(txtDob, dobError(txtDob->text().trimmed(), 5, 25, QString::fromUtf8("học sinh")));
        fc.check(txtSubjects, subjectInputError(txtSubjects->text().trimmed(), QString::fromUtf8("Môn cần học")));
        fc.check(txtSchedule, scheduleInputError(txtSchedule->text().trimmed()));
        fc.check(txtBudget, moneyError(txtBudget->text().trimmed(), QString::fromUtf8("Ngân sách")));
        if (!fc.report(&dialog)) return;

        Student proposed = *s;
        proposed.setFullName(txtName->text().trimmed().toStdString());
        proposed.setPhoneNumber(txtPhone->text().trimmed().toStdString());
        proposed.setAddress(txtAddress->text().trimmed().toStdString());
        proposed.setGender(cbGender->currentText().toStdString());
        proposed.setDateOfBirth(txtDob->text().trimmed().toStdString());
        proposed.setGrade(cbGrade->currentText().toStdString());
        proposed.setNeedSubjects(FileManager::splitString(txtSubjects->text().toStdString(), ','));
        proposed.setAvailableSchedule(txtSchedule->text().trimmed().toStdString());
        proposed.setAcademicPerformance(cbAcademic->currentText().toStdString());
        proposed.setBudgetPerHour(txtBudget->text().trimmed().toStdString());
        proposed.setNotes(txtNotes->text().trimmed().toStdString());

        string error;
        if (!ClassService(students, tutors, classes).updateStudent(*s, proposed, error)) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Không thể cập nhật học sinh"),
                                 QString::fromStdString(error));
            return;
        }
        currentMatchedStudent = nullptr;
        currentMatchedTutor = nullptr;
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin học sinh!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteStudent(Student* s) {
    if (!s) return;
    if (!ClassService(students, tutors, classes).canDeleteStudent(s->getPersonID())) {
        QMessageBox::warning(this, QString::fromUtf8("Không thể xóa học sinh"),
            QString::fromUtf8("Học sinh còn lớp học hoặc hợp đồng liên quan. "
                              "Hãy xử lý các quan hệ trước khi xóa để bảo toàn dữ liệu."));
        return;
    }
    if (QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
            QString::fromUtf8("Bạn có chắc muốn xóa học sinh %1 (%2)?")
                .arg(QString::fromStdString(s->getFullName()), QString::fromStdString(s->getPersonID())),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    for (size_t i = 0; i < students.size(); ++i) {
        if (students[i] != s) continue;
        const Vector<Student*> originalOrder = students;
        students.erase(i);
        if (!FileManager::saveAll(students, tutors, classes)) {
            students = originalOrder;
            QMessageBox::critical(this, QString::fromUtf8("Lỗi lưu dữ liệu"),
                QString::fromUtf8("Không thể xóa học sinh. Đã hoàn tác thay đổi."));
            return;
        }
        if (currentMatchedStudent == s) { currentMatchedStudent = nullptr; currentMatchedTutor = nullptr; }
        delete s;
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"),
            QString::fromUtf8("Đã xóa học sinh."));
        return;
    }
}

void MainWindow::onViewStudent(Student* s) {
    if (!s) return;
    QString info = QString::fromUtf8(
        "Mã học sinh: %1\n"
        "Họ và tên: %2\n"
        "Số điện thoại: %3\n"
        "Địa chỉ / Khu vực: %4\n"
        "Giới tính: %5\n"
        "Ngày sinh: %6\n"
        "Khối lớp: %7\n"
        "Môn học cần tìm: %8\n"
        "Lịch học rảnh: %9\n"
        "Học lực: %10\n"
        "Ngân sách: %11 đ/giờ\n"
        "Trạng thái: %12\n"
        "Ghi chú: %13"
    ).arg(
        QString::fromStdString(s->getPersonID()),
        QString::fromStdString(s->getFullName()),
        QString::fromStdString(s->getPhoneNumber()),
        QString::fromStdString(s->getAddress()),
        QString::fromStdString(s->getGender()),
        QString::fromStdString(s->getDateOfBirth()),
        QString::fromStdString(s->getGrade()),
        joinStrings(s->getNeedSubjects()),
        QString::fromStdString(s->getAvailableSchedule()),
        QString::fromStdString(s->getAcademicPerformance()),
        QString::fromStdString(s->getBudgetPerHour()),
        s->getIsMatched() ? QString::fromUtf8("Đã ghép gia sư") : QString::fromUtf8("Chưa ghép"),
        QString::fromStdString(s->getNotes())
    );

    QMessageBox::information(this, QString::fromUtf8("Thông tin chi tiết học sinh"), info);
}

// -------------------------------------------------------------
// CRUD DIALOGS: GIA SƯ
// -------------------------------------------------------------
void MainWindow::onAddTutor() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Thêm gia sư mới"));
    dialog.resize(500, 580);
    dialog.setStyleSheet(dialogStyle());

    auto* form = new QFormLayout(&dialog);
    form->addRow(dialogHeader(dialog.windowTitle()));
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("GS", tutors, [](Tutor* x) { return x->getPersonID(); });
    auto* txtId = new QLineEdit(QString::fromStdString(nextId), &dialog);
    txtId->setReadOnly(true);  // mã tự sinh, cố định, không cho sửa
    auto* txtName = new QLineEdit(&dialog);
    auto* txtPhone = new QLineEdit(&dialog);
    auto* txtAddress = new QLineEdit(&dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    auto* txtDob = birthDateField(&dialog);
    auto* txtSubjects = new QLineEdit(&dialog);
    txtSubjects->setPlaceholderText(QString::fromUtf8("Vd: Toán, Lý"));
    auto* txtUni = new QLineEdit(QString::fromUtf8("Đại Học Sư Phạm"), &dialog);
    auto* spExp = new QSpinBox(&dialog);
    spExp->setRange(0, 40);
    spExp->setValue(3);
    auto* txtFee = new QLineEdit("180000", &dialog);
    auto* txtSchedule = new QLineEdit("T2,T4,T6", &dialog);
    auto* txtAreas = new QLineEdit(QString::fromUtf8("Hải Châu, Thanh Khê"), &dialog);

    form->addRow(label(QString::fromUtf8("Mã gia sư:")), txtId);
    form->addRow(label(QString::fromUtf8("Họ và tên *:")), txtName);
    form->addRow(label(QString::fromUtf8("Số điện thoại *:")), txtPhone);
    form->addRow(label(QString::fromUtf8("Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh *:")), txtDob);
    form->addRow(label(QString::fromUtf8("Môn dạy *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Trình độ / Trường:")), txtUni);
    form->addRow(label(QString::fromUtf8("Số năm kinh nghiệm:")), spExp);
    form->addRow(label(QString::fromUtf8("Học phí / giờ:")), txtFee);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Khu vực dạy:")), txtAreas);

    attachLive(txtId, [this](const QString& v) {
        return idError(v.toUpper(), "GS", QString::fromUtf8("Mã gia sư"), [this](const std::string& x) { return findTutor(x) != nullptr; });
    });
    attachLive(txtName, nameError);
    attachLive(txtPhone, phoneError);
    attachLive(txtDob, [](const QString& v) { return dobError(v, 16, 70, QString::fromUtf8("gia sư")); });
    attachLive(txtFee, [](const QString& v) { return moneyError(v, QString::fromUtf8("Học phí")); });
    attachLive(txtSubjects, [](const QString& v) { return requiredError(v, QString::fromUtf8("Môn dạy")); });

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("QPushButton { background:#5B7188; color:white; padding:9px 18px; border:none; border-radius:6px; font-weight:600; } QPushButton:hover { background:#4A5F75; }");btnCancel->setCursor(Qt::PointingHandCursor);
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu gia sư"), &dialog);
    btnSave->setStyleSheet(QString("QPushButton { background:%1; color:%2; padding:9px 18px; border:none; border-radius:6px; font-weight:700; } QPushButton:hover { background:%3; }").arg(GRADIENT, ON_ACCENT, GRADIENT_HOVER));btnSave->setCursor(Qt::PointingHandCursor);
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {

        FormCheck fc;
        fc.check(txtId, idError(txtId->text().trimmed().toUpper(), "GS", QString::fromUtf8("Mã gia sư"),
                                [this](const std::string& x) { return findTutor(x) != nullptr; }));
        fc.check(txtName, nameError(txtName->text().trimmed()));
        fc.check(txtPhone, phoneError(txtPhone->text().trimmed()));
        fc.check(txtDob, dobError(txtDob->text().trimmed(), 16, 70, QString::fromUtf8("gia sư")));
        fc.check(txtSubjects, subjectInputError(txtSubjects->text().trimmed(), QString::fromUtf8("Môn dạy")));
        fc.check(txtSchedule, scheduleInputError(txtSchedule->text().trimmed()));
        fc.check(txtFee, moneyError(txtFee->text().trimmed(), QString::fromUtf8("Học phí")));
        if (!fc.report(&dialog)) return;
        txtId->setText(txtId->text().trimmed().toUpper());

        Vector<string> subList = FileManager::splitString(txtSubjects->text().toStdString(), ',');
        Vector<string> areaList = FileManager::splitString(txtAreas->text().toStdString(), ',');

        auto* t = new Tutor(
            txtId->text().trimmed().toStdString(),
            txtName->text().trimmed().toStdString(),
            txtPhone->text().trimmed().toStdString(),
            txtAddress->text().trimmed().toStdString(),
            cbGender->currentText().toStdString(),
            txtDob->text().trimmed().toStdString(),
            subList,
            txtUni->text().trimmed().toStdString(),
            spExp->value(),
            "Giang day tot",
            txtFee->text().trimmed().toStdString(),
            txtSchedule->text().trimmed().toStdString(),
            true,
            0,
            {},
            areaList,
            ""
        );

        tutors.push_back(t);
        if (!FileManager::saveAll(students, tutors, classes)) {
            tutors.pop_back();
            delete t;
            QMessageBox::critical(&dialog, QString::fromUtf8("Lỗi lưu dữ liệu"),
                QString::fromUtf8("Không thể lưu gia sư. Dữ liệu cũ không bị thay đổi."));
            return;
        }
        refreshAll();
        auto answer = QMessageBox::question(&dialog, QString::fromUtf8("Lưu thành công"),
            QString::fromUtf8("Đã lưu gia sư. Bạn có muốn nhập tiếp không?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        dialog.done(answer == QMessageBox::Yes ? 2 : QDialog::Accepted);
    });

    const int result = dialog.exec();
    if (result == 2) { onAddTutor(); return; }
    if (result == QDialog::Accepted) focusNewRecord(2, tutorTable, txtId->text().trimmed());
}

void MainWindow::onEditTutor(Tutor* t) {
    if (!t) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa gia sư: ") + QString::fromStdString(t->getFullName()));
    dialog.resize(500, 580);
    dialog.setStyleSheet(dialogStyle());

    auto* form = new QFormLayout(&dialog);
    form->addRow(dialogHeader(dialog.windowTitle()));
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(t->getPersonID()), &dialog);
    txtId->setEnabled(false);
    auto* txtName = new QLineEdit(QString::fromStdString(t->getFullName()), &dialog);
    auto* txtPhone = new QLineEdit(QString::fromStdString(t->getPhoneNumber()), &dialog);
    auto* txtAddress = new QLineEdit(QString::fromStdString(t->getAddress()), &dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    cbGender->setCurrentText(QString::fromStdString(t->getGender()));
    auto* txtDob = birthDateField(&dialog, QString::fromStdString(t->getDateOfBirth()));
    auto* txtSubjects = new QLineEdit(joinStrings(t->getSubjects()), &dialog);
    auto* txtUni = new QLineEdit(QString::fromStdString(t->getUniversityOrDegree()), &dialog);
    auto* spExp = new QSpinBox(&dialog);
    spExp->setRange(0, 40);
    spExp->setValue(t->getYearsOfExperience());
    auto* txtFee = new QLineEdit(QString::fromStdString(t->getRatePerHour()), &dialog);
    auto* txtSchedule = new QLineEdit(QString::fromStdString(t->getAvailability()), &dialog);
    auto* txtAreas = new QLineEdit(joinStrings(t->getTeachingAreas()), &dialog);
    auto* cbAvail = new QComboBox(&dialog);
    cbAvail->addItems({QString::fromUtf8("Sẵn sàng nhận lớp"), QString::fromUtf8("Đang bận")});
    cbAvail->setCurrentIndex(t->getIsAvailable() ? 0 : 1);

    form->addRow(label(QString::fromUtf8("Mã gia sư:")), txtId);
    form->addRow(label(QString::fromUtf8("Họ và tên *:")), txtName);
    form->addRow(label(QString::fromUtf8("Số điện thoại *:")), txtPhone);
    form->addRow(label(QString::fromUtf8("Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh *:")), txtDob);
    form->addRow(label(QString::fromUtf8("Môn dạy *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Trình độ / Trường:")), txtUni);
    form->addRow(label(QString::fromUtf8("Số năm kinh nghiệm:")), spExp);
    form->addRow(label(QString::fromUtf8("Học phí / giờ:")), txtFee);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Khu vực dạy:")), txtAreas);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbAvail);

    attachLive(txtName, nameError);
    attachLive(txtPhone, phoneError);
    attachLive(txtDob, [](const QString& v) { return dobError(v, 16, 70, QString::fromUtf8("gia sư")); });
    attachLive(txtFee, [](const QString& v) { return moneyError(v, QString::fromUtf8("Học phí")); });
    attachLive(txtSubjects, [](const QString& v) { return requiredError(v, QString::fromUtf8("Môn dạy")); });

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("QPushButton { background:#5B7188; color:white; padding:9px 18px; border:none; border-radius:6px; font-weight:600; } QPushButton:hover { background:#4A5F75; }");btnCancel->setCursor(Qt::PointingHandCursor);
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet(QString("QPushButton { background:%1; color:%2; padding:9px 18px; border:none; border-radius:6px; font-weight:700; } QPushButton:hover { background:%3; }").arg(GRADIENT, ON_ACCENT, GRADIENT_HOVER));btnSave->setCursor(Qt::PointingHandCursor);
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        FormCheck fc;
        fc.check(txtName, nameError(txtName->text().trimmed()));
        fc.check(txtPhone, phoneError(txtPhone->text().trimmed()));
        fc.check(txtDob, dobError(txtDob->text().trimmed(), 16, 70, QString::fromUtf8("gia sư")));
        fc.check(txtSubjects, subjectInputError(txtSubjects->text().trimmed(), QString::fromUtf8("Môn dạy")));
        fc.check(txtSchedule, scheduleInputError(txtSchedule->text().trimmed()));
        fc.check(txtFee, moneyError(txtFee->text().trimmed(), QString::fromUtf8("Học phí")));
        if (!fc.report(&dialog)) return;

        Tutor proposed = *t;
        proposed.setFullName(txtName->text().trimmed().toStdString());
        proposed.setPhoneNumber(txtPhone->text().trimmed().toStdString());
        proposed.setAddress(txtAddress->text().trimmed().toStdString());
        proposed.setGender(cbGender->currentText().toStdString());
        proposed.setDateOfBirth(txtDob->text().trimmed().toStdString());
        proposed.setSubjects(FileManager::splitString(txtSubjects->text().toStdString(), ','));
        proposed.setUniversityOrDegree(txtUni->text().trimmed().toStdString());
        proposed.setYearsOfExperience(spExp->value());
        proposed.setRatePerHour(txtFee->text().trimmed().toStdString());
        proposed.setAvailability(txtSchedule->text().trimmed().toStdString());
        proposed.setTeachingAreas(FileManager::splitString(txtAreas->text().toStdString(), ','));
        proposed.setIsAvailable(cbAvail->currentIndex() == 0);

        string error;
        if (!ClassService(students, tutors, classes).updateTutor(*t, proposed, error)) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Không thể cập nhật gia sư"),
                                 QString::fromStdString(error));
            return;
        }
        currentMatchedStudent = nullptr;
        currentMatchedTutor = nullptr;
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin gia sư!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteTutor(Tutor* t) {
    if (!t) return;
    if (!ClassService(students, tutors, classes).canDeleteTutor(t->getPersonID())) {
        QMessageBox::warning(this, QString::fromUtf8("Không thể xóa gia sư"),
            QString::fromUtf8("Gia sư còn lớp học hoặc hợp đồng liên quan. "
                              "Hãy xử lý các quan hệ trước khi xóa để bảo toàn dữ liệu."));
        return;
    }
    if (QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
            QString::fromUtf8("Bạn có chắc muốn xóa gia sư %1 (%2)?")
                .arg(QString::fromStdString(t->getFullName()), QString::fromStdString(t->getPersonID())),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    for (size_t i = 0; i < tutors.size(); ++i) {
        if (tutors[i] != t) continue;
        const Vector<Tutor*> originalOrder = tutors;
        tutors.erase(i);
        if (!FileManager::saveAll(students, tutors, classes)) {
            tutors = originalOrder;
            QMessageBox::critical(this, QString::fromUtf8("Lỗi lưu dữ liệu"),
                QString::fromUtf8("Không thể xóa gia sư. Đã hoàn tác thay đổi."));
            return;
        }
        if (currentMatchedTutor == t) { currentMatchedStudent = nullptr; currentMatchedTutor = nullptr; }
        delete t;
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"),
            QString::fromUtf8("Đã xóa gia sư."));
        return;
    }
}

void MainWindow::onViewTutor(Tutor* t) {
    if (!t) return;
    QString info = QString::fromUtf8(
        "Mã gia sư: %1\n"
        "Họ và tên: %2\n"
        "Số điện thoại: %3\n"
        "Địa chỉ: %4\n"
        "Môn giảng dạy: %5\n"
        "Trường / Bằng cấp: %6\n"
        "Số năm kinh nghiệm: %7 năm\n"
        "Mức phí yêu cầu: %8 đ/giờ\n"
        "Lịch rảnh: %9\n"
        "Khu vực nhận lớp: %10\n"
        "Trạng thái: %11"
    ).arg(
        QString::fromStdString(t->getPersonID()),
        QString::fromStdString(t->getFullName()),
        QString::fromStdString(t->getPhoneNumber()),
        QString::fromStdString(t->getAddress()),
        joinStrings(t->getSubjects()),
        QString::fromStdString(t->getUniversityOrDegree()),
        QString::number(t->getYearsOfExperience()),
        QString::fromStdString(t->getRatePerHour()),
        QString::fromStdString(t->getAvailability()),
        joinStrings(t->getTeachingAreas()),
        ClassService::canReceiveStudents(*t) ? QString::fromUtf8("Sẵn sàng") : QString::fromUtf8("Đang bận")
    );

    QMessageBox::information(this, QString::fromUtf8("Thông tin chi tiết gia sư"), info);
}

// -------------------------------------------------------------
// CRUD DIALOGS: LỚP HỌC
// -------------------------------------------------------------
void MainWindow::onAddClass() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Tạo lớp học mới"));
    dialog.resize(480, 445);
    dialog.setStyleSheet(dialogStyle());

    auto* form = new QFormLayout(&dialog);
    form->addRow(dialogHeader(dialog.windowTitle()));
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("LH", classes, [](TutoringClass* x) { return x->getClassID(); });
    auto* txtId = new QLineEdit(QString::fromStdString(nextId), &dialog);
    auto* cbStudent = new QComboBox(&dialog);
    for (size_t i = 0; i < students.size(); ++i) {
        cbStudent->addItem(QString("%1 - %2").arg(QString::fromStdString(students[i]->getPersonID()), QString::fromStdString(students[i]->getFullName())), QString::fromStdString(students[i]->getPersonID()));
    }

    auto* cbTutor = new QComboBox(&dialog);
    cbTutor->addItem(QString::fromUtf8("-- Chưa có gia sư --"), "");
    for (size_t i = 0; i < tutors.size(); ++i) {
        cbTutor->addItem(QString("%1 - %2").arg(QString::fromStdString(tutors[i]->getPersonID()), QString::fromStdString(tutors[i]->getFullName())), QString::fromStdString(tutors[i]->getPersonID()));
    }

    auto* txtSubject = new QLineEdit(QString::fromUtf8("Toán Lớp 10"), &dialog);
    auto* txtSchedule = new QLineEdit("T2,T4,T6", &dialog);
    auto* txtStart = new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy"), &dialog);
    auto* cbStatus = new QComboBox(&dialog);
    cbStatus->addItems({QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư"), QString::fromUtf8("Hoàn thành")});
    cbStatus->setCurrentText(QString::fromUtf8("Cần gia sư"));
    connect(cbTutor, QOverload<int>::of(&QComboBox::currentIndexChanged),
            cbStatus, [cbTutor, cbStatus](int) {
        if (cbTutor->currentData().toString().isEmpty()) {
            if (cbStatus->currentText() == QString::fromUtf8("Đang học"))
                cbStatus->setCurrentText(QString::fromUtf8("Cần gia sư"));
        } else if (cbStatus->currentText() == QString::fromUtf8("Cần gia sư")) {
            cbStatus->setCurrentText(QString::fromUtf8("Đang học"));
        }
    });
    auto* txtFee = new QLineEdit("200000", &dialog);

    form->addRow(label(QString::fromUtf8("Mã lớp:")), txtId);
    form->addRow(label(QString::fromUtf8("Học sinh *:")), cbStudent);
    form->addRow(label(QString::fromUtf8("Gia sư:")), cbTutor);
    form->addRow(label(QString::fromUtf8("Môn học *:")), txtSubject);
    form->addRow(label(QString::fromUtf8("Lịch học:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Ngày bắt đầu:")), txtStart);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);
    form->addRow(label(QString::fromUtf8("Học phí / giờ:")), txtFee);

    attachLive(txtSubject, [](const QString& v) { return requiredError(v, QString::fromUtf8("Môn học")); });
    attachStartDateLive(txtStart);
    attachLive(txtFee, [](const QString& v) { return moneyError(v, QString::fromUtf8("Học phí")); });

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("QPushButton { background:#5B7188; color:white; padding:9px 18px; border:none; border-radius:6px; font-weight:600; } QPushButton:hover { background:#4A5F75; }");btnCancel->setCursor(Qt::PointingHandCursor);
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu lớp học"), &dialog);
    btnSave->setStyleSheet(QString("QPushButton { background:%1; color:%2; padding:9px 18px; border:none; border-radius:6px; font-weight:700; } QPushButton:hover { background:%3; }").arg(GRADIENT, ON_ACCENT, GRADIENT_HOVER));btnSave->setCursor(Qt::PointingHandCursor);
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        if (cbStudent->count() == 0) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Thông báo"), QString::fromUtf8("Chưa có học sinh nào để tạo lớp. Vui lòng thêm học sinh trước!"));
            return;
        }

        {
            FormCheck fc;
            fc.check(txtId, idError(txtId->text().trimmed().toUpper(), "LH", QString::fromUtf8("Mã lớp"), [this](const std::string& x) {
                for (size_t i = 0; i < classes.size(); ++i) if (classes[i]->getClassID() == x) return true;
                return false;
            }));
            fc.check(txtSubject, requiredError(txtSubject->text().trimmed(), QString::fromUtf8("Môn học")));
            fc.check(txtStart, dateError(txtStart->text().trimmed(), QString::fromUtf8("Ngày bắt đầu")));
            fc.check(txtFee, moneyError(txtFee->text().trimmed(), QString::fromUtf8("Học phí")));
            if (!fc.report(&dialog)) return;
            txtId->setText(txtId->text().trimmed().toUpper());
        }

        string sId = cbStudent->currentData().toString().toStdString();
        string tId = cbTutor->currentData().toString().toStdString();
        string today = QDate::currentDate().toString("dd/MM/yyyy").toStdString();

        TutoringClass candidate(
            txtId->text().trimmed().toStdString(), sId, tId,
            txtSubject->text().trimmed().toStdString(),
            txtSchedule->text().trimmed().toStdString(),
            txtStart->text().trimmed().toStdString(),
            cbStatus->currentText().toStdString(),
            txtFee->text().trimmed().toStdString(), today, today);
        string error;
        if (!ClassService(students, tutors, classes).create(candidate, error)) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Không thể tạo lớp"),
                                 QString::fromStdString(error));
            return;
        }
        currentMatchedStudent = nullptr;
        currentMatchedTutor = nullptr;
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã tạo lớp học thành công!"));
        dialog.accept();
    });

    if (dialog.exec() == QDialog::Accepted) focusNewRecord(4, classTable, txtId->text().trimmed());
}

void MainWindow::onEditClass(TutoringClass* c) {
    if (!c) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa lớp học: ") + QString::fromStdString(c->getClassID()));
    dialog.resize(480, 445);
    dialog.setStyleSheet(dialogStyle());

    auto* form = new QFormLayout(&dialog);
    form->addRow(dialogHeader(dialog.windowTitle()));
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(c->getClassID()), &dialog);
    txtId->setEnabled(false);

    auto* cbStudent = new QComboBox(&dialog);
    for (size_t i = 0; i < students.size(); ++i) {
        cbStudent->addItem(QString("%1 - %2").arg(QString::fromStdString(students[i]->getPersonID()), QString::fromStdString(students[i]->getFullName())), QString::fromStdString(students[i]->getPersonID()));
        if (students[i]->getPersonID() == c->getStudentID()) {
            cbStudent->setCurrentIndex(static_cast<int>(i));
        }
    }

    auto* cbTutor = new QComboBox(&dialog);
    cbTutor->addItem(QString::fromUtf8("-- Chưa có gia sư --"), "");
    int tIdx = 0;
    for (size_t i = 0; i < tutors.size(); ++i) {
        cbTutor->addItem(QString("%1 - %2").arg(QString::fromStdString(tutors[i]->getPersonID()), QString::fromStdString(tutors[i]->getFullName())), QString::fromStdString(tutors[i]->getPersonID()));
        if (tutors[i]->getPersonID() == c->getTutorID()) {
            tIdx = static_cast<int>(i + 1);
        }
    }
    cbTutor->setCurrentIndex(tIdx);

    auto* txtSubject = new QLineEdit(QString::fromStdString(c->getSubject()), &dialog);
    auto* txtSchedule = new QLineEdit(QString::fromStdString(c->getSchedule()), &dialog);
    auto* txtStart = new QLineEdit(QString::fromStdString(c->getStartDate()), &dialog);
    auto* cbStatus = new QComboBox(&dialog);
    cbStatus->addItems({QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư"), QString::fromUtf8("Hoàn thành")});
    cbStatus->setCurrentText(QString::fromStdString(c->getStatus()));
    auto* txtFee = new QLineEdit(QString::fromStdString(c->getRatePerHour()), &dialog);

    form->addRow(label(QString::fromUtf8("Mã lớp:")), txtId);
    form->addRow(label(QString::fromUtf8("Học sinh:")), cbStudent);
    form->addRow(label(QString::fromUtf8("Gia sư:")), cbTutor);
    form->addRow(label(QString::fromUtf8("Môn học:")), txtSubject);
    form->addRow(label(QString::fromUtf8("Lịch học:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Ngày bắt đầu:")), txtStart);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);
    form->addRow(label(QString::fromUtf8("Học phí / giờ:")), txtFee);

    attachLive(txtSubject, [](const QString& v) { return requiredError(v, QString::fromUtf8("Môn học")); });
    attachStartDateLive(txtStart);
    attachLive(txtFee, [](const QString& v) { return moneyError(v, QString::fromUtf8("Học phí")); });

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("QPushButton { background:#5B7188; color:white; padding:9px 18px; border:none; border-radius:6px; font-weight:600; } QPushButton:hover { background:#4A5F75; }");btnCancel->setCursor(Qt::PointingHandCursor);
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet(QString("QPushButton { background:%1; color:%2; padding:9px 18px; border:none; border-radius:6px; font-weight:700; } QPushButton:hover { background:%3; }").arg(GRADIENT, ON_ACCENT, GRADIENT_HOVER));btnSave->setCursor(Qt::PointingHandCursor);
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        {
            FormCheck fc;
            fc.check(txtSubject, requiredError(txtSubject->text().trimmed(), QString::fromUtf8("Môn học")));
            fc.check(txtStart, dateError(txtStart->text().trimmed(), QString::fromUtf8("Ngày bắt đầu")));
            fc.check(txtFee, moneyError(txtFee->text().trimmed(), QString::fromUtf8("Học phí")));
            if (!fc.report(&dialog)) return;
        }
        TutoringClass candidate = *c;
        candidate.setStudentID(cbStudent->currentData().toString().toStdString());
        candidate.setTutorID(cbTutor->currentData().toString().toStdString());
        candidate.setSubject(txtSubject->text().trimmed().toStdString());
        candidate.setSchedule(txtSchedule->text().trimmed().toStdString());
        candidate.setStartDate(txtStart->text().trimmed().toStdString());
        candidate.setStatus(cbStatus->currentText().toStdString());
        candidate.setRatePerHour(txtFee->text().trimmed().toStdString());
        candidate.setUpdatedAt(QDate::currentDate().toString("dd/MM/yyyy").toStdString());
        string error;
        if (!ClassService(students, tutors, classes).update(c->getClassID(), candidate, error)) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Không thể cập nhật lớp"),
                                 QString::fromStdString(error));
            return;
        }
        currentMatchedStudent = nullptr;
        currentMatchedTutor = nullptr;
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin lớp học!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteClass(TutoringClass* c) {
    if (!c) return;
    if (QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
        QString::fromUtf8("Bạn có chắc muốn xóa lớp %1? Việc này sẽ cập nhật lại "
                          "trạng thái học sinh và số học sinh của gia sư.")
            .arg(QString::fromStdString(c->getClassID())),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    string error;
    if (!ClassService(students, tutors, classes).remove(c->getClassID(), error)) {
        QMessageBox::warning(this, QString::fromUtf8("Không thể xóa lớp"), QString::fromStdString(error));
        return;
    }
    currentMatchedStudent = nullptr;
    currentMatchedTutor = nullptr;
    refreshAll();
    QMessageBox::information(this, QString::fromUtf8("Thành công"),
        QString::fromUtf8("Đã xóa lớp và đồng bộ lại dữ liệu liên quan."));
}

#include "MainWindow.moc"
