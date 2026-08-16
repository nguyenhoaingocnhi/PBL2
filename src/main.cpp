#include "ui/LoginWindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QString stylePath = QCoreApplication::applicationDirPath() + "/../assets/styles/app.qss";
    QFile styleFile(stylePath);
    if (!styleFile.exists()) {
        stylePath = QCoreApplication::applicationDirPath() + "/assets/styles/app.qss";
        styleFile.setFileName(stylePath);
    }
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    LoginWindow loginWindow;
    loginWindow.resize(420, 520);
    loginWindow.show();

    return app.exec();
}
