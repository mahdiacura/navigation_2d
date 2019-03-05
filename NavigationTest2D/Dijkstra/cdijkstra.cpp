#include "cdijkstra.h"

void CDijkstra::AddWay(CWay _way){
    m_ways.push_back(_way);
}

CDijkstra::CDijkstra(){
    m_coordinatesCount  = 0;
//    m_row               = 0;
}

CDijkstra::~CDijkstra(){
    FreeMemory();
}

void CDijkstra::FreeMemory(){
    m_ways.clear();
}

void CDijkstra::AddCoordinate(CCoordinate _coordinate){
    //Set 0 to digonal row of distances matrix
    //m_row = m_coordinates.size();
    //m_distances[m_row][m_row] = 0;
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
}

int32_t CDijkstra::GetIndex(CCoordinate & _coordinate){
    m_coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), _coordinate);
    if (m_coordinates.end() == m_coordinateIterator)return -1;

    return (m_coordinateIterator - m_coordinates.begin());
}

std::vector<CCoordinate> CDijkstra::FindShortestPath(
    CCoordinate _source, CCoordinate _destination){
    std::vector<CCoordinate> shortestPath;
//    std::vector<CCoordinate> unvisitedCoordinates = m_coordinates;
    double minDistance          = 0;
    int32_t index               = 0;
    int32_t minDistanceIndex    = 0;
    int32_t currentIndex        = 0;
    std::vector<int32_t> unvisitedCoordinates;
    //Initialize the unvisited vector to index
    for (index = 0; index < m_coordinatesCount; index++)
        unvisitedCoordinates.push_back(index);

    //Find the index of coordinates in distances matrix
    m_startIndex = GetIndex(_source);
    if (-1 == m_startIndex){
        std::cout << "The source point doesn't exist in map\n";
        goto error;
    }

    m_endIndex = GetIndex(_destination);
    if (-1 == m_endIndex){
        std::cout << "The destination point doesn't exist in map\n";
        goto error;
    }

    //The djacent coordinates to the current coordinate have the value on distances matrix
    unvisitedCoordinates.erase(unvisitedCoordinates.begin() + m_startIndex);
    currentIndex = m_startIndex;
    //while (!unvisitedCoordinates.empty()){
    for (int32_t counter = 0; counter < m_coordinatesCount; counter++){
        //Find the nearest adjacent coordinate to
        minDistance         = INFINITE;
        minDistanceIndex    = -1;
        for (index = 0; index < m_coordinatesCount; index++){
            //Initialize the distance from source to current coordinate
            if (INFINITE == m_distances[index][index]){
                if (currentIndex == index)
                    m_distances[index][index] = 0;
                else
                    m_distances[index][index] = m_distances[currentIndex][index];//! check it
            }else if (m_distances[index][index] >
                      m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index] &&
                      INFINITE != m_distances[currentIndex][index]){
                m_distances[index][index] = m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index];
            }

            if ((currentIndex != index && INFINITE != m_distances[currentIndex][index]) &&
                (minDistance > m_distances[currentIndex][index] || INFINITE == minDistance)){
                minDistance         = m_distances[currentIndex][index];
                minDistanceIndex    = index;
            }
        }

        if (INFINITE != minDistanceIndex){
            m_distances[minDistanceIndex][minDistanceIndex] =
                m_distances[currentIndex][currentIndex] + m_distances[currentIndex][minDistanceIndex];

            //Check is there the minDistanceIndex in the unvisisted list or not
            std::vector<int32_t>::iterator currentCoordinate =
                std::find(unvisitedCoordinates.begin(), unvisitedCoordinates.end(), minDistanceIndex);
            if (currentCoordinate == unvisitedCoordinates.end())
                if (unvisitedCoordinates.size() > 0)
                    currentIndex = unvisitedCoordinates[0];//!
                else
                    break;
            else
                currentIndex = minDistanceIndex;
        }else{
            currentIndex = unvisitedCoordinates[0];//!
        }

        //Remove current coordinate from unvisited coordinates
        std::vector<int32_t>::iterator currentCoordinate =
            std::find(unvisitedCoordinates.begin(), unvisitedCoordinates.end(), currentIndex);
        if (currentCoordinate != unvisitedCoordinates.end())
            unvisitedCoordinates.erase(currentCoordinate);
        else
            currentIndex = unvisitedCoordinates[0];//!
        int32_t xxx = 0;
    }

    return shortestPath;

    error:;
    return std::vector<CCoordinate>(0);
}

