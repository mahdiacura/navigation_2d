#ifndef CSHAPEFILE_H
#define CSHAPEFILE_H

#include <string>
#include <cstdint>
#include <vector>
#include "Coordinate/ccoordinate.h"
#include "ShapeFile/Shapefile.h"

class CShapeFile{
public:
    void OpenFile(std::string _filePath);
    void CloseFile();
    CCoordinate GetCoordinate(int32_t _coordinateIndex);

    CShapeFile();
    ~CShapeFile();
private:
    std::vector<CCoordinate> m_coordinates;

};


#endif
