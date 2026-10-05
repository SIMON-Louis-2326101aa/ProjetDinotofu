
#include <QApplication>
#include <QFile>
#include <QTextStream>

#include "interface/qt/MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile("styles/dinotofu.qss");

    if (styleFile.open(QFile::ReadOnly))
    {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
    }

    MainWindow window;
    window.show();

    return app.exec();
}