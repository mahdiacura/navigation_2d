#ifndef CSHAPEFILE_H
#define CSHAPEFILE_H

#include <string>
#include <cstdint>
#include <vector>
#include "Coordinate/cway.h"
#include "ShapeFile/Shapefile.h"
#include "Coordinate/crectangle.h"

class CShapeFile{
public:
    void OpenFile(std::string _filePath);
    void CloseFile();
    CWay GetWay(int32_t _wayIndex);
    int32_t GetWaysCount();
    SRectangle m_area;

    CShapeFile();
    ~CShapeFile();
private:
    std::vector<CWay> m_ways;
    bool m_isOpen;

};


#endif
