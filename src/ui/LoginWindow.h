#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QRectF>
#include <QToolButton>
#include <QVector>
#include <QFont>

// Màn hình đăng nhập TutorFlow.
// Giao diện được dựng theo bản thiết kế 1578 x 996: mọi thành phần đặt theo toạ độ thiết kế
// rồi nhân với hệ số co giãn, nên khi đổi kích thước cửa sổ bố cục vẫn giữ đúng tỉ lệ như ảnh mẫu.
class LoginWindow : public QWidget {
    Q_OBJECT
public:
    explicit LoginWindow(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onLoginClicked();
    void togglePasswordVisibility();

private:
    // Một thành phần giao diện cùng vị trí / cỡ chữ theo toạ độ thiết kế.
    struct Slot {
        QWidget* widget = nullptr;
        QRectF   rect;                       // toạ độ trong ảnh thiết kế
        qreal    fontPx = 0;                 // cỡ chữ (px thiết kế), 0 = không đặt
        QFont::Weight weight = QFont::Normal;
        QPixmap  pixmap;                     // nếu có: ảnh gốc của QLabel
    };

    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QCheckBox* rememberCheck;
    QLabel* usernameError;
    QLabel* passwordError;
    QLabel* generalError;
    QToolButton* eyeButton;

    QVector<Slot> layoutItems;
    QPixmap background;        // ảnh nền gốc
    QPixmap backgroundScaled;  // ảnh nền đã co giãn theo cửa sổ
    QPixmap eyeOn, eyeOff;
    QString checkIconPath;
    qreal scale = 1.0;
    QPointF origin;

    Slot& addSlot(QWidget* w, qreal x, qreal y, qreal width, qreal height,
                  qreal fontPx = 0, QFont::Weight weight = QFont::Normal);
    QLabel* addText(const QString& text, const QString& color, qreal x, qreal y, qreal w, qreal h,
                    qreal fontPx, QFont::Weight weight, Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter);
    QLabel* addImage(const QString& file, qreal x, qreal y, qreal w, qreal h);
    void applyLayout();
    void setError(QLineEdit* field, QLabel* label, const QString& message);
};
#endif
