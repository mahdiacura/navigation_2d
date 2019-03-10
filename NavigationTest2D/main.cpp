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
}
