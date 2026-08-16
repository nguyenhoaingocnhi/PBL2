#include "LoginWindow.h"

#include "MainWindow.h"
#include "repositories/FileManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QFrame>

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent), usernameEdit(new QLineEdit(this)), passwordEdit(new QLineEdit(this)), rememberCheck(new QCheckBox("Ghi nhớ đăng nhập", this)) {
    auto* root = new QFrame(this);
    root->setObjectName("loginCard");
    root->setFixedWidth(430);
    root->setStyleSheet("QFrame#loginCard { background: #ffffff; border: 1px solid #dfe7f2; border-radius: 16px; } ");

    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* logoBox = new QLabel("Logo");
    logoBox->setAlignment(Qt::AlignCenter);
    logoBox->setFixedHeight(52);
    logoBox->setStyleSheet("background:#eef3fa; border:1px solid #dfe7f2; border-radius:8px; color:#0d6efd; font-weight:700;");

    auto* title = new QLabel("TRUNG TÂM GIA SƯ");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 28px; font-weight: 700; color: #0f172a; letter-spacing: 0.5px;");

    auto* subtitle = new QLabel("Hệ thống quản lý và tìm kiếm gia sư");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color: #64748b; font-size: 14px;");

    auto* usernameLabel = new QLabel("Tên đăng nhập");
    usernameLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    usernameEdit->setPlaceholderText("Nhập tên đăng nhập");
    usernameEdit->setStyleSheet("QLineEdit { border: 1px solid #dfe7f2; border-radius: 8px; padding: 10px 12px; font-size: 14px; }");

    auto* passwordLabel = new QLabel("Mật khẩu");
    passwordLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    passwordEdit->setPlaceholderText("Nhập mật khẩu");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet("QLineEdit { border: 1px solid #dfe7f2; border-radius: 8px; padding: 10px 12px; font-size: 14px; }");

    auto* rememberRow = new QHBoxLayout();
    rememberCheck->setStyleSheet("QCheckBox { color: #475569; }");
    rememberRow->addWidget(rememberCheck);
    rememberRow->addStretch();

    auto* forgot = new QLabel("Quên mật khẩu?");
    forgot->setStyleSheet("color: #2563eb; font-size: 12px;");
    rememberRow->addWidget(forgot);

    auto* loginButton = new QPushButton("ĐĂNG NHẬP");
    loginButton->setObjectName("primaryButton");
    loginButton->setFixedHeight(43);
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);

    auto* wrapper = new QVBoxLayout(this);
    wrapper->setContentsMargins(0, 0, 0, 0);
    wrapper->addStretch();
    wrapper->addWidget(root, 0, Qt::AlignCenter);
    wrapper->addStretch();

    layout->addWidget(logoBox);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(8);
    layout->addWidget(usernameLabel);
    layout->addWidget(usernameEdit);
    layout->addSpacing(4);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit);
    layout->addLayout(rememberRow);
    layout->addWidget(loginButton);
}

void LoginWindow::onLoginClicked() {
    const QString username = usernameEdit->text().trimmed();
    const QString password = passwordEdit->text();

    if (FileManager::validateAdminLogin(username.toStdString(), password.toStdString())) {
        auto* mainWindow = new MainWindow();
        mainWindow->show();
        this->close();
        return;
    }

    QMessageBox::warning(this, "Đăng nhập", "Tên đăng nhập hoặc mật khẩu không đúng!");
}
