#include <QtWidgets/qapplication.h>

#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow main_window;

    main_window.show();
    return app.exec();
}