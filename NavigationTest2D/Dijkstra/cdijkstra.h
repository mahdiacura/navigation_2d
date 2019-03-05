#ifndef CDIJKSTRA_H
#define CDIJKSTRA_H

#include "../Coordinate/ccoordinate.h"
#include <vector>
#include "../Coordinate/cway.h"
#include <algorithm>

class CDijkstra{
public:
    void AddWay(CWay _coordinate);
//    std::vector<CCoordinate> FindShortestPath(

    CDijkstra();
    ~CDijkstra();
//private:
    std::vector<std::vector<double>> m_distances;
    std::vector<CWay> m_ways;//! make it private
    std::vector<CCoordinate> m_coordinates; //! make it private
    int32_t m_coordinatesCount;

    void GenerateDistancesMatrix(int32_t _coordinatesCount);
    void FreeMemory();
};

#endif
