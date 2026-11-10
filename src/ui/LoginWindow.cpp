#include "LoginWindow.h"
#include "MainWindow.h"
#include "FileManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFontMetricsF>
#include <QLinearGradient>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QSettings>

namespace {

// Kích thước ảnh thiết kế gốc: mọi toạ độ bên dưới tính theo hệ này.
constexpr qreal kDesignW = 1578.0;
constexpr qreal kDesignH = 996.0;
const QRectF kCardRect(841, 88, 666, 650);

// Màu sắc lấy từ bản thiết kế
const char* kNavy      = "#141a4a";   // tiêu đề, nhãn
const char* kMuted     = "#55648a";   // chữ phụ
const char* kLink      = "#4b3fe8";   // liên kết
const char* kError     = "#e0334f";
const QColor kBorder("#dde1ee");

// Tìm thư mục assets giống cách main.cpp tìm app.qss (chạy từ build/, từ thư mục dự án hoặc cạnh file exe).
QString assetPath(const QString& relative) {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList bases = {
        appDir + "/../assets", appDir + "/assets", appDir + "/../../assets",
        QDir::currentPath() + "/assets", QDir::currentPath() + "/../assets"
    };
    for (const QString& base : bases) {
        const QString path = base + "/" + relative;
        if (QFileInfo::exists(path)) return QDir::cleanPath(path);
    }
    return QString();
}

QPixmap loadAsset(const QString& relative) {
    const QString path = assetPath(relative);
    return path.isEmpty() ? QPixmap() : QPixmap(path);
}

// Chữ "TutorFlow": "Tutor" màu navy đậm, "Flow" tô gradient xanh -> tím. Cỡ chữ tự co để vừa bề ngang ô.
class WordMark : public QWidget {
public:
    using QWidget::QWidget;
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        QFont f = font();
        f.setWeight(QFont::ExtraBold);
        f.setPixelSize(100);
        const QFontMetricsF probe(f);
        const qreal total = probe.horizontalAdvance("TutorFlow");
        f.setPixelSize(qMax(8, qRound(100.0 * width() / total)));
        p.setFont(f);
        const QFontMetricsF fm(f);
        const qreal tutorW = fm.horizontalAdvance("Tutor");
        const qreal flowW = fm.horizontalAdvance("Flow");
        const qreal base = (height() + fm.ascent() - fm.descent()) / 2.0;
        p.setPen(QColor("#10174a"));
        p.drawText(QPointF(0, base), "Tutor");
        QLinearGradient g(tutorW, 0, tutorW + flowW, 0);
        g.setColorAt(0, QColor("#3446ea"));
        g.setColorAt(1, QColor("#7a4cf0"));
        p.setPen(QPen(QBrush(g), 1));
        p.drawText(QPointF(tutorW, base), "Flow");
    }
};

// Nút "Đăng nhập" gradient xanh -> tím kèm mũi tên.
class GradientButton : public QPushButton {
public:
    explicit GradientButton(const QString& text, QWidget* parent) : QPushButton(text, parent) {
        setAttribute(Qt::WA_Hover);
        setCursor(Qt::PointingHandCursor);
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r(rect());
        const qreal radius = r.height() * 0.19;
        QLinearGradient g(r.left(), 0, r.right(), 0);
        g.setColorAt(0.0, QColor("#3a6cfb"));
        g.setColorAt(0.5, QColor("#6165f4"));
        g.setColorAt(1.0, QColor("#c569ee"));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawRoundedRect(r, radius, radius);
        if (isDown()) { p.setBrush(QColor(0, 0, 40, 38)); p.drawRoundedRect(r, radius, radius); }
        else if (underMouse()) { p.setBrush(QColor(255, 255, 255, 30)); p.drawRoundedRect(r, radius, radius); }

        p.setFont(font());
        const QFontMetricsF fm(font());
        const qreal textW = fm.horizontalAdvance(text());
        const qreal arrowW = r.height() * 0.26, gap = r.height() * 0.21;
        const qreal x0 = (r.width() - (textW + gap + arrowW)) / 2.0;
        p.setPen(Qt::white);
        p.drawText(QRectF(x0, 0, textW + 2, r.height()), Qt::AlignVCenter | Qt::AlignLeft, text());
        const qreal ax = x0 + textW + gap, cy = r.height() / 2.0, hh = arrowW * 0.36;
        QPen pen(Qt::white, r.height() * 0.032, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.drawLine(QPointF(ax, cy), QPointF(ax + arrowW, cy));
        p.drawLine(QPointF(ax + arrowW - hh, cy - hh), QPointF(ax + arrowW, cy));
        p.drawLine(QPointF(ax + arrowW - hh, cy + hh), QPointF(ax + arrowW, cy));
    }
};

} // namespace

LoginWindow::LoginWindow(QWidget* parent) : QWidget(parent),
    usernameEdit(new QLineEdit(this)), passwordEdit(new QLineEdit(this)), rememberCheck(new QCheckBox(this)),
    usernameError(nullptr), passwordError(nullptr), generalError(nullptr),
    eyeButton(new QToolButton(this)) {
    setWindowTitle(QString::fromUtf8("TutorFlow | Đăng nhập"));
    resize(1100, 694);
    setMinimumSize(800, 505);

    background = loadAsset("login/login_bg.png");
    eyeOn = loadAsset("login/eye_on.png");
    eyeOff = loadAsset("login/eye_off.png");
    checkIconPath = assetPath("login/check.png").replace('\\', '/');

    // ---------- Cột trái ----------
    addImage("login/logo_cap.png", 138, 82, 152, 120);
    auto* mark = new WordMark(this);
    addSlot(mark, 302, 84, 358, 92);
    addText(QString::fromUtf8("Học dễ hơn, tiến bộ nhanh hơn"), "#3d45e6", 307, 176, 420, 38, 26, QFont::Medium);
    addText(QString::fromUtf8("Nền tảng hành trình cá nhân hoá"), kNavy, 131, 243, 560, 40, 30, QFont::DemiBold);
    addText(QString::fromUtf8("học tập và phát triển của bạn"), kNavy, 131, 278, 560, 40, 30, QFont::DemiBold);

    struct Feature { const char* tile; QString title, desc; qreal y; };   // y = mép trên của ô icon
    const Feature features[] = {
        {"login/tile_learn.png", QString::fromUtf8("Học cá nhân hóa"),
         QString::fromUtf8("Lộ trình học tập phù hợp với năng lực và mục tiêu"), 345},
        {"login/tile_progress.png", QString::fromUtf8("Theo dõi tiến trình"),
         QString::fromUtf8("Cải thiện liên tục qua dữ liệu thực tế"), 448},
        {"login/tile_connect.png", QString::fromUtf8("Kết nối dễ dàng"),
         QString::fromUtf8("Giảng viên, mentor, bạn học chỉ trong một nền tảng"), 552},
    };
    for (const Feature& f : features) {
        addImage(f.tile, 131, f.y, 85, 85);
        addText(f.title, kNavy, 245, f.y + 10, 460, 34, 25, QFont::DemiBold);
        addText(f.desc, kMuted, 245, f.y + 49, 480, 30, 18, QFont::Normal);
    }

    // ---------- Thẻ đăng nhập ----------
    addText(QString::fromUtf8("Đăng nhập"), kNavy, 895, 118, 520, 60, 43, QFont::ExtraBold);
    addText(QString::fromUtf8("Chào mừng bạn quay trở lại! 👋"), kMuted, 895, 178, 560, 38, 25, QFont::Normal);

    addImage("login/icon_user.png", 892, 246, 46, 50);
    addText(QString::fromUtf8("Tài khoản"), kNavy, 956, 250, 300, 34, 22, QFont::DemiBold);
    usernameEdit->setPlaceholderText(QString::fromUtf8("Nhập tài khoản"));
    addSlot(usernameEdit, 956, 289, 504, 58, 22);
    usernameError = addText("", kError, 956, 349, 504, 30, 18, QFont::Normal);
    usernameError->hide();

    addImage("login/icon_lock.png", 892, 378, 46, 46);
    addText(QString::fromUtf8("Mật khẩu"), kNavy, 956, 380, 300, 34, 22, QFont::DemiBold);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Nhập mật khẩu"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    addSlot(passwordEdit, 956, 420, 504, 58, 22);
    passwordError = addText("", kError, 956, 480, 504, 28, 18, QFont::Normal);
    passwordError->hide();

    eyeButton->setCursor(Qt::PointingHandCursor);
    eyeButton->setFocusPolicy(Qt::NoFocus);
    eyeButton->setAutoRaise(true);
    eyeButton->setStyleSheet("QToolButton{border:0;background:transparent;}");
    eyeButton->setToolTip(QString::fromUtf8("Hiện / ẩn mật khẩu"));
    addSlot(eyeButton, 1408, 430, 44, 38);
    connect(eyeButton, &QToolButton::clicked, this, &LoginWindow::togglePasswordVisibility);

    rememberCheck->setText(QString::fromUtf8("Ghi nhớ tài khoản"));
    QSettings settings("TutorFlow", "TutorFlowPBL2");
    const QString rememberedUser = settings.value("login/username").toString();
    rememberCheck->setChecked(!rememberedUser.isEmpty());
    usernameEdit->setText(rememberedUser);
    rememberCheck->setCursor(Qt::PointingHandCursor);
    addSlot(rememberCheck, 895, 509, 300, 38, 22, QFont::Medium);

    // Password reset cannot be verified securely offline. Do not show an
    // action that only displays a placeholder "coming soon" popup.

    auto* login = new GradientButton(QString::fromUtf8("Đăng nhập"), this);
    addSlot(login, 895, 574, 565, 76, 26, QFont::DemiBold);
    generalError = addText("", kError, 895, 654, 565, 30, 19, QFont::Normal, Qt::AlignCenter);
    generalError->hide();

    // Thứ tự Tab: tài khoản -> mật khẩu -> nhớ đăng nhập -> nút đăng nhập
    setTabOrder(usernameEdit, passwordEdit);
    setTabOrder(passwordEdit, rememberCheck);
    setTabOrder(rememberCheck, login);


    connect(login, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(usernameEdit, &QLineEdit::returnPressed, this, [this]() { passwordEdit->setFocus(); });
    connect(usernameEdit, &QLineEdit::textChanged, this, [this] { setError(usernameEdit, usernameError, ""); generalError->hide(); });
    connect(passwordEdit, &QLineEdit::textChanged, this, [this] { setError(passwordEdit, passwordError, ""); generalError->hide(); });

    eyeButton->raise();
    applyLayout();
    // Không tự focus ô nhập khi mở để màn hình hiển thị đúng như bản thiết kế
    setFocusPolicy(Qt::ClickFocus);
    setFocus();
}

LoginWindow::Slot& LoginWindow::addSlot(QWidget* w, qreal x, qreal y, qreal width, qreal height,
                                        qreal fontPx, QFont::Weight weight) {
    Slot s; s.widget = w; s.rect = QRectF(x, y, width, height); s.fontPx = fontPx; s.weight = weight;
    layoutItems.push_back(s);
    return layoutItems.last();
}

QLabel* LoginWindow::addText(const QString& text, const QString& color, qreal x, qreal y, qreal w, qreal h,
                             qreal fontPx, QFont::Weight weight, Qt::Alignment align) {
    auto* label = new QLabel(text, this);
    label->setAlignment(align);
    label->setStyleSheet(QString("color:%1;background:transparent;").arg(color));
    label->setAttribute(Qt::WA_TransparentForMouseEvents, !text.contains("<a "));
    addSlot(label, x, y, w, h, fontPx, weight);
    return label;
}

QLabel* LoginWindow::addImage(const QString& file, qreal x, qreal y, qreal w, qreal h) {
    auto* label = new QLabel(this);
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    label->setStyleSheet("background:transparent;");
    addSlot(label, x, y, w, h).pixmap = loadAsset(file);
    return label;
}

// Đặt lại vị trí, cỡ chữ và ảnh theo kích thước cửa sổ hiện tại.
void LoginWindow::applyLayout() {
    scale = qMin(width() / kDesignW, height() / kDesignH);
    origin = QPointF((width() - kDesignW * scale) / 2.0, (height() - kDesignH * scale) / 2.0);
    const qreal dpr = devicePixelRatioF();

    for (Slot& s : layoutItems) {
        const QRect r(qRound(origin.x() + s.rect.x() * scale), qRound(origin.y() + s.rect.y() * scale),
                      qRound(s.rect.width() * scale), qRound(s.rect.height() * scale));
        s.widget->setGeometry(r);
        if (s.fontPx > 0) {
            QFont f = s.widget->font();
            f.setPixelSize(qMax(8, qRound(s.fontPx * scale)));
            f.setWeight(s.weight);
            s.widget->setFont(f);
        }
        if (!s.pixmap.isNull()) {
            QPixmap scaled = s.pixmap.scaled(r.size() * dpr, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            scaled.setDevicePixelRatio(dpr);
            static_cast<QLabel*>(s.widget)->setPixmap(scaled);
        }
    }

    // Ô nhập: bo góc, viền mảnh, viền nhấn khi focus (kích thước theo hệ số co giãn)
    const int radius = qRound(13 * scale), padL = qRound(19 * scale), border = qMax(1, qRound(1.6 * scale));
    const QString editStyle = QString(
        "QLineEdit{background:#ffffff;border:%1px solid #dde1ee;border-radius:%2px;padding:0 %3px 0 %4px;"
        "color:%5;selection-background-color:#cfd3fb;selection-color:%5;}"
        "QLineEdit:focus{border:%6px solid #6a63f0;}"
        "QLineEdit[invalid=\"true\"]{border:%6px solid #e34962;background:#fff8f9;}")
        .arg(border).arg(radius).arg(padL).arg(padL).arg(kNavy).arg(qMax(border, qRound(2.0 * scale)));
    usernameEdit->setStyleSheet(editStyle);
    passwordEdit->setStyleSheet(editStyle);
    // Chừa chỗ cho nút con mắt bên phải ô mật khẩu
    passwordEdit->setTextMargins(0, 0, qRound(58 * scale), 0);
    QPalette pal = usernameEdit->palette();
    pal.setColor(QPalette::PlaceholderText, QColor("#8a93ad"));
    usernameEdit->setPalette(pal);
    passwordEdit->setPalette(pal);

    // Nút hiện/ẩn mật khẩu
    const bool hidden = passwordEdit->echoMode() == QLineEdit::Password;
    const QPixmap& icon = hidden ? eyeOff : eyeOn;
    if (!icon.isNull()) {
        const int ih = qMax(8, qRound(37 * scale));
        eyeButton->setIcon(QIcon(icon));
        eyeButton->setIconSize(QSize(ih, ih));
    }

    // Ô tick chỉ ghi nhớ tên tài khoản
    const int box = qRound(38 * scale);
    QString checked = checkIconPath.isEmpty() ? QString() : QString("image:url(%1);").arg(checkIconPath);
    rememberCheck->setStyleSheet(QString(
        "QCheckBox{color:%1;spacing:%2px;background:transparent;}"
        "QCheckBox::indicator{width:%3px;height:%3px;border-radius:%4px;border:%5px solid #c9cfe6;background:#ffffff;}"
        "QCheckBox::indicator:hover{border-color:#8a85f2;}"
        "QCheckBox::indicator:checked{background:#5b42f0;border:%5px solid #5b42f0;%6}")
        .arg(kNavy).arg(qRound(22 * scale)).arg(box).arg(qRound(9 * scale)).arg(qMax(1, qRound(2 * scale))).arg(checked));

    // Ảnh nền co theo cửa sổ (giữ tỉ lệ thiết kế)
    if (!background.isNull()) {
        const QSize target(qRound(kDesignW * scale * dpr), qRound(kDesignH * scale * dpr));
        backgroundScaled = background.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        backgroundScaled.setDevicePixelRatio(dpr);
    }
    update();
}

void LoginWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    applyLayout();
}

void LoginWindow::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // Nền: phủ màu lavender cho phần thừa khi tỉ lệ cửa sổ khác bản thiết kế, rồi vẽ ảnh nền ở giữa.
    QLinearGradient fill(rect().topLeft(), rect().bottomRight());
    fill.setColorAt(0, QColor("#eef1ff")); fill.setColorAt(1, QColor("#e9e2fb"));
    p.fillRect(rect(), fill);
    if (!backgroundScaled.isNull()) {
        p.drawPixmap(origin, backgroundScaled);
    } else {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(162, 142, 250, 45)); p.drawEllipse(QPointF(60, 70), 210, 210);
        p.setBrush(QColor(116, 188, 255, 45)); p.drawEllipse(QPointF(width() - 65, height() - 35), 255, 230);
    }

    // Thẻ đăng nhập: bóng mềm + nền trắng bo góc
    const QRectF card(origin.x() + kCardRect.x() * scale, origin.y() + kCardRect.y() * scale,
                      kCardRect.width() * scale, kCardRect.height() * scale);
    const qreal radius = 30 * scale;
    p.setPen(Qt::NoPen);
    for (int i = 18; i >= 1; --i) {
        const qreal grow = i * 2.4 * scale;
        p.setBrush(QColor(86, 74, 190, 3));
        p.drawRoundedRect(card.adjusted(-grow, -grow + 16 * scale, grow, grow + 16 * scale), radius + grow, radius + grow);
    }
    p.setPen(QPen(QColor(238, 236, 252), 1));
    p.setBrush(Qt::white);
    p.drawRoundedRect(card, radius, radius);

}

void LoginWindow::togglePasswordVisibility() {
    passwordEdit->setEchoMode(passwordEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
    const bool hidden = passwordEdit->echoMode() == QLineEdit::Password;
    const QPixmap& icon = hidden ? eyeOff : eyeOn;
    if (!icon.isNull()) eyeButton->setIcon(QIcon(icon));
    passwordEdit->setFocus();
}

void LoginWindow::setError(QLineEdit* field, QLabel* label, const QString& message) {
    label->setText(message); label->setVisible(!message.isEmpty()); field->setProperty("invalid", !message.isEmpty());
    field->style()->unpolish(field); field->style()->polish(field);
}

// Logic đăng nhập giữ nguyên như bản cũ.
void LoginWindow::onLoginClicked() {
    const QString username = usernameEdit->text().trimmed(); const QString password = passwordEdit->text();
    setError(usernameEdit, usernameError, username.isEmpty() ? QString::fromUtf8("Vui lòng nhập tài khoản.") : QString());
    setError(passwordEdit, passwordError, password.isEmpty() ? QString::fromUtf8("Vui lòng nhập mật khẩu.") : QString());
    if (username.isEmpty()) { usernameEdit->setFocus(); return; }
    if (password.isEmpty()) { passwordEdit->setFocus(); return; }
    if (FileManager::validateAdminLogin(username.toStdString(), password.toStdString())) {
        // Store username only; NEVER persist a password or auto-authenticate.
        QSettings settings("TutorFlow", "TutorFlowPBL2");
        if (rememberCheck->isChecked()) settings.setValue("login/username", username);
        else settings.remove("login/username");
        auto* mainWindow = new MainWindow(); mainWindow->setAttribute(Qt::WA_DeleteOnClose); mainWindow->show(); close(); return;
    }
    generalError->setText(QString::fromUtf8("Tên đăng nhập hoặc mật khẩu không chính xác.")); generalError->show();
    passwordEdit->setFocus(); passwordEdit->selectAll();
}
