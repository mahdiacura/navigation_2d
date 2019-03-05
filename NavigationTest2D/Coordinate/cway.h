#ifndef CPOINT_H
#define CPOINT_H

#include <cstdint>
#include "ccoordinate.h"
#include <vector>
#include <cmath>

class CWay{
public:
    CCoordinate m_startCoordinate;
    CCoordinate m_endCoordinate;
    double m_distance;
    bool m_isOneWay;

    bool operator!= (const CWay & _way);
    bool operator== (const CWay & _way);
    CWay(CCoordinate _startCoordinate, CCoordinate _endCoordinate, bool _isOneWay);
private:

};

#endif
