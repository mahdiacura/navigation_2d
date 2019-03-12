#include "cdijkstra.h"

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
		if (2 > shape->nVertices)continue;	//Ignore points

		bool isOneWay = (1 == DBFReadDoubleAttribute(shapeDBFileHandle, rowIndex, 1)) ? true : false;

		//Get the all vertices of this path
		m_coordinatesCount += shape->nVertices;//! repetitive coordinates will be factored

		for (vertexIndex = 0; vertexIndex < shape->nVertices - 1; vertexIndex++){
			CCoordinate startCoordinate	(shape->padfX[vertexIndex],
										 shape->padfY[vertexIndex],
										 shape->padfZ[vertexIndex]);
			CCoordinate endCoordinate	(shape->padfX[vertexIndex + 1],
										 shape->padfY[vertexIndex + 1],
										 shape->padfZ[vertexIndex + 1]);
			AddWay(CWay(startCoordinate, endCoordinate, isOneWay));

			//! search and add non-repetitive coordinates here
		}

	}

	//Area
	//X
	m_area.m_left	= shapeFileHandle->adBoundsMin[0];
	m_area.m_right	= shapeFileHandle->adBoundsMax[0];
	//Y
	m_area.m_bottom	= shapeFileHandle->adBoundsMin[1];
	m_area.m_top	= shapeFileHandle->adBoundsMax[1];
	//Z
	m_area.m_back	= shapeFileHandle->adBoundsMin[2];
	m_area.m_front	= shapeFileHandle->adBoundsMax[2];

	//Center of map
	m_centerOfMap = CCoordinate((m_area.m_left + m_area.m_right) / 2,
								(m_area.m_bottom + m_area.m_top) / 2,
								(m_area.m_back + m_area.m_front) / 2);

	//Free memory of shape files
	SHPClose(shapeFileHandle);
	DBFClose(shapeDBFileHandle);
}

void CDijkstra::AddWay(CWay _way){
	m_ways.push_back(_way);
}

CDijkstra::CDijkstra(){
	m_coordinatesCount  = 0;
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

void CDijkstra::AddCoordinate(CCoordinate _coordinate){
    m_coordinates.push_back(_coordinate);
	m_coordinatesCount++;
}

//Checks that is this coordinate in the same neighborhood to
//another coordinate. Return the index of coordinate of this
//or another near coordinate
int32_t CDijkstra::AddOrMergeCoordinate(CCoordinate _coordinate){//! hotspot
	double distance = 0;
	std::vector<CCoordinate>::iterator coordinate;
	for (coordinate = m_coordinates.begin(); coordinate != m_coordinates.end(); coordinate++){
		distance =	std::pow(coordinate->m_x - _coordinate.m_x, 2) +
					std::pow(coordinate->m_y - _coordinate.m_y, 2) +
					std::pow(coordinate->m_z - _coordinate.m_z, 2);
		if (distance < MAX_NEIGHBORHOOD_DISTANCE)return (coordinate - m_coordinates.begin());
	}

	//Doesnt exist, so add it
	m_coordinates.push_back(_coordinate);
//	m_coordinatesCount++;
//	return (m_coordinates.size() - 1);
	return (m_coordinatesCount++);//! make sure
}

void CDijkstra::SetDistance(CWay & _way){//! extra//! hotspot
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

void CDijkstra::SetDistance(CWay & _way, int32_t _sourceCoordinateIndex, int32_t _destinationCoordinateIndex){//! hotspot
	m_startIndex = m_endIndex = NONE_INDEX;

	m_distances[_sourceCoordinateIndex][_destinationCoordinateIndex] = _way.m_distance;//! remove ways and get in loadshapefile()
	if (!_way.m_isOneWay)
		m_distances[_destinationCoordinateIndex][_sourceCoordinateIndex] = _way.m_distance;
	m_preCoordinateIndexes[_destinationCoordinateIndex] = _sourceCoordinateIndex;
}


void CDijkstra::GenerateDistancesMatrix(){
	//Clear the memory
	m_coordinates.clear();
	m_distances.clear();//! extra

	//! optimise. high memroy. count the coordinates instead of ways and then optimise
	m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount, INFINITE_DISTANCE));
	m_preCoordinateIndexes.resize(m_coordinatesCount, NONE_INDEX);

	//Extract the coordinates
	m_coordinatesCount = 0;//Reset to count the non-repetitive coordinates, then resize the buffers again

	//! change the search and make sure the return index of coordinate is correct
	int32_t startIndex = 0, endIndex = 0;
	for (std::vector<CWay>::iterator ways = m_ways.begin(); ways != m_ways.end(); ways++){
		startIndex = AddOrMergeCoordinate(ways->m_startCoordinate);
		endIndex = AddOrMergeCoordinate(ways->m_endCoordinate);
		SetDistance(*ways, startIndex, endIndex);
    }

	//! Resize the buffer to non-repetitive coordinates
//	m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount));
//	m_preCoordinateIndexes.resize(m_coordinatesCount);

	int x = 0;
	x = 5;
}

int32_t CDijkstra::GetIndex(CCoordinate & _coordinate){
    m_coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), _coordinate);
	if (m_coordinates.end() == m_coordinateIterator)return NONE_INDEX;

    return (m_coordinateIterator - m_coordinates.begin());
}

bool CDijkstra::IsIntersection(int32_t _coordinateIndex){
	int32_t adjacentCoordinates = 0;
	for (int32_t index = 0; index < m_coordinatesCount; index++){
		if (_coordinateIndex == index)continue;
		if (INFINITE_DISTANCE != m_distances[_coordinateIndex][index])
			adjacentCoordinates++;
	}

	if (adjacentCoordinates > 1)return true;
	return false;
}

//Reference : https://en.wikipedia.org/wiki/Haversine_formula
//Returns the distance between two points on level of earth in KM
double_t CDijkstra::GetDistanceOnEarth(CCoordinate & _source, CCoordinate & _destination){
	double_t sourceLatitude			= 0, sourceLongitude = 0;
	double_t destinationLatitude	= 0, destinationLongitude = 0;
	double_t u = 0, v = 0;

	sourceLatitude	= DegreeToRadian(_source.m_y);
	sourceLongitude	= DegreeToRadian(_source.m_x);

	destinationLatitude		= DegreeToRadian(_destination.m_y);
	destinationLongitude	= DegreeToRadian(_destination.m_x);
	u = sin((destinationLatitude - sourceLatitude)/2);
	v = sin((destinationLongitude - sourceLongitude)/2);

	return (2.0 * EARTH_RADIUS_IN_KM * std::asin(std::sqrt(u * u + std::cos(sourceLatitude) * std::cos(destinationLatitude) * v * v)));
}

int32_t CDijkstra::FindNearestCoordinate(CCoordinate & _coordinate){
	int32_t index			= 0;
	double_t distance		= 0;
	double_t minDistance	= INFINITE_DISTANCE;
	int32_t minIndex		= NONE_INDEX;

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
	CCoordinate _source,
	CCoordinate _destination,
	double & _pathDistance,
	std::vector<int8_t> & _pathDirections)
{
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
	double currentWeight	= INFINITE_DISTANCE;//! care about storing -1 in double. The result ~ -0.99999999996
	double neighborDistance	= INFINITE_DISTANCE;
	double neighborWeight	= INFINITE_DISTANCE;
	//! optimise. O(n2)
	while (unvisitedIndexes.size()){

        //Find the nearest adjacent coordinate to current coordinate
		{
			minDistance         = INFINITE_DISTANCE;
			minDistanceIndex    = NONE_INDEX;
			for (index = 0; index < m_coordinatesCount; index++){
				if (!IsConnected(currentIndex, index) || index == m_startIndex)continue;
				//! check
				if (m_preCoordinateIndexes[currentIndex] == index)continue;	//Ignore the recursive way

				//! check it
				{//Ignore if it has been visited ago
					std::vector<int32_t>::iterator result =
						std::find(unvisitedIndexes.begin(), unvisitedIndexes.end(), index);
					if (result == unvisitedIndexes.end())continue;
				}

				//Initialize and update the distance from source to current coordinate
				currentWeight		= m_distances[currentIndex][currentIndex];
				neighborDistance	= m_distances[currentIndex][index];
				neighborWeight		= m_distances[index][index];
//				if (currentWeight < 0 && neighborDistance > 0){
//					currentWeight = currentWeight + 0;

//					currentIndex = index;//! continue with next neighbor
//					continue;
//				}

				if (neighborWeight > currentWeight + neighborDistance){
					m_distances[index][index]       = currentWeight + neighborDistance;
					m_preCoordinateIndexes[index]   = currentIndex;
				}

				if ((index != m_startIndex) && (minDistance > neighborDistance)){
					minDistance         = neighborDistance;
					minDistanceIndex    = index;
				}
			}
		}

		//! extra?
        //Update the distance and continue
		{
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

			//! extra?
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

    }


	//Make the shortest path
	{
		_pathDirections.clear();
		_pathDirections.push_back(DIRECTION_GO_STRAIGHT);	//Add the end direction

		IsIntersection(m_startIndex);

		int32_t pathCounter = 1;
		_pathDistance        = 0;
		shortestPath.clear();
		shortestPath.push_back(m_coordinates[m_endIndex]);
		for (;m_endIndex != m_startIndex;){
			if (INFINITE_DISTANCE != m_preCoordinateIndexes[m_endIndex]){
				_pathDistance += GetDistanceOnEarth(
					m_coordinates[m_endIndex],
					m_coordinates[m_preCoordinateIndexes[m_endIndex]]);
				shortestPath.push_back(m_coordinates[m_preCoordinateIndexes[m_endIndex]]);

				//Calculate direction
				{
					if (m_preCoordinateIndexes[m_endIndex] == m_startIndex){
						_pathDirections.push_back(DIRECTION_GO_STRAIGHT);
					}else{
						if (IsIntersection(m_preCoordinateIndexes[m_endIndex])){
							//calculate direction
							double angle = CCoordinate::GetAngle(
								m_coordinates[m_preCoordinateIndexes[m_preCoordinateIndexes[m_endIndex]]],
								m_coordinates[m_preCoordinateIndexes[m_endIndex]],
								m_coordinates[m_endIndex]);

							float turnThreshold	= 0.00884;	//5 Degrees
							if (angle > turnThreshold)	//Turn left
								_pathDirections.push_back(DIRECTION_TURN_LEFT);
							else if (angle < -turnThreshold)	//Turn left
								_pathDirections.push_back(DIRECTION_TURN_RIGHT);
							else
								_pathDirections.push_back(DIRECTION_GO_STRAIGHT);

							angle = angle + 8;
						}else{
							_pathDirections.push_back(DIRECTION_GO_STRAIGHT);
						}
					}
				}

				m_endIndex = m_preCoordinateIndexes[m_endIndex];
			}else{	//The path is disconnect
//				shortestPath.clear();
//				_pathDirections.clear();
//				_pathDistance = 0;
				break;
			}

			//Check is the path connected or not
			pathCounter++;
			if (pathCounter > m_coordinatesCount){      //The path is disconnect
//				shortestPath.clear();
//				_pathDirections.clear();
//				_pathDistance = 0;
				break;
			}
		}

		std::reverse(shortestPath.begin(),		shortestPath.end());
		std::reverse(_pathDirections.begin(),	_pathDirections.end());
		//! Free unnecessary memory

	}

    return shortestPath;
}

