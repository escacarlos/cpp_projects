#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("WifiMap"));
    QApplication::setOrganizationName(QStringLiteral("Universite Laval"));

    MainWindow window;
    window.show();
    return application.exec();
}
