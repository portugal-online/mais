#include <QApplication>
#include "grapher.h"
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Grapher *g = new Grapher();
    g->setupui();
    g->show();
    app.exec();
    delete(g);
}
