#include "Root.h"

#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    gui::Root s;
    return app.exec();
}
