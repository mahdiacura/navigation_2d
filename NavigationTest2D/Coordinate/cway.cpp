#include "cway.h"

CWay::CWay(CCoordinate _startCoordinate, CCoordinate _endCoordinate, bool _isOneWay){
    m_startCoordinate   = _startCoordinate;
    m_endCoordinate     = _endCoordinate;
    m_isOneWay          = _isOneWay;

    //Use Distance^2 for permormance
    m_distance =
        std::pow(m_endCoordinate.m_x - m_startCoordinate.m_x, 2) +
        std::pow(m_endCoordinate.m_y - m_startCoordinate.m_y, 2) +
        std::pow(m_endCoordinate.m_z - m_startCoordinate.m_z, 2);

}

bool CWay::operator!= (const CWay & _way){
    return (!((*this) == _way));
}

bool CWay::operator== (const CWay & _way){
    return (m_startCoordinate   == _way.m_startCoordinate &&
            m_endCoordinate     == _way.m_endCoordinate);
}
