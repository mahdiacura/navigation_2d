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
	m_coordinates.clear();
	m_distances.clear();
	m_coordinatesCount = 0;
}

//Imports all data from shape file
void CDijkstra::LoadShapeFile(std::string _shapeFilePath){
	FreeMemory();

	//Import the dbf file
	std::string dataBaseFilePath =_shapeFilePath.substr(0, _shapeFilePath.length() - 4) + ".dbf";

	int32_t nShapeType			= 0;
	int32_t entitiesCount		= 0;
	int32_t rowIndex			= 0;
	SHPObject * shape			= nullptr;
	SHPHandle shapeFileHandle	= nullptr;
	DBFHandle shapeDBFileHandle	= nullptr;

	shapeFileHandle		= SHPOpen(_shapeFilePath.c_str(),	"rb" );
	shapeDBFileHandle	= DBFOpen(dataBaseFilePath.c_str(),	"rb");
	if( shapeFileHandle == NULL){
		printf( "Unable to import .dbf file\n");
		return;
	}

	if(shapeDBFileHandle  == NULL ){
		printf("Unable to import .shp file\n");
		return;
	}

	//Extract the data
	int32_t vertexIndex	= 0;
	m_coordinatesCount	= 0;
	SHPGetInfo(shapeFileHandle, &entitiesCount  , &nShapeType, NULL, NULL );
	for(rowIndex = 0; rowIndex < entitiesCount; rowIndex++){
		shape = SHPReadObject(shapeFileHandle, rowIndex);
		if (!(SHPT_ARC == shape->nSHPType && 1 == shape->nParts))continue;
		if (2 < shape->nVertices)continue;	//Ignore points

		bool isOneWay = (1 == DBFReadDoubleAttribute(shapeDBFileHandle, rowIndex, 1)) ? true : false;

		//Get the all vertices of this path
		m_coordinatesCount += shape->nVertices;//! repetitive coordinates will be factored
		for (vertexIndex = 0; vertexIndex < shape->nVertices - 1; vertexIndex++){
			CCoordinate startCoordinate	(shape->padfX[vertexIndex], shape->padfY[vertexIndex], shape->padfZ[vertexIndex]);
			CCoordinate endCoordinate	(shape->padfX[vertexIndex + 1], shape->padfY[vertexIndex + 1], shape->padfZ[vertexIndex + 1]);
			AddWay(CWay(startCoordinate, endCoordinate, isOneWay));
		}

	}

	//Area
	//X
	m_area.m_left	= shape->dfXMin;
	m_area.m_right	= shape->dfXMax;
	//Y
	m_area.m_bottom	= shape->dfYMin;
	m_area.m_top	= shape->dfYMax;
	//Z
	m_area.m_back	= shape->dfZMin;
	m_area.m_front	= shape->dfZMax;

	//Free memory of shape files
	SHPClose(shapeFileHandle);
	DBFClose(shapeDBFileHandle);
}

void CDijkstra::AddCoordinate(CCoordinate _coordinate){
    m_coordinates.push_back(_coordinate);
	m_coordinatesCount++;
}

void CDijkstra::SetDistance(CWay & _way){
	m_startIndex = m_endIndex = NONE_INDEX;

	//! hotspot
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

void CDijkstra::GenerateDistancesMatrix(){
    m_coordinates.clear();

    //Clear the distances matrix
    m_distances.clear();//! extra
	//! optimise. high memroy. count the coordinates instead of ways and then optimise
//	m_distances.resize(m_ways.size() * 2, std::vector<double>(m_ways.size() * 2, INFINITE_DISTANCE));
//	m_preCoordinateIndexes.resize(m_ways.size() * 2, NONE_INDEX);
	m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount, INFINITE_DISTANCE));
	m_preCoordinateIndexes.resize(m_coordinatesCount, NONE_INDEX);

	m_coordinatesCount = 0;//Reset to count the non-repetitive coordinates, then resize the buffers again
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

	//Resize the buffer to non-repetitive coordinates
	m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount));
	m_preCoordinateIndexes.resize(m_coordinatesCount);

}

int32_t CDijkstra::GetIndex(CCoordinate & _coordinate){
    m_coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), _coordinate);
	if (m_coordinates.end() == m_coordinateIterator)return NONE_INDEX;

    return (m_coordinateIterator - m_coordinates.begin());
}

int32_t CDijkstra::FindNearestCoordinate(CCoordinate _coordinate){
	int32_t index			= 0;
	double_t distance		= 0;
	double_t minDistance	= INFINITE_DISTANCE;
	int32_t minIndex		= NONE_INDEX;

//	m_coordinatesCount = m_coordinates.size();
	for (index = 0; index < m_coordinatesCount; index++){
		distance =
			std::pow(_coordinate.m_x - m_coordinates[index].m_x, 2) +
			std::pow(_coordinate.m_y - m_coordinates[index].m_y, 2) +
			std::pow(_coordinate.m_z - m_coordinates[index].m_z, 2);

		if (0 == distance)return index;

		if (INFINITE_DISTANCE == minDistance){
			minIndex	= index;
			minDistance	= distance;
		}else if (distance < minDistance){
			minIndex	= index;
			minDistance	= distance;
		}
	}

	return minIndex;
}

bool CDijkstra::IsConnected(int32_t _startIndex, int32_t _endIndex){
    if (_startIndex == _endIndex)return false;
    return (INFINITE_DISTANCE != m_distances[_startIndex][_endIndex]);
}

std::vector<CCoordinate> CDijkstra::FindShortestPath(
	CCoordinate _source, CCoordinate _destination, double & _pathDistance){
    std::vector<CCoordinate> shortestPath;
	double minDistance          = 0;
    int32_t index               = 0;
    int32_t minDistanceIndex    = 0;
    int32_t currentIndex        = 0;
	std::vector<int32_t> unvisitedIndexes;

	GenerateDistancesMatrix();

    //Initialize the unvisited vector to index
    for (index = 0; index < m_coordinatesCount; index++)
		unvisitedIndexes.push_back(index);

    //Find the index of coordinates in distances matrix
    m_startIndex = GetIndex(_source);
	if (NONE_INDEX == m_startIndex){
		m_startIndex = FindNearestCoordinate(_source);
		if (NONE_INDEX == m_startIndex){
			std::cout << "The source point doesn't exist in map\n";
			return std::vector<CCoordinate>(0);
		}
    }

    m_endIndex = GetIndex(_destination);
	if (NONE_INDEX == m_endIndex){
		m_endIndex = FindNearestCoordinate(_destination);
		if (NONE_INDEX == m_endIndex){
			std::cout << "The destination point doesn't exist in map\n";
			return std::vector<CCoordinate>(0);
		}
    }

    //The djacent coordinates to the current coordinate have the value on distances matrix
	//Start Coordinate Initialization
    m_preCoordinateIndexes[m_startIndex] = m_startIndex;
	unvisitedIndexes.erase(unvisitedIndexes.begin() + m_startIndex);
    m_distances[m_startIndex][m_startIndex] = 0;

    currentIndex = m_startIndex;
	//! optimise. O(n2)
    for (int32_t counter = 0; counter < m_coordinatesCount; counter++){
        //Find the nearest adjacent coordinate to current coordinate
        minDistance         = INFINITE_DISTANCE;
		minDistanceIndex    = NONE_INDEX;
        for (index = 0; index < m_coordinatesCount; index++){
            if (!IsConnected(currentIndex, index) || index == m_startIndex)continue;
			if (m_preCoordinateIndexes[currentIndex] == index)continue;	//Ignore pre coordinates

			//! check it
			{//Ignore if it has been visited ago
				std::vector<int32_t>::iterator result =
					std::find(unvisitedIndexes.begin(), unvisitedIndexes.end(), index);
				if (result == unvisitedIndexes.end())continue;
			}

            //Initialize the distance from source to current coordinate
            if (INFINITE_DISTANCE == m_distances[index][index]){
                m_distances[index][index]  = m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index];
                m_preCoordinateIndexes[index] = currentIndex;
            }else if (m_distances[index][index] >
                      m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index] &&
                      INFINITE_DISTANCE != m_distances[currentIndex][index]){
                m_distances[index][index]       = m_distances[currentIndex][currentIndex] + m_distances[currentIndex][index];
                m_preCoordinateIndexes[index]   = currentIndex;
            }

            if ((index != m_startIndex) &&
                (minDistance > m_distances[currentIndex][index] || INFINITE_DISTANCE == minDistance)){
                minDistance         = m_distances[currentIndex][index];
                minDistanceIndex    = index;
            }
        }

        //Update the distance and continue
        if (INFINITE_DISTANCE != minDistanceIndex){
            //Check is there the minDistanceIndex in the unvisisted list or not
            std::vector<int32_t>::iterator currentCoordinate =
				std::find(unvisitedIndexes.begin(), unvisitedIndexes.end(), minDistanceIndex);
			if (currentCoordinate == unvisitedIndexes.end())
				if (unvisitedIndexes.size() > 0)
					currentIndex = unvisitedIndexes[0];
                else
                    break;
            else
                currentIndex = minDistanceIndex;
        }else{
			if (unvisitedIndexes.size() > 0)
				currentIndex = unvisitedIndexes[0];
            else
                break;
        }

        //Remove current coordinate from unvisited coordinates
        std::vector<int32_t>::iterator currentCoordinate =
			std::find(unvisitedIndexes.begin(), unvisitedIndexes.end(), currentIndex);
		if (currentCoordinate != unvisitedIndexes.end())
			unvisitedIndexes.erase(currentCoordinate);
        else
			if (unvisitedIndexes.size() > 0)
				currentIndex = unvisitedIndexes[0];
            else
                break;
    }

    //Make the path
    int32_t pathCounter = 1;
    _pathDistance        = 0;
    shortestPath.clear();
    shortestPath.push_back(m_coordinates[m_endIndex]);
    for (;m_endIndex != m_startIndex;){
		if (INFINITE_DISTANCE != m_preCoordinateIndexes[m_endIndex]){
			_pathDistance += m_distances[m_preCoordinateIndexes[m_endIndex]][m_endIndex];
			shortestPath.push_back(m_coordinates[m_preCoordinateIndexes[m_endIndex]]);
			m_endIndex = m_preCoordinateIndexes[m_endIndex];
		}else{	//The path is disconnect
			shortestPath.clear();
			_pathDistance = 0;
			break;
		}

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

	FreeMemory();

    return shortestPath;
}

