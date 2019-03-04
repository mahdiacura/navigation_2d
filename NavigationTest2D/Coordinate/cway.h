#ifndef CPOINT_H
#define CPOINT_H

#include <cstdint>
#include "ccoordinate.h"

class CWay{
public:
    CCoordinate m_startCoordinate;
    CCoordinate m_endCoordinate;

    CWay(CCoordinate _startCoordinate, CCoordinate _endCoordinate);
};

#endif
