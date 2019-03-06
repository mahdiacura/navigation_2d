#ifndef CDIJKSTRA_H
#define CDIJKSTRA_H

#include "ShapeFile/cshapefile.h"
#include "../Coordinate/ccoordinate.h"
#include "../Coordinate/cbox.h"
#include <vector>
#include "../Coordinate/cway.h"
#include <algorithm>
#include <iostream>

#define INFINITE -1

class CDijkstra{
public:
    void AddWay(CWay _coordinate);
    std::vector<CCoordinate> FindShortestPath(
        CCoordinate _source,
        CCoordinate _destination,
        double & _pathDistance);

    CDijkstra();
    ~CDijkstra();
//private:
    std::vector<std::vector<double>> m_distances;
    std::vector<CWay> m_ways;//! make it private
    std::vector<CCoordinate> m_coordinates; //! make it private
    std::vector<CCoordinate>::iterator m_coordinateIterator;
    std::vector<int32_t> m_preCoordinateIndexes;
    int32_t m_coordinatesCount;
    int32_t m_startIndex;
    int32_t m_endIndex;
    SBox m_area;

    bool IsConnected(int32_t _startIndex, int32_t _endIndex);
    int32_t GetIndex(CCoordinate & _coordinate);
    void SetDistance(CWay & _way);
    void AddCoordinate(CCoordinate _coordinate);
    void GenerateDistancesMatrix(int32_t _coordinatesCount);
    void FreeMemory();
};

#endif
