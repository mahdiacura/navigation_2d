#include "mainwindow.h"
#include <QApplication>

#include "ShapeFile/cshapefile.h"

int main(int argc, char *argv[])
{
    std::string path = "E:/Workstation/C++/NavigationTest2D/Data/test/test.shp";

    CShapeFile shapeFile;
    shapeFile.OpenFile(path);

    QApplication a(argc, argv);
    MainWindow window;
    window.show();

    return a.exec();
}
