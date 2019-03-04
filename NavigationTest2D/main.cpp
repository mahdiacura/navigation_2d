#include "mainwindow.h"
#include <QApplication>

#include "ShapeFile/cshapefile.h"

int main(int argc, char *argv[])
{
    /* Opens the file and ensures it's a valid ESRI shapefile. */
    std::string path = "E:/Workstation/C++/NavigationTest2D/Data/test/test.shp";
    FILE* pShapefile = open_shapefile(path.c_str());

    if ( pShapefile == 0 ) {
        return 1;
    }

    /* Reads the shape records and returns them as an SFShape*. */
    SFShapes* pShapes = read_shapes(pShapefile);

    /* We can now iterate over each shape record. */
    for ( uint32_t x = 0; x < pShapes->num_records; ++x ) {
        /* SFShapeRecord describes the shape type, offset, and size in the file. */
        const SFShapeRecord* record = pShapes->records[x];
        /* Read the desired record into an SFPolyLine*. The caller is responsible for freeing this. */
        SFPolyLine* polyline = get_polyline_shape(pShapefile, record);

        /* Done with the polyline; it is our responsibility to free it. */
        free_polyline_shape(polyline);
        polyline = NULL;
    }

    /* We're done with our shape records; ensure that they're freed. */
    free_shapes(pShapes);
    /* We're done with the shapefile; close it. */
    close_shapefile(pShapefile);


    QApplication a(argc, argv);
    MainWindow window;
    window.show();

    return a.exec();
}
