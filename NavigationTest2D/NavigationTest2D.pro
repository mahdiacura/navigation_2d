#-------------------------------------------------
#
# Project created by QtCreator 2019-03-04T11:13:24
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NavigationTest2D
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
	mainwindow.cpp \
	Dijkstra/cdijkstra.cpp \
	Coordinate/ccoordinate.cpp \
	Coordinate/cway.cpp \
	Coordinate/cbox.cpp \
	OpenGL/openglwindow.cpp \
    ShapeLib/dbfopen.c \
    ShapeLib/safileio.c \
    ShapeLib/sbnsearch.c \
    ShapeLib/shpgeo.c \
    ShapeLib/shpopen.c \
    ShapeLib/shptree.c

HEADERS += \
	mainwindow.h \
	Dijkstra/cdijkstra.h \
	Dijkstra/csystemversion.h \
	Coordinate/ccoordinate.h \
	Coordinate/cway.h \
	Coordinate/cbox.h \
	OpenGL/openglwindow.h \
    ShapeLib/shapefil.h \
    ShapeLib/shpgeo.h



LIBS += -lOpengl32

FORMS += \
        mainwindow.ui

CONFIG += mobility
MOBILITY = 

