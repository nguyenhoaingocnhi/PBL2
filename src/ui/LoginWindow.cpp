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
    setWindowTitle("Dang nhap - Trung tam gia su");
    resize(850, 750);
    setMinimumSize(650, 650);

    setStyleSheet(R"(
        LoginWindow {
            background-color: #f3f5f9;
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
    layout->setContentsMargins(38, 34, 38, 30);
    layout->setSpacing(10);

    auto* logoBox = new QLabel("TF", loginCard);
    logoBox->setAlignment(Qt::AlignCenter);
    logoBox->setFixedSize(74, 74);

    logoBox->setStyleSheet(R"(
        QLabel {
            background-color: #ffffff;
            color: #1f5f97;
            border: 2px solid #1f5f97;
            border-radius: 10px;
            font-family: Arial;
            font-size: 24px;
            font-weight: bold;
        }
    )");

    auto* logoLayout = new QHBoxLayout();
    logoLayout->setContentsMargins(0, 0, 0, 0);
    logoLayout->addStretch();
    logoLayout->addWidget(logoBox);
    logoLayout->addStretch();

    layout->addLayout(logoLayout);
    layout->addSpacing(8);

    auto* title = new QLabel("TRUNG TAM GIA SU", loginCard);
    title->setAlignment(Qt::AlignCenter);

    title->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #1f2937;
            font-family: Arial;
            font-size: 23px;
            font-weight: bold;
        }
    )");

    layout->addWidget(title);

    auto* subtitle = new QLabel(
        "He thong quan ly va tim kiem gia su",
        loginCard
    );

    subtitle->setAlignment(Qt::AlignCenter);

    subtitle->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #64748b;
            font-family: Arial;
            font-size: 14px;
        }
    )");

    layout->addWidget(subtitle);
    layout->addSpacing(18);

    auto* usernameLabel = new QLabel(
        "Ten dang nhap",
        loginCard
    );

    usernameLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #374151;
            font-family: Arial;
            font-size: 12px;
            font-weight: bold;
        }
    )");

    layout->addWidget(usernameLabel);

    usernameEdit->setPlaceholderText("Nhap ten dang nhap");
    usernameEdit->setFixedHeight(42);

    usernameEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            color: #374151;
            border: 1px solid #d1d5db;
            border-radius: 4px;
            padding-left: 12px;
            padding-right: 12px;
            font-family: Arial;
            font-size: 14px;
        }

        QLineEdit:focus {
            border: 2px solid #1f5f97;
        }
    )");

    layout->addWidget(usernameEdit);
    layout->addSpacing(8);

    auto* passwordLabel = new QLabel(
        "Mat khau",
        loginCard
    );

    passwordLabel->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #374151;
            font-family: Arial;
            font-size: 12px;
            font-weight: bold;
        }
    )");

    layout->addWidget(passwordLabel);

    passwordEdit->setPlaceholderText("Nhap mat khau");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(42);

    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            color: #374151;
            border: 1px solid #d1d5db;
            border-radius: 4px;
            padding-left: 12px;
            padding-right: 12px;
            font-family: Arial;
            font-size: 14px;
        }

        QLineEdit:focus {
            border: 2px solid #1f5f97;
        }
    )");

    layout->addWidget(passwordEdit);

    auto* rememberRow = new QHBoxLayout();
    rememberRow->setContentsMargins(0, 6, 0, 6);

    rememberCheck->setStyleSheet(R"(
        QCheckBox {
            background: transparent;
            color: #475569;
            font-family: Arial;
            font-size: 13px;
        }

        QCheckBox::indicator {
            width: 14px;
            height: 14px;
            border: 1px solid #cbd5e1;
            border-radius: 2px;
            background: white;
        }

        QCheckBox::indicator:checked {
            background-color: #1f5f97;
            border: 1px solid #1f5f97;
        }
    )");

    rememberRow->addWidget(rememberCheck);
    rememberRow->addStretch();

    auto* forgotPassword = new QLabel(
        "Quen mat khau?",
        loginCard
    );

    forgotPassword->setCursor(Qt::PointingHandCursor);

    forgotPassword->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #1f5f97;
            font-family: Arial;
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
        "DANG NHAP",
        loginCard
    );

    loginButton->setFixedHeight(44);
    loginButton->setCursor(Qt::PointingHandCursor);

    loginButton->setStyleSheet(R"(
        QPushButton {
            background-color: #1f5f97;
            color: white;
            border: none;
            border-radius: 4px;
            font-family: Arial;
            font-size: 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #174b78;
        }

        QPushButton:pressed {
            background-color: #123b5e;
        }
    )");

    layout->addWidget(loginButton);
    layout->addSpacing(30);

    auto* info = new QLabel(
        "Chi danh cho quan tri vien va nhan vien trung tam.",
        loginCard
    );

    info->setAlignment(Qt::AlignCenter);
    info->setWordWrap(true);

    info->setStyleSheet(R"(
        QLabel {
            background: transparent;
            color: #7c8797;
            font-family: Arial;
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
            "Dang nhap",
            "Vui long nhap ten dang nhap va mat khau!"
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
        "Dang nhap",
        "Ten dang nhap hoac mat khau khong dung!"
    );
}
