#ifndef CDIJKSTRA_H
#define CDIJKSTRA_H

#include "../Coordinate/ccoordinate.h"
#include <vector>
#include "../Coordinate/cway.h"
#include <algorithm>

#define INFINITE -1

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
    std::vector<CCoordinate>::iterator m_coordinateIterator;
    int32_t m_coordinatesCount;
    int32_t m_row;
    int32_t m_startIndex;
    int32_t m_endIndex;

    void SetDistance(CWay & _way);
    void AddCoordinate(CCoordinate _coordinate);
    void GenerateDistancesMatrix(int32_t _coordinatesCount);
    void FreeMemory();
};

#endif
