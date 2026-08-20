```cpp
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
    : QWidget(parent),
      usernameEdit(new QLineEdit(this)),
      passwordEdit(new QLineEdit(this)),
      rememberCheck(new QCheckBox("Ghi nhớ đăng nhập", this))
{
    setWindowTitle("Đăng nhập - Trung tâm gia sư");
    setMinimumSize(700, 700);
    resize(850, 750);

    setStyleSheet(R"(
        QWidget {
            font-family: "Segoe UI";
            background-color: #f3f5f9;
            color: #1f2937;
        }
    )");

    auto* loginCard = new QFrame(this);
    loginCard->setObjectName("loginCard");
    loginCard->setFixedWidth(430);

    loginCard->setStyleSheet(R"(
        QFrame#loginCard {
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
        }
    )");

    auto* layout = new QVBoxLayout(loginCard);
    layout->setContentsMargins(34, 34, 34, 30);
    layout->setSpacing(12);

    auto* logoBox = new QLabel(loginCard);
    logoBox->setAlignment(Qt::AlignCenter);
    logoBox->setFixedSize(74, 74);
    logoBox->setText("♧");

    logoBox->setStyleSheet(R"(
        QLabel {
            background-color: #ffffff;
            border: 1px solid #e5e7eb;
            border-radius: 8px;
            color: #1F5F97;
            font-size: 26px;
            font-weight: bold;
        }
    )");

    auto* logoLayout = new QHBoxLayout();
    logoLayout->addStretch();
    logoLayout->addWidget(logoBox);
    logoLayout->addStretch();

    layout->addLayout(logoLayout);
    layout->addSpacing(8);

    auto* title = new QLabel("TRUNG TÂM GIA SƯ", loginCard);
    title->setAlignment(Qt::AlignCenter);

    title->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #1f2937;
            font-size: 24px;
            font-weight: 700;
        }
    )");

    layout->addWidget(title);

    auto* subtitle = new QLabel(
        "Hệ thống quản lý và tìm kiếm gia sư",
        loginCard
    );

    subtitle->setAlignment(Qt::AlignCenter);

    subtitle->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #64748b;
            font-size: 14px;
        }
    )");

    layout->addWidget(subtitle);
    layout->addSpacing(18);

    auto* usernameLabel = new QLabel(
        "Tên đăng nhập",
        loginCard
    );

    usernameLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            font-size: 12px;
            font-weight: 600;
            color: #374151;
        }
    )");

    layout->addWidget(usernameLabel);

    usernameEdit->setPlaceholderText("Nhập tên đăng nhập");
    usernameEdit->setFixedHeight(42);
    usernameEdit->setClearButtonEnabled(true);

    usernameEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #d1d5db;
            border-radius: 3px;
            padding-left: 14px;
            padding-right: 14px;
            font-size: 14px;
            color: #374151;
        }

        QLineEdit:focus {
            border: 2px solid #1F5F97;
        }
    )");

    layout->addWidget(usernameEdit);
    layout->addSpacing(8);

    auto* passwordLabel = new QLabel(
        "Mật khẩu",
        loginCard
    );

    passwordLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            font-size: 12px;
            font-weight: 600;
            color: #374151;
        }
    )");

    layout->addWidget(passwordLabel);

    passwordEdit->setPlaceholderText("Nhập mật khẩu");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(42);

    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #d1d5db;
            border-radius: 3px;
            padding-left: 14px;
            padding-right: 14px;
            font-size: 14px;
            color: #374151;
        }

        QLineEdit:focus {
            border: 2px solid #1F5F97;
        }
    )");

    layout->addWidget(passwordEdit);

    auto* rememberRow = new QHBoxLayout();
    rememberRow->setContentsMargins(0, 4, 0, 4);

    rememberCheck->setStyleSheet(R"(
        QCheckBox {
            background: transparent;
            color: #475569;
            font-size: 13px;
        }

        QCheckBox::indicator {
            width: 15px;
            height: 15px;
            border: 1px solid #cbd5e1;
            border-radius: 2px;
            background: white;
        }

        QCheckBox::indicator:checked {
            background-color: #1F5F97;
            border: 1px solid #1F5F97;
        }
    )");

    rememberRow->addWidget(rememberCheck);
    rememberRow->addStretch();

    auto* forgotPassword = new QLabel(
        "Quên mật khẩu?",
        loginCard
    );

    forgotPassword->setCursor(Qt::PointingHandCursor);

    forgotPassword->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #1F5F97;
            font-size: 12px;
        }

        QLabel:hover {
            text-decoration: underline;
        }
    )");

    rememberRow->addWidget(forgotPassword);

    layout->addLayout(rememberRow);
    layout->addSpacing(10);

    auto* loginButton = new QPushButton(
        "ĐĂNG NHẬP  →",
        loginCard
    );

    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setFixedHeight(44);

    loginButton->setStyleSheet(R"(
        QPushButton {
            background-color: #1F5F97;
            color: white;
            border: none;
            border-radius: 2px;
            font-size: 15px;
            font-weight: 700;
        }

        QPushButton:hover {
            background-color: #174B78;
        }

        QPushButton:pressed {
            background-color: #123B5E;
        }
    )");

    layout->addWidget(loginButton);
    layout->addSpacing(34);

    auto* info = new QLabel(
        "ⓘ  Chỉ dành cho quản trị viên và nhân viên trung tâm.",
        loginCard
    );

    info->setAlignment(Qt::AlignCenter);
    info->setWordWrap(true);

    info->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #7c8797;
            font-size: 12px;
        }
    )");

    layout->addWidget(info);

    auto* wrapper = new QVBoxLayout(this);
    wrapper->setContentsMargins(0, 0, 0, 0);

    wrapper->addStretch();
    wrapper->addWidget(loginCard, 0, Qt::AlignCenter);
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
    const QString username = usernameEdit->text().trimmed();
    const QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Đăng nhập",
            "Vui lòng nhập tên đăng nhập và mật khẩu!"
        );

        return;
    }

    if (
        FileManager::validateAdminLogin(
            username.toStdString(),
            password.toStdString()
        )
    )
    {
        auto* mainWindow = new MainWindow();

        mainWindow->show();

        close();

        return;
    }

    QMessageBox::warning(
        this,
        "Đăng nhập",
        "Tên đăng nhập hoặc mật khẩu không đúng!"
    );
}
```
