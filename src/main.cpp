#include "ui/LoginWindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QStringList>

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

    // Tìm thư mục chứa "data" rồi chuyển thư mục làm việc về đó, để chạy được dù mở app
    // bằng run_app.bat, bấm đúp file .exe hay chạy từ IDE (FileManager dùng đường dẫn "data/...").
    {
        QDir appDir(QCoreApplication::applicationDirPath());
        const QStringList candidates = {
            QDir::currentPath(),
            appDir.absoluteFilePath(".."),
            appDir.absolutePath(),
            appDir.absoluteFilePath("../.."),
        };
        for (const QString& base : candidates) {
            if (QDir(base).exists("data")) {
                QDir::setCurrent(QDir(base).absolutePath());
                break;
            }
        }
    }

    LoginWindow loginWindow;
    loginWindow.resize(420, 520);
    loginWindow.show();

    return app.exec();
}
