#include <QApplication>
#include <QFont>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("BeamPlayground");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("BeamPlayground");

    // Global font
    QFont defaultFont("Monospace", 9);
    app.setFont(defaultFont);

    MainWindow window;
    window.show();

    return app.exec();
}
