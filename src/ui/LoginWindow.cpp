#include "LoginWindow.h"
#include "MainWindow.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent),
      usernameEdit(new QLineEdit(this)),
      passwordEdit(new QLineEdit(this)),
      rememberCheck(new QCheckBox(this)) {
    setWindowTitle(QString::fromUtf8("Đăng nhập - Trung tâm gia sư"));
    resize(700, 650);
    setMinimumSize(600, 600);
    setStyleSheet("LoginWindow { background-color: #F4F5F7; font-family: \\\"Segoe UI\\\", Arial, sans-serif; }");

    auto* card = new QFrame(this);
    card->setObjectName("loginCard");
    card->setFixedWidth(400);
    card->setStyleSheet("QFrame#loginCard { background-color: #FCFCFD; border: 1px solid #E3E6EA; border-radius: 12px; }");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(38, 30, 38, 30);
    layout->setSpacing(0);

    auto* logo = new QLabel(card);
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(300, 90);
    const QStringList logoCandidates = {
        QCoreApplication::applicationDirPath() + "/../assets/logo.png",
        QCoreApplication::applicationDirPath() + "/assets/logo.png",
        QStringLiteral("assets/logo.png")
    };
    QPixmap logoPixmap;
    for (const QString& path : logoCandidates) {
        if (QFileInfo::exists(path) && logoPixmap.load(path)) {
            break;
        }
    }
    if (logoPixmap.isNull()) {
        logo->setText("TUTORFLOW");
        logo->setStyleSheet("QLabel { color: #24558A; background: transparent; border: none; font-size: 26px; font-weight: 700; }");
    } else {
        logo->setPixmap(logoPixmap.scaled(300, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    auto* logoRow = new QHBoxLayout();
    logoRow->addStretch();
    logoRow->addWidget(logo);
    logoRow->addStretch();
    layout->addLayout(logoRow);
    layout->addSpacing(15);

    auto* title = new QLabel(QString::fromUtf8("TRUNG TÂM GIA SƯ"), card);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel { background: transparent; color: #2B313B; font-size: 22px; font-weight: 700; }");
    layout->addWidget(title);
    layout->addSpacing(7);

    auto* subtitle = new QLabel(QString::fromUtf8("Hệ thống quản lý và tìm kiếm gia sư"), card);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("QLabel { background: transparent; color: #667085; font-size: 13px; }");
    layout->addWidget(subtitle);
    layout->addSpacing(25);

    auto* usernameLabel = new QLabel(QString::fromUtf8("Tên đăng nhập"), card);
    usernameLabel->setStyleSheet("QLabel { background: transparent; color: #343A45; font-size: 12px; font-weight: 600; }");
    layout->addWidget(usernameLabel);
    layout->addSpacing(7);
    usernameEdit->setPlaceholderText(QString::fromUtf8("Nhập tên đăng nhập"));
    usernameEdit->setFixedHeight(40);
    layout->addWidget(usernameEdit);
    layout->addSpacing(17);

    auto* passwordLabel = new QLabel(QString::fromUtf8("Mật khẩu"), card);
    passwordLabel->setStyleSheet("QLabel { background: transparent; color: #343A45; font-size: 12px; font-weight: 600; }");
    layout->addWidget(passwordLabel);
    layout->addSpacing(7);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Nhập mật khẩu"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(40);
    layout->addWidget(passwordEdit);
    layout->addSpacing(10);

    rememberCheck->setText(QString::fromUtf8("Ghi nhớ đăng nhập"));
    auto* rememberRow = new QHBoxLayout();
    rememberRow->addWidget(rememberCheck);
    rememberRow->addStretch();
    auto* forgotPassword = new QLabel(QString::fromUtf8("Quên mật khẩu"), card);
    forgotPassword->setStyleSheet("QLabel { color: #285E8E; font-size: 12px; }");
    rememberRow->addWidget(forgotPassword);
    layout->addLayout(rememberRow);
    layout->addSpacing(23);

    auto* loginButton = new QPushButton(QString::fromUtf8("ĐĂNG NHẬP"), card);
    loginButton->setFixedHeight(42);
    loginButton->setStyleSheet("QPushButton { background-color: #28629A; color: white; border: none; border-radius: 2px; font-size: 14px; font-weight: 700; } QPushButton:hover { background-color: #20537F; }");
    layout->addWidget(loginButton);
    layout->addSpacing(40);

    auto* info = new QLabel(QString::fromUtf8("Chỉ dành cho quản trị viên và nhân viên trung tâm."), card);
    info->setAlignment(Qt::AlignCenter);
    info->setWordWrap(true);
    info->setStyleSheet("QLabel { background: transparent; color: #7B8492; font-size: 11px; }");
    layout->addWidget(info);

    auto* wrapper = new QVBoxLayout(this);
    wrapper->setContentsMargins(0, 0, 0, 0);
    wrapper->addStretch();
    wrapper->addWidget(card, 0, Qt::AlignCenter);
    wrapper->addStretch();

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::onLoginClicked() {
    const QString username = usernameEdit->text().trimmed();
    const QString password = passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("Đăng nhập"), QString::fromUtf8("Vui lòng nhập tên đăng nhập và mật khẩu!"));
        return;
    }
    if (username == QStringLiteral("admin") && password == QStringLiteral("123456")) {
        auto* mainWindow = new MainWindow();
        mainWindow->show();
        close();
        return;
    }
    QMessageBox::warning(this, QString::fromUtf8("Đăng nhập"), QString::fromUtf8("Tên đăng nhập hoặc mật khẩu không đúng!"));
}
