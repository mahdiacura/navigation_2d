#include "cdijkstra.h"

void CDijkstra::AddWay(CWay _way){
    m_ways.push_back(_way);
}

CDijkstra::CDijkstra(){
    m_coordinatesCount  = 0;
    m_row               = 0;
}

CDijkstra::~CDijkstra(){
    FreeMemory();
}

void CDijkstra::FreeMemory(){
    m_ways.clear();
}

void CDijkstra::AddCoordinate(CCoordinate _coordinate){
    //Set 0 to digonal row of distances matrix
    m_row = m_coordinates.size();
    m_distances[m_row][m_row] = 0;
    m_coordinates.push_back(_coordinate);
}

void CDijkstra::SetDistance(CWay & _way){
    m_startIndex = m_endIndex = -1;

    //Start Coordinate
    m_coordinateIterator = std::find(
        m_coordinates.begin(),
        m_coordinates.end(),
        _way.m_startCoordinate);
    m_startIndex = m_coordinateIterator - m_coordinates.begin();

    //End Coordinate
    m_coordinateIterator = std::find(
        m_coordinates.begin(),
        m_coordinates.end(),
        _way.m_endCoordinate);
    m_endIndex = m_coordinateIterator - m_coordinates.begin();

    m_distances[m_startIndex][m_endIndex] = _way.m_distance;
}

void CDijkstra::GenerateDistancesMatrix(int32_t _coordinatesCount){
    m_coordinatesCount = _coordinatesCount;
    m_coordinates.clear();
    //Clear the distances matrix
    m_distances.clear();
    m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount, INFINITE));

    //Extract the coordinates
    for (std::vector<CWay>::iterator ways = m_ways.begin(); ways != m_ways.end(); ways++){
        //Search for the start coordinate
        m_coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), ways->m_startCoordinate);
        if (m_coordinateIterator == m_coordinates.end())
            AddCoordinate(ways->m_startCoordinate);

        //Search for the end coordinate
        m_coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), ways->m_endCoordinate);
        if (m_coordinateIterator == m_coordinates.end())
            AddCoordinate(ways->m_endCoordinate);

        SetDistance(*ways);
    }


    int32_t x = 0;

}
