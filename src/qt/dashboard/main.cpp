#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("VoltGuard Dashboard");
    application.setOrganizationName("VoltGuard");

    QFile stylesheet(application.applicationDirPath() + "/ui/styles.qss");
    if (stylesheet.open(QFile::ReadOnly | QFile::Text)) {
        application.setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    }

    MainWindow window;
    window.show();
    return application.exec();
}
