#include "MainWindow.h"
#include "IdGenerator.h"
#include "TextUtils.h"
#include "Validator.h"

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
#include <QLinearGradient>
#include <QPolygonF>
#include <QFontMetrics>

#include <algorithm>
#include <functional>
#include <utility>
#include <QTime>

using namespace std;

namespace {

// Kiểm tra dữ liệu nhập chung cho học sinh / gia sư. Trả về thông báo lỗi, rỗng nếu hợp lệ.
QString validatePersonInput(const QString& phone, const QString& email, const QString& dob,
                            const QString& money, const QString& moneyLabel) {
    if (!Validator::isValidPhone(phone.trimmed().toStdString())) {
        return QString::fromUtf8("Số điện thoại không hợp lệ (9-11 chữ số, bắt đầu bằng 0).");
    }
    if (!Validator::isValidEmail(email.trimmed().toStdString())) {
        return QString::fromUtf8("Email không hợp lệ.");
    }
    if (!Validator::isValidDate(dob.trimmed().toStdString())) {
        return QString::fromUtf8("Ngày sinh không hợp lệ (định dạng dd/MM/yyyy, ví dụ 25/12/2008).");
    }
    if (!Validator::isNonNegativeNumber(money.trimmed().toStdString())) {
        return moneyLabel + QString::fromUtf8(" phải là số nguyên không âm (ví dụ 200000).");
    }
    return QString();
}

}  // namespace

namespace {

// Color palette matching the modern dashboard theme
const QString SIDEBAR_BG = "#0B2161";
const QString SIDEBAR_ACTIVE = "#1E6BFF";
const QString SIDEBAR_HOVER = "#17317F";
const QString SIDEBAR_TEXT = "#C3CEF0";

const QString BG = "#EFF5FD";
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
        "QFrame#appCard { background:#FFFFFF; border:1px solid #DCE6F5; border-radius:16px; }");
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
                       const QString& bg = "#0F172A", const QString& fg = "#FFFFFF",
                       const QString& sub = QString()) {
    auto* w = new QWidget;
    auto* l = new QHBoxLayout(w);
    l->setContentsMargins(4, 2, 4, 2);
    l->setSpacing(10);

    auto* a = new QLabel(initials, w);
    a->setAlignment(Qt::AlignCenter);
    a->setFixedSize(38, 38);
    a->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:19px; font-size:13px; font-weight:700;")
        .arg(bg, fg));
    l->addWidget(a);

    auto* col = new QVBoxLayout;
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(1);
    col->addWidget(label(name, 13, true, TEXT_MAIN));
    if (!sub.isEmpty()) col->addWidget(label(sub, 11, false, TEXT_MUTED));
    l->addLayout(col);
    l->addStretch();
    return w;
}

void styleTable(QTableWidget* table, const QString& accent = "#0284C7") {
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setStyleSheet(QString(
        "QTableWidget { background:#FFFFFF; alternate-background-color:#F8FAFC; border:none; "
        "color:#334155; font-size:12px; }"
        "QTableWidget::item { border-bottom:1px solid #EEF2F7; padding:8px 8px; }"
        "QTableWidget::item:hover { background:#F0F9FF; }"
        "QTableWidget::item:selected { background:#E0F2FE; color:#0F172A; }"
        "QHeaderView::section { background:#F1F5F9; color:#334155; border:none; "
        "border-bottom:2px solid %1; padding:12px 8px; font-size:11px; font-weight:700; }").arg(accent));
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(90);
    table->horizontalHeader()->setHighlightSections(false);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->verticalHeader()->setDefaultSectionSize(54);
    table->setTextElideMode(Qt::ElideNone);
    table->setWordWrap(false);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

QMap<QTableWidget*, std::function<void()>>& pagerHooks() {
    static QMap<QTableWidget*, std::function<void()>> m;
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

        p.setPen(isDark ? QColor("#FFFFFF") : QColor("#0F172A"));
        QFont fMain = p.font();
        fMain.setPointSize(16);
        fMain.setBold(true);
        p.setFont(fMain);
        p.drawText(QRectF(0, height() / 2 - 20, width(), 24), Qt::AlignCenter, centerMain);

        p.setPen(isDark ? QColor("#38BDF8") : QColor("#64748B"));
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
    explicit BarChartWidget(const QColor& c = QColor("#0284C7"), QWidget* parent = nullptr)
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
            p.setPen(QColor("#94A3B8"));
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
        p.setPen(QPen(QColor("#E2E8F0"), 1));
        for (int i = 0; i <= 4; ++i) {
            double y = area.bottom() - area.height() * i / 4.0;
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            p.setPen(QColor("#94A3B8"));
            p.drawText(QRectF(0, y - 8, 28, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(maxV * i / 4.0, 'f', 0));
            p.setPen(QPen(QColor("#E2E8F0"), 1));
        }
        int n = labels.size();
        double slot = area.width() / n;
        double bw = qMin(46.0, slot * 0.55);
        QFontMetrics fm(base);
        for (int i = 0; i < n; ++i) {
            double h = area.height() * values[i] / maxV;
            QRectF bar(area.left() + slot * i + (slot - bw) / 2, area.bottom() - h, bw, h);
            QLinearGradient g(bar.topLeft(), bar.bottomLeft());
            g.setColorAt(0, color);
            g.setColorAt(1, color.lighter(135));
            p.setPen(Qt::NoPen);
            p.setBrush(g);
            p.drawRoundedRect(bar, 5, 5);
            p.setPen(QColor("#0F172A"));
            p.drawText(QRectF(bar.left() - 10, bar.top() - 18, bw + 20, 16), Qt::AlignCenter,
                       QString::number(values[i], 'f', 0));
            p.setPen(QColor("#64748B"));
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

        p.setPen(QPen(QColor("#F1F5F9"), pw, Qt::SolidLine, Qt::FlatCap));
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
        p.setPen(QColor("#0F172A"));
        p.drawText(QRectF(r.left(), r.center().y() - 22, r.width(), 28), Qt::AlignCenter,
                   QString::number(static_cast<int>(total)));
        f.setPointSize(8);
        f.setBold(false);
        p.setFont(f);
        p.setPen(QColor("#64748B"));
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
            p.setPen(QColor("#334155"));
            p.drawText(QRectF(lx + 20, y, width() - lx - 90, rowH), Qt::AlignLeft | Qt::AlignVCenter, labels[i]);
            lf.setBold(true);
            p.setFont(lf);
            p.setPen(QColor("#0F172A"));
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
            p.setPen(QColor("#94A3B8"));
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
            p.setPen(QPen(QColor("#E2E8F0"), 1, Qt::DashLine));
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            p.setPen(QColor("#94A3B8"));
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
        g.setColorAt(0, QColor(2, 132, 199, 95));
        g.setColorAt(1, QColor(2, 132, 199, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawPolygon(poly);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor("#0284C7"), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        for (int i = 0; i + 1 < pts.size(); ++i) p.drawLine(pts[i], pts[i + 1]);

        for (int i = 0; i < n; ++i) {
            p.setPen(QPen(QColor("#0284C7"), 2));
            p.setBrush(QColor("#FFFFFF"));
            p.drawEllipse(pts[i], 4.5, 4.5);
            p.setPen(QColor("#0F172A"));
            p.drawText(QRectF(pts[i].x() - 20, pts[i].y() - 22, 40, 16), Qt::AlignCenter,
                       QString::number(values[i], 'f', 0));
            p.setPen(QColor("#64748B"));
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
    static const char* pal[][2] = {{"#DBEAFE", "#1E40AF"}, {"#FCE7F3", "#9D174D"}, {"#DCFCE7", "#166534"},
                                   {"#FEF3C7", "#92400E"}, {"#EDE9FE", "#5B21B6"}, {"#CFFAFE", "#155E75"}};
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
    static const char* pal[][2] = {{"#DBEAFE", "#1D4ED8"}, {"#FCE7F3", "#BE185D"}, {"#FFEDD5", "#C2410C"},
                                   {"#DCFCE7", "#15803D"}, {"#EDE9FE", "#6D28D9"}, {"#CFFAFE", "#0E7490"}};
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
        Book books[] = {{QRectF(14, 80, 132, 14), QColor("#1D4ED8")},
                        {QRectF(22, 66, 120, 14), QColor("#3B82F6")},
                        {QRectF(10, 52, 124, 14), QColor("#93C5FD")}};
        for (const Book& b : books) {
            p.setBrush(b.c);
            p.drawRoundedRect(b.r, 4, 4);
            p.setBrush(QColor(255, 255, 255, 170));
            p.drawRoundedRect(QRectF(b.r.right() - 10, b.r.top() + 3, 8, b.r.height() - 6), 2, 2);
        }
        // mũ tốt nghiệp
        QPolygonF base;
        base << QPointF(50, 32) << QPointF(50, 50) << QPointF(80, 60) << QPointF(110, 50) << QPointF(110, 32);
        p.setBrush(QColor("#1E3A8A"));
        p.drawPolygon(base);
        QPolygonF board;
        board << QPointF(80, 8) << QPointF(136, 28) << QPointF(80, 46) << QPointF(24, 28);
        p.setBrush(QColor("#0F2A66"));
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
        "stop:0 #FFFFFF,stop:1 #E3EEFF); border:1px solid #DCE6F5; border-radius:16px; }");
    auto* l = new QHBoxLayout(f);
    l->setContentsMargins(24, 16, 16, 16);
    l->setSpacing(12);

    int hr = QTime::currentTime().hour();
    QString greet = hr < 12 ? QString::fromUtf8("Chào buổi sáng,")
                  : hr < 18 ? QString::fromUtf8("Chào buổi chiều,")
                            : QString::fromUtf8("Chào buổi tối,");
    auto* col = new QVBoxLayout;
    col->setSpacing(2);
    col->addWidget(label(QString::fromUtf8("👋  ") + greet, 14, true, "#0B1F4B"));
    col->addWidget(label("Admin User!", 28, true, "#0B1F4B"));
    col->addWidget(label(QString::fromUtf8("Chúc bạn một ngày làm việc hiệu quả cùng TutorFlow!"), 12, false, "#5B6B8C"));
    col->addStretch();
    l->addLayout(col, 2);

    struct Tile { const char* icon; const char* title; const char* iconBg; const char* bg; };
    const Tile tiles[4] = {
        {"👥", "Tổng học sinh", "#DBEAFE", "#FFFFFF"},
        {"🎓", "Tổng gia sư", "#D1FAE5", "#F2FBF7"},
        {"📖", "Lớp học đang diễn ra", "#EDE9FE", "#F6F3FF"},
        {"📄", "Hợp đồng hiệu lực", "#FFE4E6", "#FFF4F6"}};
    for (const Tile& t : tiles) {
        auto* c = new QFrame;
        c->setObjectName("miniStat");
        c->setStyleSheet(QString("QFrame#miniStat { background:%1; border:1px solid #E5ECF7; border-radius:12px; }")
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
        top->addWidget(label(QString::fromUtf8(t.title), 11, false, "#475569"), 1);
        v->addLayout(top);
        auto* val = label("0", 24, true, "#0B1F4B");
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
            "QPushButton { background:#FFFFFF; color:#475569; border:1px solid #DCE6F5; border-radius:10px; "
            "padding:0 18px; font-size:12px; font-weight:600; }"
            "QPushButton:hover { background:#F0F6FF; }"
            "QPushButton:checked { background:#1E6BFF; color:#FFFFFF; border:1px solid #1E6BFF; }");
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
            "QComboBox { background:white; border:1px solid #D6E2F3; border-radius:8px; padding:0 8px; font-size:12px; }");
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
    }
    ~PagerBar() override { pagerHooks().remove(table); }

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
                ? "QPushButton { background:#1E6BFF; color:white; border:none; border-radius:8px; font-weight:700; }"
                : "QPushButton { background:white; color:#475569; border:1px solid #DCE6F5; border-radius:8px; }"
                  "QPushButton:hover { background:#F0F6FF; } QPushButton:disabled { color:#CBD5E1; }");
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

QLineEdit* addFilter(QHBoxLayout* l, const QString& placeholder, int width = 200) {
    auto* edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setFixedHeight(42);
    edit->setMinimumWidth(width);
    edit->setStyleSheet(
        "QLineEdit { background:white; border:1px solid #D6E2F3; border-radius:10px; "
        "padding:0 12px; font-size:12px; color:#344054; }"
        "QLineEdit:focus { border:1px solid #1E6BFF; }");
    l->addWidget(edit, 1);
    return edit;
}

QComboBox* combo(const QStringList& items, int width = 150) {
    auto* c = new QComboBox;
    c->addItems(items);
    c->setFixedSize(width, 42);
    c->setStyleSheet(
        "QComboBox { background:white; border:1px solid #D6E2F3; border-radius:10px; "
        "padding:0 10px; font-size:12px; color:#344054; }"
        "QComboBox::drop-down { border:none; width:24px; }");
    return c;
}


void applyBanner(QFrame* f, const QString&, const QString&) {
    f->setObjectName("pageBanner");
    f->setStyleSheet(
        "QFrame#pageBanner { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #FFFFFF,stop:0.55 #EAF3FF,stop:1 #D3E6FF); "
        "border:1px solid #DCE6F5; border-radius:16px; }");
}

void styleBannerButton(QPushButton* b) {
    b->setFixedHeight(44);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(
        "QPushButton { background:#1E6BFF; color:#FFFFFF; border:none; border-radius:10px; "
        "padding:0 22px; font-size:13px; font-weight:700; }"
        "QPushButton:hover { background:#1558D6; }");
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
    valLabelOut = label(initialValue, 24, true, color);
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

    auto* search = new QLineEdit(h);
    search->setPlaceholderText(QString::fromUtf8("🔍  Tìm kiếm hệ thống..."));
    search->setFixedSize(560, 44);
    search->setStyleSheet(
        "QLineEdit { background:#FFFFFF; border:1px solid #DCE6F5; "
        "border-radius:12px; padding:0 16px; color:#0F172A; font-size:13px; }"
        "QLineEdit:focus { border:1px solid #0284C7; background:#FFFFFF; }");
    l->addWidget(search);
    l->addStretch();

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
    sep->setStyleSheet("background:#E2E8F0; border:none;");
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
        "background:#2563EB; color:white; border-radius:17px; font-size:14px;");
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
    icon->setFixedSize(36, 36);
    icon->setStyleSheet(QString(
        "background:%1; color:%2; border-radius:8px; font-size:16px;")
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

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    students = FileManager::loadStudents();
    tutors = FileManager::loadTutors();
    classes = FileManager::loadClasses();
    contracts = FileManager::loadContracts();
    registrations = FileManager::loadRegistrations();

    setWindowTitle(QString::fromUtf8("QUẢN LÝ GIA SƯ - Hệ thống quản trị TutorFlow"));
    resize(1360, 860);
    setMinimumSize(1100, 720);

    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");
    central->setStyleSheet(QString("QWidget#centralWidget { background:%1; }").arg(BG));
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
    stackedWidget->addWidget(createContractPage());      // 5
    stackedWidget->addWidget(createStatisticsPage());    // 6
    stackedWidget->addWidget(createRegistrationPage());  // 7

    mainLayout->addWidget(stackedWidget, 1);
    setCentralWidget(central);

    refreshAll();
}

MainWindow::~MainWindow() {
    for (size_t i = 0; i < students.size(); ++i) delete students[i];
    for (size_t i = 0; i < tutors.size(); ++i) delete tutors[i];
    for (size_t i = 0; i < classes.size(); ++i) delete classes[i];
    for (size_t i = 0; i < contracts.size(); ++i) delete contracts[i];
    for (size_t i = 0; i < registrations.size(); ++i) delete registrations[i];
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
    }
}

QWidget* MainWindow::createSidebar() {
    auto* panel = new QFrame;
    panel->setFrameShape(QFrame::NoFrame);
    panel->setFixedWidth(240);
    panel->setObjectName("appSidebar");
    panel->setAttribute(Qt::WA_StyledBackground, true);
    panel->setStyleSheet(QString(
        "QFrame#appSidebar { background:%1; color:white; border-right:1px solid #16307A; }")
        .arg(SIDEBAR_BG));

    auto* l = new QVBoxLayout(panel);
    l->setContentsMargins(14, 20, 14, 16);
    l->setSpacing(6);

    // Brand logo + text + version subtitle
    auto* brandWidget = new QFrame(panel);
    brandWidget->setObjectName("brandCard");
    brandWidget->setAttribute(Qt::WA_StyledBackground, true);
    brandWidget->setStyleSheet(
        "QFrame#brandCard { background:transparent; border:none; }"
    );
    auto* bl = new QHBoxLayout(brandWidget);
    bl->setContentsMargins(10, 8, 10, 8);
    bl->setSpacing(10);

    auto* logoIcon = new QLabel(QString::fromUtf8("🎓"), brandWidget);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setFixedSize(36, 36);
    logoIcon->setStyleSheet("background:#2563EB; border-radius:12px; font-size:20px; color:#FFFFFF;");
    bl->addWidget(logoIcon);

    auto* brandCol = new QVBoxLayout;
    brandCol->setContentsMargins(0, 0, 0, 0);
    brandCol->setSpacing(2);
    auto* brandTitle = new QLabel(QString::fromUtf8("TUTORFLOW"), brandWidget);
    brandTitle->setStyleSheet(
        "color:#FFFFFF; font-size:13px; font-weight:800; letter-spacing:0.5px; background:transparent;");
    auto* brandSub = new QLabel(QString::fromUtf8("Quản lý gia sư"), brandWidget);
    brandSub->setStyleSheet("color:#9DB7FF; font-size:12px; font-weight:500; background:transparent;");
    brandCol->addWidget(brandTitle);
    brandCol->addWidget(brandSub);
    bl->addLayout(brandCol);
    bl->addStretch();
    l->addWidget(brandWidget);
    l->addSpacing(8);

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
        {QString::fromUtf8("📊"), QString::fromUtf8("Thống kê")},
        {QString::fromUtf8("📝"), QString::fromUtf8("Đăng ký môn")}
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

    auto* promo = new QFrame(panel);
    promo->setObjectName("promoCard");
    promo->setStyleSheet(
        "QFrame#promoCard { background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1E4FD8,stop:1 #1636A0); "
        "border-radius:14px; }");
    auto* pv = new QVBoxLayout(promo);
    pv->setContentsMargins(14, 14, 14, 8);
    pv->setSpacing(4);
    pv->addWidget(label(QString::fromUtf8("Kết nối tri thức\nKiến tạo tương lai"), 13, true, "#FFFFFF"));
    auto* promoDesc = label(QString::fromUtf8("TutorFlow – Đồng hành cùng bạn trên hành trình học tập và phát triển."), 10, false, "#C7D6FF");
    promoDesc->setWordWrap(true);
    pv->addWidget(promoDesc);
    pv->addWidget(new BookIllustration(promo), 0, Qt::AlignHCenter);
    l->addWidget(promo);
    l->addSpacing(6);

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

    connect(btnAddStudent, &QPushButton::clicked, this, &MainWindow::onAddStudent);
    connect(btnAddTutor, &QPushButton::clicked, this, &MainWindow::onAddTutor);
    connect(btnFindTutor, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(btnCreateClass, &QPushButton::clicked, this, &MainWindow::onAddClass);

    l->addLayout(headerRow);

    // Grid of 4 Metric Cards
    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Tổng học sinh"), dashTotalStudentsLabel, "0", "+12%", QString::fromUtf8(" theo dữ liệu"), QString::fromUtf8("👥"), "#F1F5F9", "#475569", true), 1);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Tổng gia sư"), dashTotalTutorsLabel, "0", "+5%", QString::fromUtf8(" theo dữ liệu"), QString::fromUtf8("🎓"), "#E0F2FE", "#0284C7", true), 1);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Lớp đang hoạt động"), dashActiveClassesLabel, "0", "Thời gian thực", QString::fromUtf8(" hệ thống"), QString::fromUtf8("📥"), "#F1F5F9", "#475569", true), 1);
    metrics->addWidget(makeMetricCardWithLabel(QString::fromUtf8("Tỷ lệ ghép thành công"), dashMatchRateLabel, "0%", "Tự động", QString::fromUtf8(" từ dữ liệu"), QString::fromUtf8("🤝"), "#E0F2FE", "#0284C7", true), 1);
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
    dashChartTutorSubject = new BarChartWidget(QColor("#0284C7"));
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
        "QPushButton { background:transparent; color:#0284C7; border:none; "
        "font-size:11px; font-weight:700; } QPushButton:hover { color:#0369A1; }");
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
    dashboardRecentTable->setFixedHeight(225);
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
        dashboardRecentTable->setCellWidget(i, 1, userAvatarItem(sInit, sName));

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
            "QPushButton { background:transparent; color:#64748B; border:none; font-size:14px; font-weight:bold; } "
            "QPushButton:hover { color:#0F172A; background:#F1F5F9; border-radius:4px; }");
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
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#0EA5E9", "#4F46E5");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý học sinh"), 22, true, "#0B1F4B");
    auto* pageDesc = label(QString::fromUtf8("Quản lý danh sách, tìm kiếm và phân bổ học sinh cho gia sư."), 13, false, "#5B6B8C");
    tCol->addWidget(pageTitle);
    tCol->addWidget(pageDesc);
    topLayout->addLayout(tCol, 1);

    auto* btnAddStudent = new QPushButton(QString::fromUtf8("＋  Thêm học sinh"), topCard);
    btnAddStudent->setFixedHeight(40);
    btnAddStudent->setCursor(Qt::PointingHandCursor);
    btnAddStudent->setStyleSheet(
        "QPushButton { background:#0F172A; color:white; border:none; border-radius:6px; "
        "padding:0 20px; font-size:13px; font-weight:600; } QPushButton:hover { background:#1E293B; }");
    connect(btnAddStudent, &QPushButton::clicked, this, &MainWindow::onAddStudent);
    styleBannerButton(btnAddStudent);
    topLayout->addWidget(btnAddStudent);
    topLayout->addWidget(new BookIllustration(topCard));
    l->addWidget(topCard);

    auto* stuSummary = new QHBoxLayout;
    stuSummary->setSpacing(14);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Tổng học sinh"), stuSumTotal, "0", "#0EA5E9"), 1);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Đã ghép gia sư"), stuSumMatched, "0", GREEN), 1);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Chưa ghép"), stuSumUnmatched, "0", "#F59E0B"), 1);
    stuSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Ngân sách TB"), stuSumAvgBudget, "0đ", "#4F46E5"), 1);
    l->addLayout(stuSummary);

    // Filter Bar Card
    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);

    studentSearchEdit = addFilter(fl, QString::fromUtf8("🔍  Tìm kiếm theo mã, tên, môn học, SĐT..."), 280);
    connect(studentSearchEdit, &QLineEdit::textChanged, this, [this]() { populateStudentTable(); });

    studentGradeCombo = combo({QString::fromUtf8("Tất cả khối lớp"), "10", "11", "12", "5", "6", "7", "8", "9"}, 160);
    connect(studentGradeCombo, &QComboBox::currentTextChanged, this, [this]() { populateStudentTable(); });
    fl->addWidget(studentGradeCombo);

    studentStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")}, 150);
    connect(studentStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateStudentTable(); });
    fl->addWidget(studentStatusCombo);

    l->addWidget(filters);

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
    styleTable(studentTable, "#0EA5E9");
    studentTable->setMinimumHeight(350);

    tl->addWidget(studentTable);
    tl->addWidget(new PagerBar(studentTable, QString::fromUtf8("học sinh"), tableCard));
    l->addWidget(tableCard);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateStudentTable() {
    if (!studentTable) return;
    QString kw = studentSearchEdit ? studentSearchEdit->text().trimmed().toLower() : "";
    QString gradeFilter = studentGradeCombo ? studentGradeCombo->currentText() : "";
    QString statusFilter = studentStatusCombo ? studentStatusCombo->currentText() : "";

    Vector<Student*> filtered;
    for (size_t i = 0; i < students.size(); ++i) {
        Student* s = students[i];
        if (!kw.isEmpty()) {
            QString id = QString::fromStdString(s->getPersonID()).toLower();
            QString name = QString::fromStdString(s->getFullName()).toLower();
            QString phone = QString::fromStdString(s->getPhone()).toLower();
            QString subj = joinStrings(s->getNeedSubjects()).toLower();
            if (!id.contains(kw) && !name.contains(kw) && !phone.contains(kw) && !subj.contains(kw)) {
                continue;
            }
        }
        if (!gradeFilter.isEmpty() && gradeFilter != QString::fromUtf8("Tất cả khối lớp")) {
            QString g = QString::fromStdString(s->getGrade());
            if (!g.contains(gradeFilter) && !gradeFilter.contains(g)) {
                continue;
            }
        }
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
            p->setStyleSheet("background:#DCFCE7; color:#166534; border-radius:12px; font-size:11px; font-weight:700;");
            schl->addWidget(p);
        }
        studentTable->setCellWidget(row, 3, schW);

        // Budget
        studentTable->setCellWidget(row, 4, label(QString::fromStdString(s->getBudgetPerHour()) + QString::fromUtf8("đ/buổi"), 12, true, BLUE));

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
        btnView->setStyleSheet("QPushButton { background:#E8F1FF; border:none; border-radius:16px; color:#1E6BFF; font-size:12px; } QPushButton:hover { background:#E2E8F0; }");

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#EFF6FF; border:none; border-radius:16px; color:#0284C7; font-size:12px; } QPushButton:hover { background:#DBEAFE; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#DC2626; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

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
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#7C3AED", "#DB2777");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý gia sư"), 22, true, "#0B1F4B");
    auto* pageDesc = label(QString::fromUtf8("Danh sách gia sư, chuyên môn và lịch nhận lớp."), 13, false, "#5B6B8C");
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

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    tutorSearchEdit = addFilter(fl, QString::fromUtf8("🔍  Tìm kiếm gia sư theo tên, chuyên môn, SĐT..."), 280);
    connect(tutorSearchEdit, &QLineEdit::textChanged, this, [this]() { populateTutorTable(); });

    tutorSubjectCombo = combo({QString::fromUtf8("Tất cả môn"), "Toán", "Lý", "Hóa", "Tiếng Anh", "Văn"}, 140);
    connect(tutorSubjectCombo, &QComboBox::currentTextChanged, this, [this]() { populateTutorTable(); });
    fl->addWidget(tutorSubjectCombo);

    tutorAreaCombo = combo({QString::fromUtf8("Tất cả khu vực"), "Hải Châu", "Thanh Khê", "Liên Chiểu", "Ngũ Hành Sơn", "Sơn Trà"}, 150);
    connect(tutorAreaCombo, &QComboBox::currentTextChanged, this, [this]() { populateTutorTable(); });
    fl->addWidget(tutorAreaCombo);

    tutorStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Sẵn sàng"), QString::fromUtf8("Bận")}, 140);
    connect(tutorStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateTutorTable(); });
    fl->addWidget(tutorStatusCombo);
    l->addWidget(filters);

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
    styleTable(tutorTable, "#7C3AED");
    tutorTable->setMinimumHeight(350);

    tl->addWidget(tutorTable);
    tl->addWidget(new PagerBar(tutorTable, QString::fromUtf8("gia sư"), tableCard));
    l->addWidget(tableCard, 1);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateTutorTable() {
    if (!tutorTable) return;
    QString kw = tutorSearchEdit ? tutorSearchEdit->text().trimmed().toLower() : "";
    QString subjFilter = tutorSubjectCombo ? tutorSubjectCombo->currentText() : "";
    QString areaFilter = tutorAreaCombo ? tutorAreaCombo->currentText() : "";
    QString statusFilter = tutorStatusCombo ? tutorStatusCombo->currentText() : "";

    Vector<Tutor*> filtered;
    for (size_t i = 0; i < tutors.size(); ++i) {
        Tutor* t = tutors[i];
        if (!kw.isEmpty()) {
            QString id = QString::fromStdString(t->getPersonID()).toLower();
            QString name = QString::fromStdString(t->getFullName()).toLower();
            QString phone = QString::fromStdString(t->getPhone()).toLower();
            QString sub = joinStrings(t->getSubjects()).toLower();
            if (!id.contains(kw) && !name.contains(kw) && !phone.contains(kw) && !sub.contains(kw)) {
                continue;
            }
        }
        if (!subjFilter.isEmpty() && subjFilter != QString::fromUtf8("Tất cả môn")) {
            QString s = joinStrings(t->getSubjects());
            if (!s.contains(subjFilter)) continue;
        }
        if (!areaFilter.isEmpty() && areaFilter != QString::fromUtf8("Tất cả khu vực")) {
            QString a = joinStrings(t->getTeachingAreas());
            if (!a.contains(areaFilter)) continue;
        }
        if (statusFilter == QString::fromUtf8("Sẵn sàng") && !t->getIsAvailable()) continue;
        if (statusFilter == QString::fromUtf8("Bận") && t->getIsAvailable()) continue;

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
        tutorTable->setCellWidget(row, 4, label(QString::fromStdString(t->getRatePerHour()) + QString::fromUtf8("đ/buổi"), 12, true, BLUE));
        tutorTable->setCellWidget(row, 5, label(QString::fromStdString(t->getAvailability()), 12, false, TEXT_MAIN));
        tutorTable->setCellWidget(row, 6, label(joinStrings(t->getTeachingAreas()), 12, false, TEXT_MUTED));

        tutorTable->setCellWidget(row, 7, pillBadge(
            t->getIsAvailable() ? QString::fromUtf8("Sẵn sàng") : QString::fromUtf8("Bận"),
            t->getIsAvailable() ? GREEN_BG : RED_BG,
            t->getIsAvailable() ? GREEN : RED, true));

        auto* actW = new QWidget;
        auto* actL = new QHBoxLayout(actW);
        actL->setContentsMargins(4, 2, 4, 2);
        actL->setSpacing(4);

        auto* btnView = new QPushButton(QString::fromUtf8("👁"), actW);
        btnView->setToolTip(QString::fromUtf8("Xem chi tiết"));
        btnView->setFixedSize(32, 32);
        btnView->setStyleSheet("QPushButton { background:#E8F1FF; border:none; border-radius:16px; color:#1E6BFF; font-size:12px; } QPushButton:hover { background:#E2E8F0; }");

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#EFF6FF; border:none; border-radius:16px; color:#0284C7; font-size:12px; } QPushButton:hover { background:#DBEAFE; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#DC2626; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

        actL->addWidget(btnView);
        actL->addWidget(btnEdit);
        actL->addWidget(btnDel);
        actL->addStretch();
        tutorTable->setCellWidget(row, 8, actW);

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
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(3);
    tCol->addWidget(label(QString::fromUtf8("Tìm Gia Sư Thông Minh"), 24, true, TEXT_MAIN));
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
        "QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:8px; "
        "padding:0 12px; font-size:13px; font-weight:600; color:#0F172A; } "
        "QComboBox::drop-down { border:none; width:28px; }");
    connect(matchStudentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateMatchingStudentReqs);
    scLayout->addWidget(matchStudentCombo);

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
    matchReqSubject = label(QString::fromUtf8("Toán Học"), 13, true, TEXT_MAIN);
    matchReqGrade = new QLabel(QString::fromUtf8("Lớp 10"), t1);
    matchReqGrade->setStyleSheet("background:#0F172A; color:white; border-radius:4px; padding:2px 6px; font-size:10px; font-weight:700;");
    t1v->addWidget(matchReqSubject);
    t1v->addWidget(matchReqGrade);
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
    matchReqArea = label(QString::fromUtf8("Hải Châu"), 13, true, TEXT_MAIN);
    t2v->addWidget(pin);
    t2v->addWidget(matchReqArea);
    t2v->addStretch();
    t2l->addLayout(t2v);
    tilesRow->addWidget(t2, 1);

    // Tile 3: Ngân sách
    auto* t3 = new QFrame(reqCard);
    t3->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* t3l = new QVBoxLayout(t3);
    t3l->setContentsMargins(12, 10, 12, 10);
    t3l->addWidget(label(QString::fromUtf8("NGÂN SÁCH (BUỔI)"), 10, true, TEXT_MUTED));
    matchReqBudget = label(QString::fromUtf8("200,000đ"), 14, true, BLUE);
    t3l->addWidget(matchReqBudget);
    tilesRow->addWidget(t3, 1);

    // Tile 4: Lịch học yêu cầu
    auto* t4 = new QFrame(reqCard);
    t4->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
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
        dl->setStyleSheet("background:#E2E8F0; color:#94A3B8; border-radius:3px; font-size:10px; font-weight:600;");
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
        "QPushButton { background:#060D1E; color:white; border:none; border-radius:22px; "
        "font-size:13px; font-weight:700; letter-spacing:0.5px; } QPushButton:hover { background:#111E38; }");
    connect(findBtn, &QPushButton::clicked, this, &MainWindow::runSmartMatching);
    l->addWidget(findBtn, 0, Qt::AlignCenter);

    // Matching Results Section Header
    auto* resHeader = new QHBoxLayout;
    resHeader->addWidget(label(QString::fromUtf8("💫  Kết Quả Matching"), 16, true, TEXT_MAIN));
    resHeader->addStretch();
    resHeader->addWidget(pillBadge(QString::fromUtf8("Đề xuất tối ưu theo thuật toán PBL2"), "#F1F5F9", "#64748B"));
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

    auto* topRibbon = new QLabel("TOP 1", candidateCard);
    topRibbon->setAlignment(Qt::AlignCenter);
    topRibbon->setFixedSize(54, 26);
    topRibbon->setStyleSheet("background:#EAB308; color:#000000; font-size:11px; font-weight:800; border-radius:4px;");
    candLayout->addWidget(topRibbon, 0, Qt::AlignLeft);

    auto* tutorPhoto = new QLabel(QString::fromUtf8("👨‍🏫"), candidateCard);
    tutorPhoto->setAlignment(Qt::AlignCenter);
    tutorPhoto->setFixedSize(90, 90);
    tutorPhoto->setStyleSheet(
        "background:#111E38; border:2px solid #38BDF8; border-radius:12px; font-size:44px;");
    candLayout->addWidget(tutorPhoto, 0, Qt::AlignCenter);

    candidateId = new QLabel("GS001", candidateCard);
    candidateId->setAlignment(Qt::AlignCenter);
    candidateId->setFixedSize(65, 22);
    candidateId->setStyleSheet("background:#111E38; color:#94A3B8; border-radius:11px; font-size:10px; font-weight:700;");
    candLayout->addWidget(candidateId, 0, Qt::AlignCenter);

    candidateName = new QLabel(QString::fromUtf8("Chưa chọn học sinh"), candidateCard);
    candidateName->setAlignment(Qt::AlignCenter);
    candidateName->setStyleSheet("color:#FFFFFF; font-size:18px; font-weight:800; background:transparent;");
    candLayout->addWidget(candidateName);

    candidateDegree = new QLabel(QString::fromUtf8("🎓 Đang chờ tìm kiếm"), candidateCard);
    candidateDegree->setAlignment(Qt::AlignCenter);
    candidateDegree->setStyleSheet("color:#38BDF8; font-size:12px; font-weight:500; background:transparent;");
    candLayout->addWidget(candidateDegree);

    candLayout->addSpacing(8);

    candidateDonutContainer = new QWidget(candidateCard);
    auto* donL = new QVBoxLayout(candidateDonutContainer);
    donL->setContentsMargins(0, 0, 0, 0);
    auto* donut = new DonutChartWidget(85, QColor("#0284C7"), QColor("#111E38"), "85%", QString::fromUtf8("ĐỘ PHÙ HỢP"), true, candidateDonutContainer);
    donL->addWidget(donut, 0, Qt::AlignCenter);
    candLayout->addWidget(candidateDonutContainer, 0, Qt::AlignCenter);

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

    auto makeStatBox = [](const QString& t, QLabel*& valOut, const QString& initialVal, const QString& valColor = TEXT_MAIN) {
        auto* b = new QFrame;
        b->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
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

    addScoreRow(0, 0, QString::fromUtf8("Kinh nghiệm chuyên môn"), "score_exp", "20/25", 80);
    addScoreRow(0, 1, QString::fromUtf8("Khu vực địa lý"), "score_loc", "10/10", 100);
    addScoreRow(1, 0, QString::fromUtf8("Học phí yêu cầu"), "score_price", "40/45", 88);
    addScoreRow(1, 1, QString::fromUtf8("Lịch học trùng khớp"), "score_sched", "20/20", 100);

    mrLayout->addWidget(candidateScoreBreakdownContainer);

    // "VÌ SAO GIA SƯ NÀY PHÙ HỢP?" Card
    candidateWhyContainer = new QFrame(matchRight);
    candidateWhyContainer->setStyleSheet("background:#F8FAFC; border:1px solid #E2E8F0; border-radius:8px;");
    auto* wl = new QVBoxLayout(candidateWhyContainer);
    wl->setContentsMargins(14, 12, 14, 12);
    wl->setSpacing(8);

    auto* whyTitle = new QHBoxLayout;
    auto* bulb = new QLabel(QString::fromUtf8("💡"), candidateWhyContainer);
    bulb->setStyleSheet("color:#0284C7; font-size:14px;");
    whyTitle->addWidget(bulb);
    whyTitle->addWidget(label(QString::fromUtf8("VÌ SAO GIA SƯ NÀY PHÙ HỢP?"), 11, true, BLUE));
    whyTitle->addStretch();
    wl->addLayout(whyTitle);

    auto* why1 = label(QString::fromUtf8("• Môn giảng dạy hoàn toàn phù hợp với nhu cầu học tập của sinh viên."), 12, false, TEXT_MAIN);
    why1->setObjectName("why_1");
    auto* why2 = label(QString::fromUtf8("• Lịch dạy trống tương thích tốt với thời gian rảnh của học viên."), 12, false, TEXT_MAIN);
    why2->setObjectName("why_2");
    auto* why3 = label(QString::fromUtf8("• Mức học phí và khu vực nằm trong khoảng mong muốn."), 12, false, TEXT_MAIN);
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
        "QPushButton { background:#0284C7; color:white; border:none; border-radius:6px; "
        "padding:0 24px; font-size:12px; font-weight:700; } QPushButton:hover { background:#0369A1; }");
    connect(btnAcceptCandidate, &QPushButton::clicked, this, &MainWindow::acceptCandidateMatch);
    btnRow->addWidget(btnAcceptCandidate);
    mrLayout->addLayout(btnRow);

    matchBody->addWidget(matchRight, 1);
    l->addLayout(matchBody);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::updateMatchingStudentReqs(int studentIndex) {
    if (studentIndex < 0 || studentIndex >= static_cast<int>(students.size())) return;
    Student* s = students[studentIndex];

    if (matchReqSubject) matchReqSubject->setText(joinStrings(s->getNeedSubjects()));
    if (matchReqGrade) matchReqGrade->setText(QString::fromStdString(s->getGrade()));
    if (matchReqArea) matchReqArea->setText(QString::fromStdString(s->getAddress()));
    if (matchReqBudget) matchReqBudget->setText(QString::fromStdString(s->getBudgetPerHour()) + QString::fromUtf8("đ/buổi"));

    if (matchReqScheduleContainer) {
        QString sched = QString::fromStdString(s->getAvailableSchedule());
        const QStringList dArr = {"T2", "T3", "T4", "T5", "T6", "T7", "CN"};
        for (const auto& item : dArr) {
            auto* dl = matchReqScheduleContainer->findChild<QLabel*>("day_" + item);
            if (dl) {
                bool act = sched.contains(item);
                dl->setStyleSheet(act
                    ? "background:#0284C7; color:white; border-radius:3px; font-size:10px; font-weight:700;"
                    : "background:#E2E8F0; color:#94A3B8; border-radius:3px; font-size:10px; font-weight:600;");
            }
        }
    }
}

void MainWindow::runSmartMatching() {
    if (!matchStudentCombo || matchStudentCombo->currentIndex() < 0 ||
        matchStudentCombo->currentIndex() >= static_cast<int>(students.size())) {
        QMessageBox::warning(this, QString::fromUtf8("Thông báo"), QString::fromUtf8("Vui lòng chọn học sinh cần tìm gia sư!"));
        return;
    }

    Student* s = students[matchStudentCombo->currentIndex()];
    Vector<MatchResult> results = MatchingService::matchStudentsToTutors(*s, tutors);

    if (results.empty()) {
        QMessageBox::information(this, QString::fromUtf8("Kết quả tìm kiếm"),
            QString::fromUtf8("Không tìm thấy gia sư phù hợp với môn học và điều kiện của học sinh %1!").arg(QString::fromStdString(s->getFullName())));
        currentMatchedStudent = nullptr;
        currentMatchedTutor = nullptr;
        return;
    }

    MatchResult top = results[0];
    currentMatchedStudent = s;
    currentMatchedTutor = top.tutor;

    if (candidateName) candidateName->setText(QString::fromStdString(currentMatchedTutor->getFullName()));
    if (candidateId) candidateId->setText(QString::fromStdString(currentMatchedTutor->getPersonID()));
    if (candidateDegree) candidateDegree->setText("🎓 " + QString::fromStdString(currentMatchedTutor->getDegrees()));

    int percent = min(100, max(0, static_cast<int>(top.totalScore + 0.5)));
    if (candidateDonutContainer) {
        DonutChartWidget* donut = nullptr;
        for (QWidget* child : candidateDonutContainer->findChildren<QWidget*>()) {
            donut = dynamic_cast<DonutChartWidget*>(child);
            if (donut) break;
        }
        if (donut) {
            donut->setPercent(percent, QString("%1%").arg(percent));
        }
    }

    if (candidateExp) candidateExp->setText(QString::number(currentMatchedTutor->getYearsOfExperience()) + QString::fromUtf8(" năm"));
    if (candidateArea) candidateArea->setText(joinStrings(currentMatchedTutor->getTeachingAreas()));
    if (candidateFee) candidateFee->setText(QString::fromStdString(currentMatchedTutor->getRatePerHour()) + QString::fromUtf8("đ/b"));
    if (candidateSchedule) candidateSchedule->setText(QString::fromStdString(currentMatchedTutor->getAvailability()));

    if (candidateScoreBreakdownContainer) {
        auto* expLbl = candidateScoreBreakdownContainer->findChild<QLabel*>("score_exp");
        if (expLbl) expLbl->setText(QString("%1/25").arg(static_cast<int>(top.experienceScore)));
        auto* pbExp = candidateScoreBreakdownContainer->findChild<QProgressBar*>("pb_score_exp");
        if (pbExp) pbExp->setValue(min(100, static_cast<int>(top.experienceScore * 4)));

        auto* locLbl = candidateScoreBreakdownContainer->findChild<QLabel*>("score_loc");
        if (locLbl) locLbl->setText(QString("%1/10").arg(static_cast<int>(top.locationScore)));
        auto* pbLoc = candidateScoreBreakdownContainer->findChild<QProgressBar*>("pb_score_loc");
        if (pbLoc) pbLoc->setValue(min(100, static_cast<int>(top.locationScore * 10)));

        auto* priceLbl = candidateScoreBreakdownContainer->findChild<QLabel*>("score_price");
        if (priceLbl) priceLbl->setText(QString("%1/45").arg(static_cast<int>(top.priceScore)));
        auto* pbPrice = candidateScoreBreakdownContainer->findChild<QProgressBar*>("pb_score_price");
        if (pbPrice) pbPrice->setValue(min(100, static_cast<int>(top.priceScore * 100 / 45)));

        auto* schedLbl = candidateScoreBreakdownContainer->findChild<QLabel*>("score_sched");
        if (schedLbl) schedLbl->setText(QString("%1/20").arg(static_cast<int>(top.scheduleScore)));
        auto* pbSched = candidateScoreBreakdownContainer->findChild<QProgressBar*>("pb_score_sched");
        if (pbSched) pbSched->setValue(min(100, static_cast<int>(top.scheduleScore * 5)));
    }

    if (candidateWhyContainer) {
        // Lý do lấy từ kết quả chấm điểm thật của MatchingService.
        const char* whyNames[3] = {"why_1", "why_2", "why_3"};
        for (size_t k = 0; k < 3; ++k) {
            auto* w = candidateWhyContainer->findChild<QLabel*>(whyNames[k]);
            if (!w) continue;
            if (k < top.reasons.size()) {
                w->setText(QString::fromUtf8("• ") + QString::fromStdString(top.reasons[k]));
            } else {
                w->setText(QString());
            }
        }
    }

    QMessageBox::information(this, QString::fromUtf8("Đã tìm thấy gia sư"),
        QString::fromUtf8("Thuật toán đã tìm thấy gia sư tối ưu nhất: %1 (Độ phù hợp %2%)")
            .arg(QString::fromStdString(currentMatchedTutor->getFullName()), QString::number(percent)));
}

void MainWindow::acceptCandidateMatch() {
    if (!currentMatchedStudent || !currentMatchedTutor) {
        QMessageBox::warning(this, QString::fromUtf8("Thông báo"), QString::fromUtf8("Vui lòng thực hiện tìm gia sư trước khi chọn!"));
        return;
    }

    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận ghép gia sư"),
        QString::fromUtf8("Bạn có đồng ý ghép gia sư %1 cho học sinh %2 và tự động tạo Lớp học + Hợp đồng mới?")
            .arg(QString::fromStdString(currentMatchedTutor->getFullName()), QString::fromStdString(currentMatchedStudent->getFullName())),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    currentMatchedStudent->setIsMatched(true);
    // Gia sư nhận được nhiều học sinh: chỉ đóng nhận khi đã đủ số lượng tối đa.
    currentMatchedTutor->setCurrentStudents(currentMatchedTutor->getCurrentStudents() + 1);
    if (currentMatchedTutor->getCurrentStudents() >= MatchingService::kMaxStudentsPerTutor) {
        currentMatchedTutor->setIsAvailable(false);
    }

    string classID = IdGenerator::nextFor("LH", classes, [](TutoringClass* x) { return x->getClassID(); });
    string contractID = IdGenerator::nextFor("HD", contracts, [](Contract* x) { return x->getContractID(); });
    string today = QDate::currentDate().toString("dd/MM/yyyy").toStdString();
    string endDay = QDate::currentDate().addMonths(3).toString("dd/MM/yyyy").toStdString();
    string subj = currentMatchedStudent->getGrade() + " - " + joinStrings(currentMatchedStudent->getNeedSubjects()).toStdString();

    auto* newClass = new TutoringClass(
        classID,
        currentMatchedStudent->getPersonID(),
        currentMatchedTutor->getPersonID(),
        subj,
        currentMatchedStudent->getAvailableSchedule(),
        today,
        endDay,
        "Đang học",
        currentMatchedTutor->getRatePerHour(),
        today,
        today
    );
    classes.push_back(newClass);

    long long fee = 2000000;
    try {
        if (!currentMatchedTutor->getRatePerHour().empty()) fee = stoll(currentMatchedTutor->getRatePerHour()) * 12;
    } catch (...) {}

    auto* newContract = new Contract(
        contractID,
        classID,
        currentMatchedStudent->getPersonID(),
        currentMatchedTutor->getPersonID(),
        today,
        today,
        endDay,
        to_string(fee),
        "Đang hiệu lực"
    );
    contracts.push_back(newContract);

    FileManager::saveStudents(students);
    FileManager::saveTutors(tutors);
    FileManager::saveClasses(classes);
    FileManager::saveContracts(contracts);

    refreshAll();

    QMessageBox::information(this, QString::fromUtf8("Ghép nối thành công"),
        QString::fromUtf8("Đã ghép thành công gia sư %1 cho học sinh %2!\n"
                          "Hệ thống đã tự động tạo Lớp học mã %3 và Hợp đồng mã %4.")
            .arg(QString::fromStdString(currentMatchedTutor->getFullName()),
                 QString::fromStdString(currentMatchedStudent->getFullName()),
                 QString::fromStdString(classID),
                 QString::fromStdString(contractID)));

    switchPage(4); // Switch to Class Page
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
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(16);

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#059669", "#0891B2");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    auto* pageTitle = label(QString::fromUtf8("Quản lý lớp học"), 22, true, "#0B1F4B");
    auto* pageDesc = label(QString::fromUtf8("Theo dõi lịch học, học sinh, gia sư và tiến độ lớp."), 13, false, "#5B6B8C");
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

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    classSearchEdit = addFilter(fl, QString::fromUtf8("🔍  Tìm mã lớp, học sinh, gia sư, môn..."), 280);
    connect(classSearchEdit, &QLineEdit::textChanged, this, [this]() { populateClassTable(); });

    classSubjectCombo = combo({QString::fromUtf8("Tất cả môn"), "Toán", "Lý", "Hóa", "Văn", "Anh"}, 140);
    connect(classSubjectCombo, &QComboBox::currentTextChanged, this, [this]() { populateClassTable(); });
    fl->addWidget(classSubjectCombo);

    classStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư")}, 160);
    connect(classStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateClassTable(); });
    fl->addWidget(classStatusCombo);
    l->addWidget(filters);

    {
        auto* tabRow = new QHBoxLayout;
        tabRow->setSpacing(10);
        classTabs = statusTabs(tabRow, classStatusCombo, QStringList{QString::fromUtf8("Tất cả"), QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư")});
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
    styleTable(classTable, "#059669");
    classTable->setMinimumHeight(350);

    tl->addWidget(classTable);
    tl->addWidget(new PagerBar(classTable, QString::fromUtf8("lớp học"), tableCard));
    l->addWidget(tableCard, 1);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateClassTable() {
    if (!classTable) return;
    QString kw = classSearchEdit ? classSearchEdit->text().trimmed().toLower() : "";
    QString subjFilter = classSubjectCombo ? classSubjectCombo->currentText() : "";
    QString statusFilter = classStatusCombo ? classStatusCombo->currentText() : "";

    Vector<TutoringClass*> filtered;
    for (size_t i = 0; i < classes.size(); ++i) {
        TutoringClass* c = classes[i];
        if (!kw.isEmpty()) {
            QString id = QString::fromStdString(c->getClassID()).toLower();
            QString sId = QString::fromStdString(c->getStudentID()).toLower();
            QString tId = QString::fromStdString(c->getTutorID()).toLower();
            QString sub = QString::fromStdString(c->getSubject()).toLower();

            Student* s = findStudent(c->getStudentID());
            Tutor* t = findTutor(c->getTutorID());
            QString sName = s ? QString::fromStdString(s->getFullName()).toLower() : "";
            QString tName = t ? QString::fromStdString(t->getFullName()).toLower() : "";

            if (!id.contains(kw) && !sId.contains(kw) && !tId.contains(kw) && !sub.contains(kw) &&
                !sName.contains(kw) && !tName.contains(kw)) {
                continue;
            }
        }
        if (!subjFilter.isEmpty() && subjFilter != QString::fromUtf8("Tất cả môn")) {
            QString sub = QString::fromStdString(c->getSubject());
            if (!sub.contains(subjFilter)) continue;
        }
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
        classTable->setCellWidget(row, 4, label(QString::fromStdString(c->getSchedule()), 12, false, TEXT_MUTED));
        classTable->setCellWidget(row, 5, label(QString::fromStdString(c->getRatePerHour()) + QString::fromUtf8("đ/b"), 12, true, BLUE));

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
        btnEdit->setStyleSheet("QPushButton { background:#EFF6FF; border:none; border-radius:16px; color:#0284C7; font-size:12px; } QPushButton:hover { background:#DBEAFE; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#DC2626; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

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
// 5. QUẢN LÝ HỢP ĐỒNG
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

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#EA580C", "#DC2626");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    tCol->addWidget(label(QString::fromUtf8("Quản lý hợp đồng"), 22, true, "#0B1F4B"));
    tCol->addWidget(label(QString::fromUtf8("Theo dõi toàn bộ hợp đồng giảng dạy và thanh toán."), 13, false, "#5B6B8C"));
    topLayout->addLayout(tCol, 1);
    auto* btnCreateContract = blueButton(QString::fromUtf8("📄⁺  Tạo hợp đồng"), topCard);
    connect(btnCreateContract, &QPushButton::clicked, this, &MainWindow::onAddContract);
    styleBannerButton(btnCreateContract);
    topLayout->addWidget(btnCreateContract);
    topLayout->addWidget(new BookIllustration(topCard));
    l->addWidget(topCard);

    auto* summary = new QHBoxLayout;
    summary->setSpacing(14);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Tổng hợp đồng"), contractSumTotal, "0", BLUE), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Đang hiệu lực"), contractSumActive, "0", GREEN), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Chờ xử lý"), contractSumPending, "0", "#F59E0B"), 1);
    summary->addWidget(infoTileWithLabel(QString::fromUtf8("Hoàn thành"), contractSumCompleted, "0", "#8B5CF6"), 1);
    l->addLayout(summary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    contractSearchEdit = addFilter(fl, QString::fromUtf8("🔍  Tìm mã hợp đồng, lớp, học sinh, gia sư..."), 280);
    connect(contractSearchEdit, &QLineEdit::textChanged, this, [this]() { populateContractTable(); });

    contractStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Đang hiệu lực"), QString::fromUtf8("Chờ xử lý"), QString::fromUtf8("Hoàn thành")}, 170);
    connect(contractStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateContractTable(); });
    fl->addWidget(contractStatusCombo);
    l->addWidget(filters);

    {
        auto* tabRow = new QHBoxLayout;
        tabRow->setSpacing(10);
        contractTabs = statusTabs(tabRow, contractStatusCombo, QStringList{QString::fromUtf8("Tất cả"), QString::fromUtf8("Đang hiệu lực"), QString::fromUtf8("Chờ xử lý"), QString::fromUtf8("Hoàn thành")});
        tabRow->addStretch();
        l->addLayout(tabRow);
    }

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    contractTable = new QTableWidget(0, 8, tableCard);
    contractTable->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ HĐ"),
        QString::fromUtf8("MÃ LỚP"),
        QString::fromUtf8("HỌC SINH"),
        QString::fromUtf8("GIA SƯ"),
        QString::fromUtf8("BẮT ĐẦU"),
        QString::fromUtf8("TỔNG PHÍ"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(contractTable, "#EA580C");
    contractTable->setMinimumHeight(350);

    tl->addWidget(contractTable);
    tl->addWidget(new PagerBar(contractTable, QString::fromUtf8("hợp đồng"), tableCard));
    l->addWidget(tableCard, 1);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateContractTable() {
    if (!contractTable) return;
    QString kw = contractSearchEdit ? contractSearchEdit->text().trimmed().toLower() : "";
    QString statusFilter = contractStatusCombo ? contractStatusCombo->currentText() : "";

    Vector<Contract*> filtered;
    for (size_t i = 0; i < contracts.size(); ++i) {
        Contract* c = contracts[i];
        if (!kw.isEmpty()) {
            QString id = QString::fromStdString(c->getContractID()).toLower();
            QString clId = QString::fromStdString(c->getClassID()).toLower();
            Student* s = findStudent(c->getStudentID());
            Tutor* t = findTutor(c->getTutorID());
            QString sName = s ? QString::fromStdString(s->getFullName()).toLower() : "";
            QString tName = t ? QString::fromStdString(t->getFullName()).toLower() : "";

            if (!id.contains(kw) && !clId.contains(kw) && !sName.contains(kw) && !tName.contains(kw)) {
                continue;
            }
        }
        if (!statusFilter.isEmpty() && statusFilter != QString::fromUtf8("Tất cả trạng thái")) {
            if (QString::fromStdString(c->getStatus()) != statusFilter) continue;
        }

        filtered.push_back(c);
    }

    contractTable->setRowCount(static_cast<int>(filtered.size()));
    for (size_t i = 0; i < filtered.size(); ++i) {
        Contract* c = filtered[i];
        int row = static_cast<int>(i);

        contractTable->setCellWidget(row, 0, label(QString::fromStdString(c->getContractID()), 12, true, BLUE));
        contractTable->setCellWidget(row, 1, label(QString::fromStdString(c->getClassID()), 12, false, TEXT_MAIN));

        Student* s = findStudent(c->getStudentID());
        QString sName = s ? QString::fromStdString(s->getFullName()) : QString::fromStdString(c->getStudentID());
        contractTable->setCellWidget(row, 2, label(sName, 12, true, TEXT_MAIN));

        Tutor* t = findTutor(c->getTutorID());
        QString tName = t ? QString::fromStdString(t->getFullName()) : (c->getTutorID().empty() ? QString::fromUtf8("Chưa có") : QString::fromStdString(c->getTutorID()));
        contractTable->setCellWidget(row, 3, label(tName, 12, false, TEXT_SUBTLE));

        contractTable->setCellWidget(row, 4, label(QString::fromStdString(c->getStartDate()), 12, false, TEXT_MUTED));
        contractTable->setCellWidget(row, 5, label(QString::fromStdString(c->getTotalFee()) + "đ", 12, true, BLUE));

        QString st = QString::fromStdString(c->getStatus());
        QString bg = st == "Đang hiệu lực" ? GREEN_BG : (st == "Chờ xử lý" ? PEACH_BG : BLUE_BG);
        QString fg = st == "Đang hiệu lực" ? GREEN : (st == "Chờ xử lý" ? PEACH_TEXT : BLUE);
        contractTable->setCellWidget(row, 6, pillBadge(st, bg, fg));

        auto* actW = new QWidget;
        auto* actL = new QHBoxLayout(actW);
        actL->setContentsMargins(4, 2, 4, 2);
        actL->setSpacing(4);

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#EFF6FF; border:none; border-radius:16px; color:#0284C7; font-size:12px; } QPushButton:hover { background:#DBEAFE; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#DC2626; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

        actL->addWidget(btnEdit);
        actL->addWidget(btnDel);
        actL->addStretch();
        contractTable->setCellWidget(row, 7, actW);

        connect(btnEdit, &QPushButton::clicked, this, [this, c]() { onEditContract(c); });
        connect(btnDel, &QPushButton::clicked, this, [this, c]() { onDeleteContract(c); });
    }
    fitColumns(contractTable);
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
    scroll->setStyleSheet(QString("QScrollArea { background:%1; border:none; }").arg(BG));

    auto* content = new QWidget;
    content->setStyleSheet(QString("background:%1;").arg(BG));
    auto* l = new QVBoxLayout(content);
    l->setContentsMargins(28, 22, 28, 24);
    l->setSpacing(18);

    auto* titleRow = new QHBoxLayout;
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(2);
    tCol->addWidget(label(QString::fromUtf8("Thống kê hệ thống"), 24, true, TEXT_MAIN));
    tCol->addWidget(label(QString::fromUtf8("Tổng quan dữ liệu hoạt động của nền tảng quản lý gia sư. Các chỉ số được cập nhật theo thời gian thực để hỗ trợ ra quyết định nhanh chóng."), 13, false, TEXT_MUTED));
    titleRow->addLayout(tCol, 1);

    auto* refreshBtn = new QPushButton(QString::fromUtf8("🔄  Làm mới"), content);
    refreshBtn->setFixedHeight(38);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { background:#0F172A; color:white; border:none; border-radius:6px; "
        "padding:0 18px; font-size:12px; font-weight:600; } QPushButton:hover { background:#1E293B; }");
    connect(refreshBtn, &QPushButton::clicked, this, [this]() { refreshAll(); });
    titleRow->addWidget(refreshBtn);
    l->addLayout(titleRow);

    auto* metrics = new QHBoxLayout;
    metrics->setSpacing(12);

    auto makeStatCardWithLbl = [](const QString& iconEmoji, const QString& trend, QLabel*& valOut,
                                   const QString& initVal, const QString& sub, const QString& iconBg = "#DBEAFE",
                                   const QString& iconFg = "#0284C7", bool isDark = false) {
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

        valOut = label(initVal, 24, true, isDark ? "#FFFFFF" : TEXT_MAIN);
        auto* desc = label(sub, 12, false, isDark ? "#94A3B8" : TEXT_MUTED);
        l->addWidget(valOut);
        l->addWidget(desc);
        return f;
    };

    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("👥"), "Dữ liệu", statTotalStudents, "0", QString::fromUtf8("Tổng học sinh"), "#DBEAFE", "#0284C7"), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("👨‍🏫"), "Dữ liệu", statTotalTutors, "0", QString::fromUtf8("Tổng gia sư"), "#0F172A", "#FFFFFF"), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("👤"), QString::fromUtf8("● LIVE"), statAvailTutors, "0", QString::fromUtf8("Gia sư sẵn sàng nhận lớp"), "#111E38", "#38BDF8", true), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("📖"), "Dữ liệu", statActiveClasses, "0", QString::fromUtf8("Lớp đang hoạt động"), "#DBEAFE", "#0284C7"), 1);
    metrics->addWidget(makeStatCardWithLbl(QString::fromUtf8("⚙"), "Tự động", statMatchRate, "0%", QString::fromUtf8("Tỷ lệ ghép thành công"), "#FEF3C7", "#D97706"), 1);

    l->addLayout(metrics);

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
        "Tất cả chỉ số đều được đồng bộ trực tiếp từ các thực thể trong cơ sở dữ liệu hệ thống (học sinh, gia sư, lớp học, hợp đồng và đăng ký môn). "
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
    statChartGrade = new BarChartWidget(QColor("#0284C7"));
    chartRow1->addWidget(chartCard(QString::fromUtf8("Tình trạng ghép học sinh"),
                                   QString::fromUtf8("Đã ghép so với chưa ghép"), statChartMatch), 1);
    chartRow1->addWidget(chartCard(QString::fromUtf8("Trạng thái lớp học"),
                                   QString::fromUtf8("Phân bổ lớp theo tiến độ"), statChartClass), 1);
    chartRow1->addWidget(chartCard(QString::fromUtf8("Học sinh theo khối lớp"),
                                   QString::fromUtf8("Số học sinh mỗi khối"), statChartGrade), 1);
    l->addLayout(chartRow1);

    auto* chartRow2 = new QHBoxLayout;
    chartRow2->setSpacing(14);
    statChartSubject = new BarChartWidget(QColor("#0F172A"));
    statChartMonthly = new LineChartWidget;
    chartRow2->addWidget(chartCard(QString::fromUtf8("Gia sư theo môn dạy"),
                                   QString::fromUtf8("Các môn có nhiều gia sư nhất"), statChartSubject), 1);
    chartRow2->addWidget(chartCard(QString::fromUtf8("Lớp học mới theo tháng"),
                                   QString::fromUtf8("Xu hướng mở lớp theo ngày bắt đầu"), statChartMonthly), 1);
    l->addLayout(chartRow2);
    l->addStretch();

    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

// -------------------------------------------------------------
// 7. QUẢN LÝ ĐĂNG KÝ MÔN HỌC
// -------------------------------------------------------------
QWidget* MainWindow::createRegistrationPage() {
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

    l->addWidget(welcomeStrip(welcomeLabels));
    auto* topCard = new QFrame(content);
    applyBanner(topCard, "#0D9488", "#2563EB");
    auto* topLayout = new QHBoxLayout(topCard);
    topLayout->setContentsMargins(24, 18, 24, 18);
    auto* tCol = new QVBoxLayout;
    tCol->setSpacing(4);
    tCol->addWidget(label(QString::fromUtf8("Đăng ký môn học"), 22, true, "#0B1F4B"));
    tCol->addWidget(label(QString::fromUtf8("Quản lý phiếu đăng ký môn học của sinh viên - 1 sinh viên có thể đăng ký nhiều môn."), 13, false, "#5B6B8C"));
    topLayout->addLayout(tCol, 1);
    auto* btnNewReg = blueButton(QString::fromUtf8("📝⁺  Đăng ký môn mới"), topCard);
    connect(btnNewReg, &QPushButton::clicked, this, &MainWindow::onAddRegistration);
    styleBannerButton(btnNewReg);
    topLayout->addWidget(btnNewReg);
    topLayout->addWidget(new BookIllustration(topCard));
    l->addWidget(topCard);

    auto* regSummary = new QHBoxLayout;
    regSummary->setSpacing(14);
    regSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Tổng phiếu đăng ký"), regSumTotal, "0", "#0D9488"), 1);
    regSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Chờ ghép gia sư"), regSumPending, "0", "#F59E0B"), 1);
    regSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Đã duyệt"), regSumApproved, "0", "#2563EB"), 1);
    regSummary->addWidget(infoTileWithLabel(QString::fromUtf8("Đã ghép"), regSumMatched, "0", GREEN), 1);
    l->addLayout(regSummary);

    auto* filters = card();
    auto* fl = new QHBoxLayout(filters);
    fl->setContentsMargins(14, 10, 14, 10);
    fl->setSpacing(12);
    regSearchEdit = addFilter(fl, QString::fromUtf8("🔍  Tìm kiếm theo mã phiếu, mã SV, tên SV, môn học..."), 300);
    connect(regSearchEdit, &QLineEdit::textChanged, this, [this]() { populateRegistrationTable(); });

    regStatusCombo = combo({QString::fromUtf8("Tất cả trạng thái"), QString::fromUtf8("Cho ghep gia su"), QString::fromUtf8("Da duyet"), QString::fromUtf8("Đã ghép")}, 180);
    connect(regStatusCombo, &QComboBox::currentTextChanged, this, [this]() { populateRegistrationTable(); });
    fl->addWidget(regStatusCombo);
    l->addWidget(filters);

    {
        auto* tabRow = new QHBoxLayout;
        tabRow->setSpacing(10);
        regTabs = statusTabs(tabRow, regStatusCombo, QStringList{QString::fromUtf8("Tất cả"), QString::fromUtf8("Chờ ghép"), QString::fromUtf8("Đã duyệt"), QString::fromUtf8("Đã ghép")});
        tabRow->addStretch();
        l->addLayout(tabRow);
    }

    auto* tableCard = card();
    auto* tl = new QVBoxLayout(tableCard);
    tl->setContentsMargins(16, 14, 16, 14);

    registrationTable = new QTableWidget(0, 9, tableCard);
    registrationTable->setHorizontalHeaderLabels({
        QString::fromUtf8("MÃ PHIẾU"),
        QString::fromUtf8("MÃ SV"),
        QString::fromUtf8("HỌ TÊN"),
        QString::fromUtf8("CÁC MÔN ĐĂNG KÝ"),
        QString::fromUtf8("SỐ MÔN"),
        QString::fromUtf8("NGÀY ĐK"),
        QString::fromUtf8("TRẠNG THÁI"),
        QString::fromUtf8("GHI CHÚ"),
        QString::fromUtf8("THAO TÁC")
    });
    styleTable(registrationTable, "#0D9488");
    registrationTable->setMinimumHeight(350);

    tl->addWidget(registrationTable);
    tl->addWidget(new PagerBar(registrationTable, QString::fromUtf8("phiếu đăng ký"), tableCard));
    l->addWidget(tableCard, 1);

    l->addStretch();
    scroll->setWidget(content);
    root->addWidget(scroll, 1);
    return page;
}

void MainWindow::populateRegistrationTable() {
    if (!registrationTable) return;
    QString kw = regSearchEdit ? regSearchEdit->text().trimmed().toLower() : "";
    QString statusFilter = regStatusCombo ? regStatusCombo->currentText() : "";

    Vector<Registration*> filtered;
    for (size_t i = 0; i < registrations.size(); ++i) {
        Registration* r = registrations[i];
        if (!kw.isEmpty()) {
            QString regId = QString::fromStdString(r->getRegistrationID()).toLower();
            QString sId = QString::fromStdString(r->getStudentID()).toLower();
            QString sName = QString::fromStdString(r->getStudentName()).toLower();
            QString sub = QString::fromStdString(r->getSubjectsSummary()).toLower();
            if (!regId.contains(kw) && !sId.contains(kw) && !sName.contains(kw) && !sub.contains(kw)) {
                continue;
            }
        }
        if (!statusFilter.isEmpty() && statusFilter != QString::fromUtf8("Tất cả trạng thái")) {
            if (QString::fromStdString(r->getStatus()) != statusFilter) continue;
        }

        filtered.push_back(r);
    }

    registrationTable->setRowCount(static_cast<int>(filtered.size()));
    for (size_t i = 0; i < filtered.size(); ++i) {
        Registration* reg = filtered[i];
        int row = static_cast<int>(i);

        registrationTable->setCellWidget(row, 0, label(QString::fromStdString(reg->getRegistrationID()), 12, true, BLUE));
        registrationTable->setCellWidget(row, 1, label(QString::fromStdString(reg->getStudentID()), 12, false, TEXT_MUTED));
        registrationTable->setCellWidget(row, 2, label(QString::fromStdString(reg->getStudentName()), 12, true, TEXT_MAIN));
        registrationTable->setCellWidget(row, 3, label(QString::fromStdString(reg->getSubjectsSummary()), 12, false, TEXT_MAIN));
        registrationTable->setCellWidget(row, 4, label(QString::number(reg->getSubjectCount()) + QString::fromUtf8(" môn"), 12, true, BLUE));
        registrationTable->setCellWidget(row, 5, label(QString::fromStdString(reg->getRegistrationDate()), 12, false, TEXT_MUTED));

        QString st = QString::fromStdString(reg->getStatus());
        registrationTable->setCellWidget(row, 6, pillBadge(st, st == "Da duyet" ? GREEN_BG : PEACH_BG, st == "Da duyet" ? GREEN : PEACH_TEXT));
        registrationTable->setCellWidget(row, 7, label(QString::fromStdString(reg->getNotes()), 12, false, TEXT_MUTED));

        auto* actW = new QWidget;
        auto* actL = new QHBoxLayout(actW);
        actL->setContentsMargins(4, 2, 4, 2);
        actL->setSpacing(4);

        auto* btnEdit = new QPushButton(QString::fromUtf8("✎"), actW);
        btnEdit->setToolTip(QString::fromUtf8("Chỉnh sửa"));
        btnEdit->setFixedSize(32, 32);
        btnEdit->setStyleSheet("QPushButton { background:#EFF6FF; border:none; border-radius:16px; color:#0284C7; font-size:12px; } QPushButton:hover { background:#DBEAFE; }");

        auto* btnDel = new QPushButton(QString::fromUtf8("🗑"), actW);
        btnDel->setToolTip(QString::fromUtf8("Xóa"));
        btnDel->setFixedSize(32, 32);
        btnDel->setStyleSheet("QPushButton { background:#FEF2F2; border:none; border-radius:16px; color:#DC2626; font-size:12px; } QPushButton:hover { background:#FEE2E2; }");

        actL->addWidget(btnEdit);
        actL->addWidget(btnDel);
        actL->addStretch();
        registrationTable->setCellWidget(row, 8, actW);

        connect(btnEdit, &QPushButton::clicked, this, [this, reg]() { onEditRegistration(reg); });
        connect(btnDel, &QPushButton::clicked, this, [this, reg]() { onDeleteRegistration(reg); });
    }
    fitColumns(registrationTable);
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
    for (size_t i = 0; i < tutors.size(); ++i) if (tutors[i]->getIsAvailable()) ++avail;
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
    for (size_t i = 0; i < tutors.size(); ++i) if (tutors[i]->getIsAvailable()) ++avail;
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

    if (statChartMatch) {
        static_cast<SegmentDonutWidget*>(statChartMatch)->setData(
            QStringList{QString::fromUtf8("Đã ghép"), QString::fromUtf8("Chưa ghép")},
            QVector<double>{double(matched), double(totalS - matched)},
            QVector<QColor>{QColor("#0284C7"), QColor("#CBD5E1")}, QString::fromUtf8("học sinh"));
    }
    if (statChartClass) {
        static_cast<SegmentDonutWidget*>(statChartClass)->setData(
            QStringList{QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"),
                        QString::fromUtf8("Cần gia sư"), QString::fromUtf8("Khác")},
            QVector<double>{double(active), double(pending), double(needTutor), double(other)},
            QVector<QColor>{QColor("#16A34A"), QColor("#F59E0B"), QColor("#DC2626"), QColor("#94A3B8")},
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
        if (tutors[i]->getIsAvailable()) ++avail;
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

void MainWindow::refreshContractSummary() {
    int active = 0, pending = 0, done = 0;
    for (size_t i = 0; i < contracts.size(); ++i) {
        QString st = toQ(contracts[i]->getStatus());
        if (st == QString::fromUtf8("Đang hiệu lực")) ++active;
        else if (st == QString::fromUtf8("Chờ xử lý")) ++pending;
        else if (st == QString::fromUtf8("Hoàn thành")) ++done;
    }
    if (contractSumTotal) contractSumTotal->setText(QString::number(static_cast<int>(contracts.size())));
    if (contractSumActive) contractSumActive->setText(QString::number(active));
    if (contractSumPending) contractSumPending->setText(QString::number(pending));
    if (contractSumCompleted) contractSumCompleted->setText(QString::number(done));
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

void MainWindow::refreshRegistrationSummary() {
    int pending = 0, approved = 0, matched = 0;
    for (size_t i = 0; i < registrations.size(); ++i) {
        QString st = toQ(registrations[i]->getStatus());
        if (st == "Cho ghep gia su") ++pending;
        else if (st == "Da duyet") ++approved;
        else if (st == QString::fromUtf8("Đã ghép")) ++matched;
    }
    if (regSumTotal) regSumTotal->setText(QString::number(static_cast<int>(registrations.size())));
    if (regSumPending) regSumPending->setText(QString::number(pending));
    if (regSumApproved) regSumApproved->setText(QString::number(approved));
    if (regSumMatched) regSumMatched->setText(QString::number(matched));
}

void MainWindow::refreshOverviewWidgets() {
    int totalS = static_cast<int>(students.size());
    int totalT = static_cast<int>(tutors.size());
    int matched = 0, avail = 0;
    int cActive = 0, cPending = 0, cNeed = 0;
    int kActive = 0, kPending = 0, kDone = 0;
    int rPending = 0, rApproved = 0, rMatched = 0;
    for (size_t i = 0; i < students.size(); ++i) if (students[i]->getIsMatched()) ++matched;
    for (size_t i = 0; i < tutors.size(); ++i) if (tutors[i]->getIsAvailable()) ++avail;
    for (size_t i = 0; i < classes.size(); ++i) {
        QString st = toQ(classes[i]->getStatus());
        if (st == QString::fromUtf8("Đang học")) ++cActive;
        else if (st == QString::fromUtf8("Chờ xếp lịch")) ++cPending;
        if (st == QString::fromUtf8("Cần gia sư") || classes[i]->getTutorID().empty()) ++cNeed;
    }
    for (size_t i = 0; i < contracts.size(); ++i) {
        QString st = toQ(contracts[i]->getStatus());
        if (st == QString::fromUtf8("Đang hiệu lực")) ++kActive;
        else if (st == QString::fromUtf8("Chờ xử lý")) ++kPending;
        else if (st == QString::fromUtf8("Hoàn thành")) ++kDone;
    }
    for (size_t i = 0; i < registrations.size(); ++i) {
        QString st = toQ(registrations[i]->getStatus());
        if (st == "Cho ghep gia su") ++rPending;
        else if (st == "Da duyet") ++rApproved;
        else if (st == QString::fromUtf8("Đã ghép")) ++rMatched;
    }
    const int vals[4] = {totalS, totalT, cActive, kActive};
    for (int i = 0; i < welcomeLabels.size(); ++i) {
        if (welcomeLabels[i]) welcomeLabels[i]->setText(QString::number(vals[i % 4]));
    }
    setTabCounts(stuTabs, {totalS, matched, totalS - matched});
    setTabCounts(tutorTabs, {totalT, avail, totalT - avail});
    setTabCounts(classTabs, {static_cast<int>(classes.size()), cActive, cPending, cNeed});
    setTabCounts(contractTabs, {static_cast<int>(contracts.size()), kActive, kPending, kDone});
    setTabCounts(regTabs, {static_cast<int>(registrations.size()), rPending, rApproved, rMatched});
}

void MainWindow::refreshAll() {
    populateStudentTable();
    populateTutorTable();
    populateClassTable();
    populateContractTable();
    populateRegistrationTable();
    populateDashboardRecentTable();
    refreshDashboardMetrics();
    refreshStatisticsMetrics();
    refreshTutorSummary();
    refreshClassSummary();
    refreshContractSummary();
    refreshStudentSummary();
    refreshRegistrationSummary();
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
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("SV", students, [](Student* x) { return x->getPersonID(); });
    auto* txtId = new QLineEdit(QString::fromStdString(nextId), &dialog);
    auto* txtName = new QLineEdit(&dialog);
    auto* txtPhone = new QLineEdit(&dialog);
    auto* txtEmail = new QLineEdit(&dialog);
    auto* txtAddress = new QLineEdit(&dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    auto* txtDob = new QLineEdit("10/05/2010", &dialog);
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
    form->addRow(label(QString::fromUtf8("Email:")), txtEmail);
    form->addRow(label(QString::fromUtf8("Khu vực / Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh:")), txtDob);
    form->addRow(label(QString::fromUtf8("Khối lớp:")), cbGrade);
    form->addRow(label(QString::fromUtf8("Môn cần học *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Học lực:")), cbAcademic);
    form->addRow(label(QString::fromUtf8("Ngân sách / buổi:")), txtBudget);
    form->addRow(label(QString::fromUtf8("Ghi chú:")), txtNotes);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu học sinh"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        if (txtName->text().trimmed().isEmpty() || txtPhone->text().trimmed().isEmpty() || txtSubjects->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Thông báo"), QString::fromUtf8("Vui lòng điền đầy đủ Họ tên, SĐT và Môn học!"));
            return;
        }

        {
            QString err = validatePersonInput(txtPhone->text(), txtEmail->text(), txtDob->text(), txtBudget->text(), QString::fromUtf8("Ngân sách"));
            if (err.isEmpty() && txtId->text().trimmed().isEmpty()) {
                err = QString::fromUtf8("Mã không được để trống.");
            }
            if (err.isEmpty() && findStudent(txtId->text().trimmed().toStdString()) != nullptr) {
                err = QString::fromUtf8("Mã này đã tồn tại, vui lòng chọn mã khác.");
            }
            if (!err.isEmpty()) {
                QMessageBox::warning(&dialog, QString::fromUtf8("Dữ liệu chưa hợp lệ"), err);
                return;
            }
        }

        Vector<string> subList = FileManager::splitString(txtSubjects->text().toStdString(), ',');
        auto* s = new Student(
            txtId->text().trimmed().toStdString(),
            txtName->text().trimmed().toStdString(),
            txtPhone->text().trimmed().toStdString(),
            txtEmail->text().trimmed().toStdString(),
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
        FileManager::saveStudents(students);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã thêm học sinh mới thành công!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onEditStudent(Student* s) {
    if (!s) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa học sinh: ") + QString::fromStdString(s->getFullName()));
    dialog.resize(500, 560);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(s->getPersonID()), &dialog);
    txtId->setEnabled(false);
    auto* txtName = new QLineEdit(QString::fromStdString(s->getFullName()), &dialog);
    auto* txtPhone = new QLineEdit(QString::fromStdString(s->getPhone()), &dialog);
    auto* txtEmail = new QLineEdit(QString::fromStdString(s->getEmail()), &dialog);
    auto* txtAddress = new QLineEdit(QString::fromStdString(s->getAddress()), &dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    cbGender->setCurrentText(QString::fromStdString(s->getGender()));
    auto* txtDob = new QLineEdit(QString::fromStdString(s->getDateOfBirth()), &dialog);
    auto* cbGrade = new QComboBox(&dialog);
    cbGrade->addItems({"10", "11", "12", "9", "8", "7", "6", "5"});
    cbGrade->setCurrentText(QString::fromStdString(s->getGrade()));
    auto* txtSubjects = new QLineEdit(joinStrings(s->getNeedSubjects()), &dialog);
    auto* txtSchedule = new QLineEdit(QString::fromStdString(s->getAvailableSchedule()), &dialog);
    auto* cbAcademic = new QComboBox(&dialog);
    cbAcademic->addItems({QString::fromUtf8("Trung Bình"), QString::fromUtf8("Khá"), QString::fromUtf8("Giỏi")});
    cbAcademic->setCurrentText(QString::fromStdString(s->getLearningLevel()));
    auto* txtBudget = new QLineEdit(QString::fromStdString(s->getBudgetPerHour()), &dialog);
    auto* txtNotes = new QLineEdit(QString::fromStdString(s->getNotes()), &dialog);

    form->addRow(label(QString::fromUtf8("Mã học sinh:")), txtId);
    form->addRow(label(QString::fromUtf8("Họ và tên *:")), txtName);
    form->addRow(label(QString::fromUtf8("Số điện thoại *:")), txtPhone);
    form->addRow(label(QString::fromUtf8("Email:")), txtEmail);
    form->addRow(label(QString::fromUtf8("Khu vực / Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh:")), txtDob);
    form->addRow(label(QString::fromUtf8("Khối lớp:")), cbGrade);
    form->addRow(label(QString::fromUtf8("Môn cần học *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Học lực:")), cbAcademic);
    form->addRow(label(QString::fromUtf8("Ngân sách / buổi:")), txtBudget);
    form->addRow(label(QString::fromUtf8("Ghi chú:")), txtNotes);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        {
            QString err = validatePersonInput(txtPhone->text(), txtEmail->text(), txtDob->text(), txtBudget->text(), QString::fromUtf8("Ngân sách"));
            if (!err.isEmpty()) {
                QMessageBox::warning(&dialog, QString::fromUtf8("Dữ liệu chưa hợp lệ"), err);
                return;
            }
        }

        s->setFullName(txtName->text().trimmed().toStdString());
        s->setPhone(txtPhone->text().trimmed().toStdString());
        s->setEmail(txtEmail->text().trimmed().toStdString());
        s->setAddress(txtAddress->text().trimmed().toStdString());
        s->setGender(cbGender->currentText().toStdString());
        s->setDateOfBirth(txtDob->text().trimmed().toStdString());
        s->setGrade(cbGrade->currentText().toStdString());
        s->setNeedSubjects(FileManager::splitString(txtSubjects->text().toStdString(), ','));
        s->setAvailableSchedule(txtSchedule->text().trimmed().toStdString());
        s->setLearningLevel(cbAcademic->currentText().toStdString());
        s->setBudgetPerHour(txtBudget->text().trimmed().toStdString());
        s->setNotes(txtNotes->text().trimmed().toStdString());

        FileManager::saveStudents(students);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin học sinh!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteStudent(Student* s) {
    if (!s) return;
    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
        QString::fromUtf8("Bạn có chắc chắn muốn xóa học sinh %1 (%2)?")
            .arg(QString::fromStdString(s->getFullName()), QString::fromStdString(s->getPersonID())),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (size_t i = 0; i < students.size(); ++i) {
            if (students[i] == s) {
                delete students[i];
                students.erase(i);
                break;
            }
        }
        FileManager::saveStudents(students);
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã xóa học sinh thành công!"));
    }
}

void MainWindow::onViewStudent(Student* s) {
    if (!s) return;
    QString info = QString::fromUtf8(
        "Mã học sinh: %1\n"
        "Họ và tên: %2\n"
        "Số điện thoại: %3\n"
        "Email: %4\n"
        "Địa chỉ / Khu vực: %5\n"
        "Giới tính: %6\n"
        "Ngày sinh: %7\n"
        "Khối lớp: %8\n"
        "Môn học cần tìm: %9\n"
        "Lịch học rảnh: %10\n"
        "Học lực: %11\n"
        "Ngân sách: %12 đ/buổi\n"
        "Trạng thái: %13\n"
        "Ghi chú: %14"
    ).arg(
        QString::fromStdString(s->getPersonID()),
        QString::fromStdString(s->getFullName()),
        QString::fromStdString(s->getPhone()),
        QString::fromStdString(s->getEmail()),
        QString::fromStdString(s->getAddress()),
        QString::fromStdString(s->getGender()),
        QString::fromStdString(s->getDateOfBirth()),
        QString::fromStdString(s->getGrade()),
        joinStrings(s->getNeedSubjects()),
        QString::fromStdString(s->getAvailableSchedule()),
        QString::fromStdString(s->getLearningLevel()),
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
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox, QSpinBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("GS", tutors, [](Tutor* x) { return x->getPersonID(); });
    auto* txtId = new QLineEdit(QString::fromStdString(nextId), &dialog);
    auto* txtName = new QLineEdit(&dialog);
    auto* txtPhone = new QLineEdit(&dialog);
    auto* txtEmail = new QLineEdit(&dialog);
    auto* txtAddress = new QLineEdit(&dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    auto* txtDob = new QLineEdit("10/05/1998", &dialog);
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
    form->addRow(label(QString::fromUtf8("Email:")), txtEmail);
    form->addRow(label(QString::fromUtf8("Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh:")), txtDob);
    form->addRow(label(QString::fromUtf8("Môn dạy *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Trình độ / Trường:")), txtUni);
    form->addRow(label(QString::fromUtf8("Số năm kinh nghiệm:")), spExp);
    form->addRow(label(QString::fromUtf8("Học phí / buổi:")), txtFee);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Khu vực dạy:")), txtAreas);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu gia sư"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        if (txtName->text().trimmed().isEmpty() || txtPhone->text().trimmed().isEmpty() || txtSubjects->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Thông báo"), QString::fromUtf8("Vui lòng nhập Họ tên, SĐT và Môn dạy!"));
            return;
        }

        {
            QString err = validatePersonInput(txtPhone->text(), txtEmail->text(), txtDob->text(), txtFee->text(), QString::fromUtf8("Học phí"));
            if (err.isEmpty() && txtId->text().trimmed().isEmpty()) {
                err = QString::fromUtf8("Mã không được để trống.");
            }
            if (err.isEmpty() && findTutor(txtId->text().trimmed().toStdString()) != nullptr) {
                err = QString::fromUtf8("Mã này đã tồn tại, vui lòng chọn mã khác.");
            }
            if (!err.isEmpty()) {
                QMessageBox::warning(&dialog, QString::fromUtf8("Dữ liệu chưa hợp lệ"), err);
                return;
            }
        }

        Vector<string> subList = FileManager::splitString(txtSubjects->text().toStdString(), ',');
        Vector<string> areaList = FileManager::splitString(txtAreas->text().toStdString(), ',');

        auto* t = new Tutor(
            txtId->text().trimmed().toStdString(),
            txtName->text().trimmed().toStdString(),
            txtPhone->text().trimmed().toStdString(),
            txtEmail->text().trimmed().toStdString(),
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
        FileManager::saveTutors(tutors);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã thêm gia sư mới thành công!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onEditTutor(Tutor* t) {
    if (!t) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa gia sư: ") + QString::fromStdString(t->getFullName()));
    dialog.resize(500, 580);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox, QSpinBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(t->getPersonID()), &dialog);
    txtId->setEnabled(false);
    auto* txtName = new QLineEdit(QString::fromStdString(t->getFullName()), &dialog);
    auto* txtPhone = new QLineEdit(QString::fromStdString(t->getPhone()), &dialog);
    auto* txtEmail = new QLineEdit(QString::fromStdString(t->getEmail()), &dialog);
    auto* txtAddress = new QLineEdit(QString::fromStdString(t->getAddress()), &dialog);
    auto* cbGender = new QComboBox(&dialog);
    cbGender->addItems({"Nam", QString::fromUtf8("Nữ")});
    cbGender->setCurrentText(QString::fromStdString(t->getGender()));
    auto* txtDob = new QLineEdit(QString::fromStdString(t->getDateOfBirth()), &dialog);
    auto* txtSubjects = new QLineEdit(joinStrings(t->getSubjects()), &dialog);
    auto* txtUni = new QLineEdit(QString::fromStdString(t->getDegrees()), &dialog);
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
    form->addRow(label(QString::fromUtf8("Email:")), txtEmail);
    form->addRow(label(QString::fromUtf8("Địa chỉ:")), txtAddress);
    form->addRow(label(QString::fromUtf8("Giới tính:")), cbGender);
    form->addRow(label(QString::fromUtf8("Ngày sinh:")), txtDob);
    form->addRow(label(QString::fromUtf8("Môn dạy *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Trình độ / Trường:")), txtUni);
    form->addRow(label(QString::fromUtf8("Số năm kinh nghiệm:")), spExp);
    form->addRow(label(QString::fromUtf8("Học phí / buổi:")), txtFee);
    form->addRow(label(QString::fromUtf8("Lịch rảnh:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Khu vực dạy:")), txtAreas);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbAvail);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        {
            QString err = validatePersonInput(txtPhone->text(), txtEmail->text(), txtDob->text(), txtFee->text(), QString::fromUtf8("Học phí"));
            if (!err.isEmpty()) {
                QMessageBox::warning(&dialog, QString::fromUtf8("Dữ liệu chưa hợp lệ"), err);
                return;
            }
        }

        t->setFullName(txtName->text().trimmed().toStdString());
        t->setPhone(txtPhone->text().trimmed().toStdString());
        t->setEmail(txtEmail->text().trimmed().toStdString());
        t->setAddress(txtAddress->text().trimmed().toStdString());
        t->setGender(cbGender->currentText().toStdString());
        t->setDateOfBirth(txtDob->text().trimmed().toStdString());
        t->setSubjects(FileManager::splitString(txtSubjects->text().toStdString(), ','));
        t->setDegrees(txtUni->text().trimmed().toStdString());
        t->setYearsOfExperience(spExp->value());
        t->setRatePerHour(txtFee->text().trimmed().toStdString());
        t->setAvailability(txtSchedule->text().trimmed().toStdString());
        t->setTeachingAreas(FileManager::splitString(txtAreas->text().toStdString(), ','));
        t->setIsAvailable(cbAvail->currentIndex() == 0);

        FileManager::saveTutors(tutors);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin gia sư!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteTutor(Tutor* t) {
    if (!t) return;
    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
        QString::fromUtf8("Bạn có chắc chắn muốn xóa gia sư %1 (%2)?")
            .arg(QString::fromStdString(t->getFullName()), QString::fromStdString(t->getPersonID())),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (size_t i = 0; i < tutors.size(); ++i) {
            if (tutors[i] == t) {
                delete tutors[i];
                tutors.erase(i);
                break;
            }
        }
        FileManager::saveTutors(tutors);
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã xóa gia sư thành công!"));
    }
}

void MainWindow::onViewTutor(Tutor* t) {
    if (!t) return;
    QString info = QString::fromUtf8(
        "Mã gia sư: %1\n"
        "Họ và tên: %2\n"
        "Số điện thoại: %3\n"
        "Email: %4\n"
        "Địa chỉ: %5\n"
        "Môn giảng dạy: %6\n"
        "Trường / Bằng cấp: %7\n"
        "Số năm kinh nghiệm: %8 năm\n"
        "Mức phí yêu cầu: %9 đ/buổi\n"
        "Lịch rảnh: %10\n"
        "Khu vực nhận lớp: %11\n"
        "Trạng thái: %12"
    ).arg(
        QString::fromStdString(t->getPersonID()),
        QString::fromStdString(t->getFullName()),
        QString::fromStdString(t->getPhone()),
        QString::fromStdString(t->getEmail()),
        QString::fromStdString(t->getAddress()),
        joinStrings(t->getSubjects()),
        QString::fromStdString(t->getDegrees()),
        QString::number(t->getYearsOfExperience()),
        QString::fromStdString(t->getRatePerHour()),
        QString::fromStdString(t->getAvailability()),
        joinStrings(t->getTeachingAreas()),
        t->getIsAvailable() ? QString::fromUtf8("Sẵn sàng") : QString::fromUtf8("Đang bận")
    );

    QMessageBox::information(this, QString::fromUtf8("Thông tin chi tiết gia sư"), info);
}

// -------------------------------------------------------------
// CRUD DIALOGS: LỚP HỌC
// -------------------------------------------------------------
void MainWindow::onAddClass() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Tạo lớp học mới"));
    dialog.resize(480, 480);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
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
    auto* txtEnd = new QLineEdit(QDate::currentDate().addMonths(3).toString("dd/MM/yyyy"), &dialog);
    auto* cbStatus = new QComboBox(&dialog);
    cbStatus->addItems({QString::fromUtf8("Đang học"), QString::fromUtf8("Chờ xếp lịch"), QString::fromUtf8("Cần gia sư"), QString::fromUtf8("Hoàn thành")});
    auto* txtFee = new QLineEdit("200000", &dialog);

    form->addRow(label(QString::fromUtf8("Mã lớp:")), txtId);
    form->addRow(label(QString::fromUtf8("Học sinh *:")), cbStudent);
    form->addRow(label(QString::fromUtf8("Gia sư:")), cbTutor);
    form->addRow(label(QString::fromUtf8("Môn học *:")), txtSubject);
    form->addRow(label(QString::fromUtf8("Lịch học:")), txtSchedule);
    form->addRow(label(QString::fromUtf8("Ngày bắt đầu:")), txtStart);
    form->addRow(label(QString::fromUtf8("Ngày kết thúc:")), txtEnd);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);
    form->addRow(label(QString::fromUtf8("Học phí / buổi:")), txtFee);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu lớp học"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        if (cbStudent->count() == 0 || txtSubject->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Thông báo"), QString::fromUtf8("Vui lòng chọn học sinh và nhập môn học!"));
            return;
        }

        string sId = cbStudent->currentData().toString().toStdString();
        string tId = cbTutor->currentData().toString().toStdString();
        string today = QDate::currentDate().toString("dd/MM/yyyy").toStdString();

        auto* c = new TutoringClass(
            txtId->text().trimmed().toStdString(),
            sId,
            tId,
            txtSubject->text().trimmed().toStdString(),
            txtSchedule->text().trimmed().toStdString(),
            txtStart->text().trimmed().toStdString(),
            txtEnd->text().trimmed().toStdString(),
            cbStatus->currentText().toStdString(),
            txtFee->text().trimmed().toStdString(),
            today,
            today
        );

        classes.push_back(c);
        FileManager::saveClasses(classes);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã tạo lớp học thành công!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onEditClass(TutoringClass* c) {
    if (!c) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa lớp học: ") + QString::fromStdString(c->getClassID()));
    dialog.resize(480, 480);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
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
    auto* txtEnd = new QLineEdit(QString::fromStdString(c->getEndDate()), &dialog);
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
    form->addRow(label(QString::fromUtf8("Ngày kết thúc:")), txtEnd);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);
    form->addRow(label(QString::fromUtf8("Học phí / buổi:")), txtFee);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        c->setStudentID(cbStudent->currentData().toString().toStdString());
        c->setTutorID(cbTutor->currentData().toString().toStdString());
        c->setSubject(txtSubject->text().trimmed().toStdString());
        c->setSchedule(txtSchedule->text().trimmed().toStdString());
        c->setStartDate(txtStart->text().trimmed().toStdString());
        c->setEndDate(txtEnd->text().trimmed().toStdString());
        c->setStatus(cbStatus->currentText().toStdString());
        c->setRatePerHour(txtFee->text().trimmed().toStdString());
        c->setUpdatedAt(QDate::currentDate().toString("dd/MM/yyyy").toStdString());

        FileManager::saveClasses(classes);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin lớp học!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteClass(TutoringClass* c) {
    if (!c) return;
    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
        QString::fromUtf8("Bạn có chắc chắn muốn xóa lớp học %1?").arg(QString::fromStdString(c->getClassID())),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (size_t i = 0; i < classes.size(); ++i) {
            if (classes[i] == c) {
                delete classes[i];
                classes.erase(i);
                break;
            }
        }
        FileManager::saveClasses(classes);
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã xóa lớp học thành công!"));
    }
}

// -------------------------------------------------------------
// CRUD DIALOGS: HỢP ĐỒNG
// -------------------------------------------------------------
void MainWindow::onAddContract() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Tạo hợp đồng mới"));
    dialog.resize(480, 480);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("HD", contracts, [](Contract* x) { return x->getContractID(); });
    auto* txtId = new QLineEdit(QString::fromStdString(nextId), &dialog);

    auto* cbClass = new QComboBox(&dialog);
    for (size_t i = 0; i < classes.size(); ++i) {
        cbClass->addItem(QString("%1 - %2").arg(QString::fromStdString(classes[i]->getClassID()), QString::fromStdString(classes[i]->getSubject())), QString::fromStdString(classes[i]->getClassID()));
    }

    auto* cbStudent = new QComboBox(&dialog);
    for (size_t i = 0; i < students.size(); ++i) {
        cbStudent->addItem(QString("%1 - %2").arg(QString::fromStdString(students[i]->getPersonID()), QString::fromStdString(students[i]->getFullName())), QString::fromStdString(students[i]->getPersonID()));
    }

    auto* cbTutor = new QComboBox(&dialog);
    for (size_t i = 0; i < tutors.size(); ++i) {
        cbTutor->addItem(QString("%1 - %2").arg(QString::fromStdString(tutors[i]->getPersonID()), QString::fromStdString(tutors[i]->getFullName())), QString::fromStdString(tutors[i]->getPersonID()));
    }

    auto* txtStart = new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy"), &dialog);
    auto* txtEnd = new QLineEdit(QDate::currentDate().addMonths(3).toString("dd/MM/yyyy"), &dialog);
    auto* txtFee = new QLineEdit("2400000", &dialog);
    auto* cbStatus = new QComboBox(&dialog);
    cbStatus->addItems({QString::fromUtf8("Đang hiệu lực"), QString::fromUtf8("Chờ xử lý"), QString::fromUtf8("Hoàn thành")});

    form->addRow(label(QString::fromUtf8("Mã hợp đồng:")), txtId);
    form->addRow(label(QString::fromUtf8("Lớp học:")), cbClass);
    form->addRow(label(QString::fromUtf8("Học sinh:")), cbStudent);
    form->addRow(label(QString::fromUtf8("Gia sư:")), cbTutor);
    form->addRow(label(QString::fromUtf8("Ngày bắt đầu:")), txtStart);
    form->addRow(label(QString::fromUtf8("Ngày kết thúc:")), txtEnd);
    form->addRow(label(QString::fromUtf8("Tổng chi phí:")), txtFee);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu hợp đồng"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        string clId = cbClass->currentData().toString().toStdString();
        string sId = cbStudent->currentData().toString().toStdString();
        string tId = cbTutor->currentData().toString().toStdString();
        string today = QDate::currentDate().toString("dd/MM/yyyy").toStdString();

        auto* c = new Contract(
            txtId->text().trimmed().toStdString(),
            clId,
            sId,
            tId,
            today,
            txtStart->text().trimmed().toStdString(),
            txtEnd->text().trimmed().toStdString(),
            txtFee->text().trimmed().toStdString(),
            cbStatus->currentText().toStdString()
        );

        contracts.push_back(c);
        FileManager::saveContracts(contracts);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã tạo hợp đồng thành công!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onEditContract(Contract* c) {
    if (!c) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa hợp đồng: ") + QString::fromStdString(c->getContractID()));
    dialog.resize(480, 480);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(c->getContractID()), &dialog);
    txtId->setEnabled(false);

    auto* txtClassId = new QLineEdit(QString::fromStdString(c->getClassID()), &dialog);
    auto* txtStart = new QLineEdit(QString::fromStdString(c->getStartDate()), &dialog);
    auto* txtEnd = new QLineEdit(QString::fromStdString(c->getEndDate()), &dialog);
    auto* txtFee = new QLineEdit(QString::fromStdString(c->getTotalFee()), &dialog);
    auto* cbStatus = new QComboBox(&dialog);
    cbStatus->addItems({QString::fromUtf8("Đang hiệu lực"), QString::fromUtf8("Chờ xử lý"), QString::fromUtf8("Hoàn thành")});
    cbStatus->setCurrentText(QString::fromStdString(c->getStatus()));

    form->addRow(label(QString::fromUtf8("Mã hợp đồng:")), txtId);
    form->addRow(label(QString::fromUtf8("Mã lớp:")), txtClassId);
    form->addRow(label(QString::fromUtf8("Ngày bắt đầu:")), txtStart);
    form->addRow(label(QString::fromUtf8("Ngày kết thúc:")), txtEnd);
    form->addRow(label(QString::fromUtf8("Tổng chi phí:")), txtFee);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        c->setClassID(txtClassId->text().trimmed().toStdString());
        c->setStartDate(txtStart->text().trimmed().toStdString());
        c->setEndDate(txtEnd->text().trimmed().toStdString());
        c->setTotalFee(txtFee->text().trimmed().toStdString());
        c->setStatus(cbStatus->currentText().toStdString());

        FileManager::saveContracts(contracts);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin hợp đồng!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteContract(Contract* c) {
    if (!c) return;
    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
        QString::fromUtf8("Bạn có chắc chắn muốn xóa hợp đồng %1?").arg(QString::fromStdString(c->getContractID())),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (size_t i = 0; i < contracts.size(); ++i) {
            if (contracts[i] == c) {
                delete contracts[i];
                contracts.erase(i);
                break;
            }
        }
        FileManager::saveContracts(contracts);
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã xóa hợp đồng thành công!"));
    }
}

// -------------------------------------------------------------
// CRUD DIALOGS: ĐĂNG KÝ MÔN HỌC
// -------------------------------------------------------------
void MainWindow::onAddRegistration() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Đăng ký môn học mới"));
    dialog.resize(460, 400);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    string nextId = IdGenerator::nextFor("DK", registrations, [](Registration* x) { return x->getRegistrationID(); });
    auto* txtRegID = new QLineEdit(QString::fromStdString(nextId), &dialog);

    auto* cbStudent = new QComboBox(&dialog);
    for (size_t i = 0; i < students.size(); ++i) {
        cbStudent->addItem(QString("%1 - %2").arg(QString::fromStdString(students[i]->getPersonID()), QString::fromStdString(students[i]->getFullName())), QString::fromStdString(students[i]->getPersonID()));
    }

    auto* txtSubjects = new QLineEdit(&dialog);
    txtSubjects->setPlaceholderText(QString::fromUtf8("Nhập các môn cách nhau bởi dấu phẩy, vd: Toán, Lý, Hóa"));
    auto* txtDate = new QLineEdit(QDate::currentDate().toString("dd/MM/yyyy"), &dialog);
    auto* txtNotes = new QLineEdit(&dialog);

    form->addRow(label(QString::fromUtf8("Mã phiếu:")), txtRegID);
    form->addRow(label(QString::fromUtf8("Sinh viên *:")), cbStudent);
    form->addRow(label(QString::fromUtf8("Các môn đăng ký *:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Ngày đăng ký:")), txtDate);
    form->addRow(label(QString::fromUtf8("Ghi chú:")), txtNotes);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Lưu đăng ký"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        if (cbStudent->count() == 0 || txtSubjects->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, QString::fromUtf8("Thông báo"), QString::fromUtf8("Vui lòng chọn sinh viên và nhập ít nhất một môn học!"));
            return;
        }

        string sId = cbStudent->currentData().toString().toStdString();
        Student* s = findStudent(sId);
        string sName = s ? s->getFullName() : sId;

        Vector<string> subList = FileManager::splitString(txtSubjects->text().toStdString(), ',');

        auto* newReg = new Registration(
            txtRegID->text().trimmed().toStdString(),
            sId,
            sName,
            subList,
            txtDate->text().trimmed().toStdString(),
            "Cho ghep gia su",
            txtNotes->text().trimmed().toStdString()
        );

        registrations.push_back(newReg);
        FileManager::saveRegistrations(registrations);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã đăng ký thành công các môn học cho sinh viên!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onEditRegistration(Registration* r) {
    if (!r) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Chỉnh sửa phiếu đăng ký: ") + QString::fromStdString(r->getRegistrationID()));
    dialog.resize(460, 400);
    dialog.setStyleSheet("QDialog { background:#FFFFFF; } QLineEdit, QComboBox { background:#F8FAFC; border:1px solid #CBD5E1; border-radius:6px; padding:6px 10px; color:#0F172A; }");

    auto* form = new QFormLayout(&dialog);
    form->setContentsMargins(24, 20, 24, 20);
    form->setSpacing(12);

    auto* txtId = new QLineEdit(QString::fromStdString(r->getRegistrationID()), &dialog);
    txtId->setEnabled(false);
    auto* txtSId = new QLineEdit(QString::fromStdString(r->getStudentID()), &dialog);
    txtSId->setEnabled(false);
    auto* txtName = new QLineEdit(QString::fromStdString(r->getStudentName()), &dialog);
    auto* txtSubjects = new QLineEdit(QString::fromStdString(r->getSubjectsSummary()), &dialog);
    auto* txtDate = new QLineEdit(QString::fromStdString(r->getRegistrationDate()), &dialog);
    auto* cbStatus = new QComboBox(&dialog);
    cbStatus->addItems({"Cho ghep gia su", "Da duyet", "Huy"});
    cbStatus->setCurrentText(QString::fromStdString(r->getStatus()));
    auto* txtNotes = new QLineEdit(QString::fromStdString(r->getNotes()), &dialog);

    form->addRow(label(QString::fromUtf8("Mã phiếu:")), txtId);
    form->addRow(label(QString::fromUtf8("Mã sinh viên:")), txtSId);
    form->addRow(label(QString::fromUtf8("Tên sinh viên:")), txtName);
    form->addRow(label(QString::fromUtf8("Các môn đăng ký:")), txtSubjects);
    form->addRow(label(QString::fromUtf8("Ngày đăng ký:")), txtDate);
    form->addRow(label(QString::fromUtf8("Trạng thái:")), cbStatus);
    form->addRow(label(QString::fromUtf8("Ghi chú:")), txtNotes);

    auto* btnBox = new QHBoxLayout;
    auto* btnCancel = new QPushButton(QString::fromUtf8("Hủy"), &dialog);
    btnCancel->setStyleSheet("background:#E2E8F0; color:#334155; padding:8px 16px; border-radius:6px; font-weight:600;");
    auto* btnSave = new QPushButton(QString::fromUtf8("Cập nhật"), &dialog);
    btnSave->setStyleSheet("background:#0284C7; color:white; padding:8px 16px; border-radius:6px; font-weight:600;");
    btnBox->addStretch();
    btnBox->addWidget(btnCancel);
    btnBox->addWidget(btnSave);
    form->addRow(btnBox);

    connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, [&]() {
        r->setStudentName(txtName->text().trimmed().toStdString());
        r->setSubjects(FileManager::splitString(txtSubjects->text().toStdString(), ','));
        r->setRegistrationDate(txtDate->text().trimmed().toStdString());
        r->setStatus(cbStatus->currentText().toStdString());
        r->setNotes(txtNotes->text().trimmed().toStdString());

        FileManager::saveRegistrations(registrations);
        refreshAll();
        QMessageBox::information(&dialog, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã cập nhật thông tin phiếu đăng ký!"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::onDeleteRegistration(Registration* r) {
    if (!r) return;
    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận xóa"),
        QString::fromUtf8("Bạn có chắc chắn muốn xóa phiếu đăng ký %1?").arg(QString::fromStdString(r->getRegistrationID())),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (size_t i = 0; i < registrations.size(); ++i) {
            if (registrations[i] == r) {
                delete registrations[i];
                registrations.erase(i);
                break;
            }
        }
        FileManager::saveRegistrations(registrations);
        refreshAll();
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã xóa phiếu đăng ký thành công!"));
    }
}
