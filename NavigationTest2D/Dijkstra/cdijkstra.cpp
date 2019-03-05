#include "cdijkstra.h"

void CDijkstra::AddWay(CWay _way){
    m_ways.push_back(_way);
}

CDijkstra::CDijkstra(){
    m_coordinatesCount = 0;
}

CDijkstra::~CDijkstra(){
    FreeMemory();
}

void CDijkstra::FreeMemory(){
    m_ways.clear();
}

void CDijkstra::GenerateDistancesMatrix(int32_t _coordinatesCount){
    m_coordinatesCount = _coordinatesCount;
    m_coordinates.clear();
    //Clear the distances matrix
    m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount, 0));

    //Add the coordinates to first row of distances matrix
    std::vector<CCoordinate>::iterator coordinateIterator;
    for (std::vector<CWay>::iterator ways = m_ways.begin(); ways != m_ways.end(); ways++){
        //Search for the start coordinate
        coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), ways->m_startCoordinate);
        if (coordinateIterator == m_coordinates.end())
            m_coordinates.push_back(ways->m_startCoordinate);

        //Search for the end coordinate
        coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), ways->m_endCoordinate);
        if (coordinateIterator == m_coordinates.end())
            m_coordinates.push_back(ways->m_endCoordinate);

    }
    int32_t x = 0;

}
