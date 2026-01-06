#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Cp.FTP");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Cp.FTP");

    MainWindow window;
    window.show();

    return app.exec();
}
