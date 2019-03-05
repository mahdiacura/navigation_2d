#include "mainwindow.h"
#include <QApplication>

#include "ShapeFile/cshapefile.h"
#include "Dijkstra/cdijkstra.h"

#include <algorithm>
#include <cstdint>
#include <iostream>

int main(int argc, char *argv[])
{
    //std::string path = "E:/Workstation/C++/NavigationTest2D/Data/test/test.shp";
    //CShapeFile shapeFile;
    //shapeFile.OpenFile(path);

    CCoordinate a(0, 0, 0), b(5, 1, 0), c(7, 0, 0), d(3, 2, 0), e(4, 5, 0), f(9, 5, 0);
    CDijkstra dijkstra;
    dijkstra.AddWay(CWay(a, b, true));
    dijkstra.AddWay(CWay(a, d, true));
    dijkstra.AddWay(CWay(b, c, true));
    dijkstra.AddWay(CWay(d, e, true));
    dijkstra.AddWay(CWay(e, c, true));
    dijkstra.AddWay(CWay(d, f, true));
    dijkstra.AddWay(CWay(f, c, true));
    dijkstra.GenerateDistancesMatrix(6);


    std::vector<CCoordinate> shortestPath =
        dijkstra.FindShortestPath(a, c);
    std::cout << "\nShortest path " << shortestPath.size() << " points:\n";
    for (std::vector<CCoordinate>::iterator coordinates = shortestPath.begin();
         coordinates != shortestPath.end();
         coordinates++){
        std::cout << "(" <<
            coordinates->m_x << ", " <<
            coordinates->m_y << ", " <<
            coordinates->m_z << ")\n";
    }
    std::cout.flush();

    QApplication application(argc, argv);
    MainWindow window;
    window.show();

    return application.exec();
}
