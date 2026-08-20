#include "LoginWindow.h"
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QFrame>
#include <QPixmap>
#include <QCoreApplication>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent),
      usernameEdit(new QLineEdit(this)),
      passwordEdit(new QLineEdit(this)),
      rememberCheck(new QCheckBox(this))
{
    setWindowTitle(QString::fromUtf8("Đăng Nhập - Trung tâm gia sư"));
    resize(700, 650);
    setMinimumSize(600, 600);

    setStyleSheet(R"(
        LoginWindow {
            background-color: #F4F5F7;
            font-family: "Segoe UI", Arial, sans-serif;
        }
    )");

    auto* card = new QFrame(this);
    card->setObjectName("loginCard");
    card->setFixedWidth(400);

    card->setStyleSheet(R"(
        QFrame#loginCard {
            background-color: #FCFCFD;
            border: 1px solid #E3E6EA;
            border-radius: 12px;
        }
    )");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(38, 30, 38, 30);
    layout->setSpacing(0);

    auto* logo = new QLabel(card);
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(300, 90);

    const QStringList logoCandidates = {
        QCoreApplication::applicationDirPath() + "/../assets/logo.png",
        QCoreApplication::applicationDirPath() + "/assets/logo.png",
        QStringLiteral("assets/logo.png"),
        QCoreApplication::applicationDirPath() + "/../assets/logo.svg",
        QCoreApplication::applicationDirPath() + "/assets/logo.svg",
        QStringLiteral("assets/logo.svg")
    };

    QPixmap logoPixmap;
    for (const QString& logoPath : logoCandidates) {
        if (QFileInfo::exists(logoPath) && logoPixmap.load(logoPath)) {
            break;
        }
    }

    if (logoPixmap.isNull())
    {
        logoPixmap = QPixmap(900, 320);
        logoPixmap.fill(Qt::transparent);

        QPainter painter(&logoPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#0b4ca8"), 18, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        QPainterPath cap;
        cap.moveTo(32, 92);
        cap.lineTo(256, 12);
        cap.lineTo(512, 92);
        cap.lineTo(256, 174);
        cap.closeSubpath();
        painter.drawPath(cap);

        QPainterPath book;
        book.moveTo(58, 112);
        book.lineTo(58, 246);
        book.quadTo(58, 278, 90, 278);
        book.lineTo(422, 278);
        book.quadTo(454, 278, 454, 246);
        book.lineTo(454, 112);
        book.moveTo(256, 174);
        book.lineTo(256, 292);
        book.moveTo(256, 292);
        book.quadTo(190, 234, 82, 234);
        book.moveTo(256, 292);
        book.quadTo(322, 234, 430, 234);
        painter.drawPath(book);

        QPainterPath tassel;
        tassel.moveTo(332, 82);
        tassel.lineTo(332, 190);
        tassel.lineTo(316, 246);
        painter.drawPath(tassel);

        painter.setBrush(QColor("#0b4ca8"));
        painter.drawEllipse(QPointF(256, 54), 18, 18);
        painter.drawEllipse(QPointF(332, 190), 15, 15);

        painter.setPen(QColor("#0b4ca8"));
        painter.setFont(QFont("Segoe UI", 74, QFont::Bold));
        painter.drawText(QPointF(560, 184), "TUTORFLOW");
        painter.end();
    }

    logo->setPixmap(
        logoPixmap.scaled(
            300,
            90,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );

    auto* logoRow = new QHBoxLayout();
    logoRow->setContentsMargins(0, 0, 0, 0);

    logoRow->addStretch();
    logoRow->addWidget(logo);
    logoRow->addStretch();

    layout->addLayout(logoRow);
    layout->addSpacing(15);

    auto* title = new QLabel(
        QString::fromUtf8("TRUNG TÂM GIA SƯ"),
        card
    );

    title->setAlignment(Qt::AlignCenter);

    title->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #2B313B;
            font-size: 22px;
            font-weight: 700;
        }
    )");

    layout->addWidget(title);
    layout->addSpacing(7);

    auto* subtitle = new QLabel(
        QString::fromUtf8(
            "Hệ Thống Quản Lý và Tìm Kiếm Gia Sư"
        ),
        card
    );

    subtitle->setAlignment(Qt::AlignCenter);

    subtitle->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #667085;
            font-size: 13px;
        }
    )");

    layout->addWidget(subtitle);
    layout->addSpacing(25);

    auto* usernameLabel = new QLabel(
        QString::fromUtf8("Tên đăng nhập"),
        card
    );

    usernameLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #343A45;
            font-size: 12px;
            font-weight: 600;
        }
    )");

    layout->addWidget(usernameLabel);
    layout->addSpacing(7);

    usernameEdit->setPlaceholderText(
        QString::fromUtf8("Nhập tên đăng nhập")
    );

    usernameEdit->setFixedHeight(40);

    usernameEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #FFFFFF;
            color: #344054;
            border: 1px solid #CDD2D8;
            border-radius: 3px;
            padding: 0px 12px;
            font-size: 13px;
        }

        QLineEdit::placeholder {
            color: #98A2B3;
        }

        QLineEdit:focus {
            border: 1px solid #2C638F;
        }
    )");

    layout->addWidget(usernameEdit);
    layout->addSpacing(17);

    auto* passwordLabel = new QLabel(
        QString::fromUtf8("Mật khẩu"),
        card
    );

    passwordLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #343A45;
            font-size: 12px;
            font-weight: 600;
        }
    )");

    layout->addWidget(passwordLabel);
    layout->addSpacing(7);

    passwordEdit->setPlaceholderText(
        QString::fromUtf8("Nhập mật khẩu")
    );

    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(40);

    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #FFFFFF;
            color: #344054;
            border: 1px solid #CDD2D8;
            border-radius: 3px;
            padding: 0px 12px;
            font-size: 13px;
        }

        QLineEdit::placeholder {
            color: #98A2B3;
        }

        QLineEdit:focus {
            border: 1px solid #2C638F;
        }
    )");

    layout->addWidget(passwordEdit);
    layout->addSpacing(10);

    rememberCheck->setText(
        QString::fromUtf8("Ghi nhớ đăng nhập")
    );

    rememberCheck->setStyleSheet(R"(
        QCheckBox {
            background: transparent;
            color: #667085;
            font-size: 12px;
            spacing: 7px;
        }

        QCheckBox::indicator {
            width: 13px;
            height: 13px;
            border: 1px solid #D0D5DD;
            border-radius: 2px;
            background-color: #FFFFFF;
        }

        QCheckBox::indicator:checked {
            background-color: #285E8E;
            border: 1px solid #285E8E;
        }
    )");

    auto* rememberRow = new QHBoxLayout();
    rememberRow->setContentsMargins(0, 0, 0, 0);

    rememberRow->addWidget(rememberCheck);
    rememberRow->addStretch();

    auto* forgotPassword = new QLabel(
        QString::fromUtf8("Quên mật khẩu"),
        card
    );

    forgotPassword->setCursor(Qt::PointingHandCursor);

    forgotPassword->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #285E8E;
            font-size: 12px;
            font-weight: 500;
        }

        QLabel:hover {
            color: #1F4F78;
        }
    )");

    rememberRow->addWidget(forgotPassword);

    layout->addLayout(rememberRow);
    layout->addSpacing(23);

    auto* loginButton = new QPushButton(
        QString::fromUtf8("ĐĂNG NHẬP"),
        card
    );

    loginButton->setFixedHeight(42);
    loginButton->setCursor(Qt::PointingHandCursor);

    loginButton->setStyleSheet(R"(
        QPushButton {
            background-color: #28629A;
            color: #FFFFFF;
            border: none;
            border-radius: 2px;
            font-size: 14px;
            font-weight: 700;
        }

        QPushButton:hover {
            background-color: #20537F;
        }

        QPushButton:pressed {
            background-color: #194366;
        }
    )");

    layout->addWidget(loginButton);
    layout->addSpacing(40);

    auto* info = new QLabel(
        QString::fromUtf8(
            "Chỉ dành cho quản trị viên và nhân viên trung tâm."
        ),
        card
    );

    info->setAlignment(Qt::AlignCenter);
    info->setWordWrap(true);

    info->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #7B8492;
            font-size: 11px;
        }
    )");

    layout->addWidget(info);

    auto* wrapper = new QVBoxLayout(this);

    wrapper->setContentsMargins(0, 0, 0, 0);

    wrapper->addStretch();
    wrapper->addWidget(
        card,
        0,
        Qt::AlignCenter
    );
    wrapper->addStretch();

    connect(
        loginButton,
        &QPushButton::clicked,
        this,
        &LoginWindow::onLoginClicked
    );

    connect(
        passwordEdit,
        &QLineEdit::returnPressed,
        this,
        &LoginWindow::onLoginClicked
    );
}

void LoginWindow::onLoginClicked()
{
    const QString username =
        usernameEdit->text().trimmed();

    const QString password =
        passwordEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            QString::fromUtf8("Đăng Nhập"),
            QString::fromUtf8(
                "Vui lòng nhập tên đăng nhập và mật khẩu!"
            )
        );

        return;
    }

    if (
        username == "admin" &&
        password == "123456"
    )
    {
        auto* mainWindow = new MainWindow();

        mainWindow->show();

        close();

        return;
    }

    QMessageBox::warning(
        this,
        QString::fromUtf8("Đăng nhập"),
        QString::fromUtf8(
            "Tên đăng nhập hoặc mật khẩu không đúng!"
        )
    );
}
