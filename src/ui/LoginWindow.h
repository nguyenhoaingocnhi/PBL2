#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

private slots:
    void onLoginClicked();

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QCheckBox* rememberCheck;
};

#endif
