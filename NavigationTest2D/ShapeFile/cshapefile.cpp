#include "cshapefile.h"

CShapeFile::CShapeFile(){
    m_isOpen = false;
}

CShapeFile::~CShapeFile(){
    CloseFile();
}

void CShapeFile::OpenFile(std::string _filePath){
    m_ways.clear();
    /* Opens the file and ensures it's a valid ESRI shapefile. */
//    std::string path = "E:/Workstation/C++/NavigationTest2D/Data/test/test.shp";
    FILE* pShapefile = open_shapefile(_filePath.c_str());

    if ( pShapefile == 0 ) {
        return;
    }

    /* Reads the shape records and returns them as an SFShape*. */
    SFShapes* pShapes = read_shapes(pShapefile);

    /* We can now iterate over each shape record. */
    for ( uint32_t x = 0; x < pShapes->num_records; ++x ) {
        /* SFShapeRecord describes the shape type, offset, and size in the file. */
        const SFShapeRecord* record = pShapes->records[x];
        /* Read the desired record into an SFPolyLine*. The caller is responsible for freeing this. */
        SFPolyLine* polyline = get_polyline_shape(pShapefile, record);

        //Get the coordinates
        m_ways.push_back(
            CWay(
                CCoordinate(polyline->points[0].x, polyline->points[0].y, 0),
                CCoordinate(polyline->points[1].x, polyline->points[1].y, 0)));

        //Update the rectangle
        //Left
        if (polyline->points[0].x < m_area.m_left)     m_area.m_left      = polyline->points[0].x;
        if (polyline->points[1].x < m_area.m_left)     m_area.m_left      = polyline->points[1].x;
        //Right
        if (polyline->points[0].x > m_area.m_right)    m_area.m_right     = polyline->points[0].x;
        if (polyline->points[1].x > m_area.m_right)    m_area.m_right     = polyline->points[1].x;
        //Top
        if (polyline->points[0].y < m_area.m_top)      m_area.m_top       = polyline->points[0].y;
        if (polyline->points[1].y < m_area.m_top)      m_area.m_top       = polyline->points[1].y;
        //Bottom
        if (polyline->points[0].y > m_area.m_bottom)   m_area.m_bottom    = polyline->points[0].y;
        if (polyline->points[1].y > m_area.m_bottom)   m_area.m_bottom    = polyline->points[1].y;

        /* Done with the polyline; it is our responsibility to free it. */
        free_polyline_shape(polyline);
        polyline = NULL;
    }

    /* We're done with our shape records; ensure that they're freed. */
    free_shapes(pShapes);
    /* We're done with the shapefile; close it. */
    close_shapefile(pShapefile);



    m_isOpen = true;
}

void CShapeFile::CloseFile(){
    m_isOpen = false;
    m_ways.clear();
}

CWay CShapeFile::GetWay(int32_t _wayIndex){
    return m_ways[_wayIndex];
}

int32_t CShapeFile::GetWaysCount(){
    return m_ways.size();
}

