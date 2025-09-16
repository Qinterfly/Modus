
#include <config.h>
#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    // Force the light theme
#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#endif

    // Create the application
    QApplication application(argc, argv);

    // Set up the language
    QString lang = "ru";
#ifdef Q_OS_LINUX
    lang = QLocale::system().name();
    lang.truncate(lang.lastIndexOf('_'));
#endif
    Frontend::MainWindow::language = lang;

    // Create the graphical window
    Frontend::MainWindow window;
    window.show();

    return application.exec();
}
