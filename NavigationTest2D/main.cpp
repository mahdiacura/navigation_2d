#include "mainwindow.h"
#include <QApplication>

#include "Dijkstra/cdijkstra.h"

#include <QLabel>
#include <QSurfaceFormat>
#ifndef QT_NO_OPENGL
#include "OpenGL/openglwindow.h"
#endif

#include <algorithm>
#include <cstdint>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    application.setApplicationName("2D Navigation");
    application.setApplicationVersion("1.2");
    #ifndef QT_NO_OPENGL
        OpenGLWindow openGLWindow;
        openGLWindow.resize(1280, 720);
        openGLWindow.show();
    #else
        QLabel note("OpenGL Support required");
        note.show();
    #endif
    return application.exec();

    //std::string path = "E:/Workstation/C++/NavigationTest2D/Data/test/test.shp";
    //CShapeFile shapeFile;
    //shapeFile.OpenFile(path);



//    CCoordinate a(0, 0, 0), b(5, 1, 0), c(7, 0, 0), d(3, 2, 0), e(4, 5, 0), f(9, 5, 0);
//    CDijkstra dijkstra;
//    dijkstra.AddWay(CWay(a, b, true));
//    dijkstra.AddWay(CWay(a, d, false));
//    dijkstra.AddWay(CWay(b, c, true));
//    dijkstra.AddWay(CWay(d, e, false));
//    dijkstra.AddWay(CWay(e, c, true));
//    dijkstra.AddWay(CWay(d, f, false));
//    dijkstra.AddWay(CWay(f, c, true));
//    dijkstra.AddWay(CWay(e, b, true));
//    dijkstra.GenerateDistancesMatrix(6);

//    double pathDistance = 0;
//    std::vector<CCoordinate> shortestPath =
//        //dijkstra.FindShortestPath(a, c, pathDistance);//Succeeded
//        dijkstra.FindShortestPath(d, c, pathDistance);//Succeeded
//        //dijkstra.FindShortestPath(c, f, pathDistance);//Succeeded
//        //dijkstra.FindShortestPath(e, a, pathDistance);//Succeeded
//        //dijkstra.FindShortestPath(e, f, pathDistance);//Succeeded
//        //dijkstra.FindShortestPath(e, b, pathDistance);//Succeeded
//    std::cout << "\nShortest path has " << pathDistance << " distance and " << shortestPath.size() << " points:\n";
//    for (std::vector<CCoordinate>::iterator coordinates = shortestPath.begin();
//         coordinates != shortestPath.end();
//         coordinates++){
//        std::cout << "(" <<
//            coordinates->m_x << ", " <<
//            coordinates->m_y << ", " <<
//            coordinates->m_z << ")\n";
//    }
//    std::cout.flush();

      //Qt Application
//    QApplication application(argc, argv);
//    MainWindow window;
//    window.show();

//    return application.exec();
}
