#ifndef CDIJKSTRA_H
#define CDIJKSTRA_H

#include "ccoordinate.h"
#include <vector>

class CDijkstra{
public:
    void AddCoordinate(CCoordinate _coordinate);

private:
    std::vector<CCoordinate> m_coordinates;

    //void SetDistance(CCoordinate _sourceCoordinate, CCoordinate _destinationCoordinate, float _distance);
};

#endif
