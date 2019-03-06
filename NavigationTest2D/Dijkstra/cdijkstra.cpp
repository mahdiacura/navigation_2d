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

    m_distances[m_startIndex][m_endIndex]   = _way.m_distance;
    if (!_way.m_isOneWay)
        m_distances[m_endIndex][m_startIndex] = _way.m_distance;
    m_preCoordinateIndexes[m_endIndex]      = m_startIndex;
}

void CDijkstra::GenerateDistancesMatrix(int32_t _coordinatesCount){
    m_coordinatesCount = _coordinatesCount;
    m_coordinates.clear();

    //Clear the distances matrix
    m_distances.clear();//! extra
    m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount, INFINITE));
    m_preCoordinateIndexes.resize(m_coordinatesCount, -1);

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

bool CDijkstra::IsConnected(int32_t _startIndex, int32_t _endIndex){
    if (_startIndex == _endIndex)return false;
    return (INFINITE != m_distances[_startIndex][_endIndex]);
}

std::vector<CCoordinate> CDijkstra::FindShortestPath(
    CCoordinate _source, CCoordinate _destination, double & _pathDistance){
    std::vector<CCoordinate> shortestPath;
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
        return std::vector<CCoordinate>(0);
    }

    m_endIndex = GetIndex(_destination);
    if (-1 == m_endIndex){
        std::cout << "The destination point doesn't exist in map\n";
        return std::vector<CCoordinate>(0);
    }

    //The djacent coordinates to the current coordinate have the value on distances matrix
    m_preCoordinateIndexes[m_startIndex] = m_startIndex;
    unvisitedCoordinates.erase(unvisitedCoordinates.begin() + m_startIndex);
    m_distances[m_startIndex][m_startIndex] = 0;
    currentIndex = m_startIndex;
    for (int32_t counter = 0; counter < m_coordinatesCount; counter++){
        //Find the nearest adjacent coordinate to current coordinate
        minDistance         = INFINITE;
        minDistanceIndex    = -1;
        for (index = 0; index < m_coordinatesCount; index++){
            if (!IsConnected(currentIndex, index) || index == m_startIndex)continue;

            //Initialize the distance from source to current coordinate
            if (INFINITE == m_distances[index][index]){
                m_distances[index][index]  = m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index];
                m_preCoordinateIndexes[index] = currentIndex;
            }else if (m_distances[index][index] >
                      m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index] &&
                      INFINITE != m_distances[currentIndex][index]){
                m_distances[index][index]       = m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index];
                m_preCoordinateIndexes[index]   = currentIndex;
            }

            if ((index != m_startIndex) &&
                (minDistance > m_distances[currentIndex][index] || INFINITE == minDistance)){
                minDistance         = m_distances[currentIndex][index];
                minDistanceIndex    = index;
            }
        }

        //Update the distance and continue
        if (INFINITE != minDistanceIndex){
            //Check is there the minDistanceIndex in the unvisisted list or not
            std::vector<int32_t>::iterator currentCoordinate =
                std::find(unvisitedCoordinates.begin(), unvisitedCoordinates.end(), minDistanceIndex);
            if (currentCoordinate == unvisitedCoordinates.end())
                if (unvisitedCoordinates.size() > 0)
                    currentIndex = unvisitedCoordinates[0];
                else
                    break;
            else
                currentIndex = minDistanceIndex;
        }else{
            if (unvisitedCoordinates.size() > 0)
                currentIndex = unvisitedCoordinates[0];
            else
                break;
        }

        //Remove current coordinate from unvisited coordinates
        std::vector<int32_t>::iterator currentCoordinate =
            std::find(unvisitedCoordinates.begin(), unvisitedCoordinates.end(), currentIndex);
        if (currentCoordinate != unvisitedCoordinates.end())
            unvisitedCoordinates.erase(currentCoordinate);
        else
            if (unvisitedCoordinates.size() > 0)
                currentIndex = unvisitedCoordinates[0];
            else
                break;
    }

    //Make the path
    int32_t pathCounter = 1;
    _pathDistance        = 0;
    shortestPath.clear();
    shortestPath.push_back(m_coordinates[m_endIndex]);
    for (;m_endIndex != m_startIndex;){
        _pathDistance += m_distances[m_preCoordinateIndexes[m_endIndex]][m_endIndex];
        shortestPath.push_back(m_coordinates[m_preCoordinateIndexes[m_endIndex]]);
        m_endIndex = m_preCoordinateIndexes[m_endIndex];

        //Check is the path connected or not
        pathCounter++;
        if (pathCounter > m_coordinatesCount){      //The path is disconnect
            shortestPath.clear();
            _pathDistance = 0;
            break;
        }

    }
    std::reverse(shortestPath.begin(), shortestPath.end());
    _pathDistance = std::sqrt(_pathDistance);

    return shortestPath;
}

