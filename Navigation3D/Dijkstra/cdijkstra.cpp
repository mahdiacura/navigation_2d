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
	//m_preCoordinateIndexes[m_endIndex]      = m_startIndex;
}

void CDijkstra::SetDistance(CWay & _way, int32_t & _sourceCoordinateIndex, int32_t & _destinationCoordinateIndex){//! hotspot
	if (_sourceCoordinateIndex == _destinationCoordinateIndex)return;//Ignore very close coordinates
	m_distances[_sourceCoordinateIndex][_destinationCoordinateIndex] = _way.m_distance;//! remove ways and get in loadshapefile()
	if (!_way.m_isOneWay)
		m_distances[_destinationCoordinateIndex][_sourceCoordinateIndex] = _way.m_distance;
	//m_preCoordinateIndexes[_destinationCoordinateIndex] = _sourceCoordinateIndex;
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
		startIndex	= AddOrMergeCoordinate(ways->m_startCoordinate);
		endIndex	= AddOrMergeCoordinate(ways->m_endCoordinate);
		SetDistance(*ways, startIndex, endIndex);

		if (startIndex == endIndex){
			startIndex = endIndex + 0;
			startIndex = endIndex + 0;
			startIndex = endIndex + 0;

		}
    }

	//! Resize the buffer to non-repetitive coordinates
//	m_distances.resize(m_coordinatesCount, std::vector<double>(m_coordinatesCount));
//	m_preCoordinateIndexes.resize(m_coordinatesCount);
}

int32_t CDijkstra::GetIndex(CCoordinate & _coordinate){
    m_coordinateIterator = std::find(m_coordinates.begin(), m_coordinates.end(), _coordinate);
	if (m_coordinates.end() == m_coordinateIterator)return NONE_INDEX;

    return (m_coordinateIterator - m_coordinates.begin());
}

int32_t CDijkstra::GetOutputsNumber(int32_t _coordinateIndex){
	if (NONE_INDEX == _coordinateIndex)return 0;

	int32_t outputsCounter = 0;
	for (int32_t index = 0; index < m_coordinatesCount; index++){
		if (_coordinateIndex == index)continue;
		if (INFINITE_DISTANCE != m_distances[_coordinateIndex][index])
			outputsCounter++;
	}

	return outputsCounter;
}

int32_t CDijkstra::PopupNearestVertext(
		std::vector<int32_t> & _priorityQueue,
		std::vector<bool> & _isVisited){
	int32_t length			= _priorityQueue.size();
	int32_t nearestIndex	= NONE_INDEX;
	int32_t vertexIndex		= NONE_INDEX;
	int32_t index			= NONE_INDEX;
	double_t minDistance	= INFINITE_DISTANCE;

	for (index = 0; index < length; index++){
		vertexIndex = _priorityQueue[index];
		if (false == _isVisited[vertexIndex] &&
				m_distances[vertexIndex][vertexIndex] < minDistance){
			minDistance		= m_distances[vertexIndex][vertexIndex];
			nearestIndex	= index;
		}
	}

	if (NONE_INDEX == nearestIndex){
		index = index + 0;
		return NONE_INDEX;
	}

	index = nearestIndex;
	nearestIndex	= _priorityQueue[nearestIndex];

	_priorityQueue.erase(_priorityQueue.begin() + index);
	_isVisited[nearestIndex] = true;
	return nearestIndex;
}

int32_t CDijkstra::GetInputsNumber(int32_t _coordinateIndex){
	if (NONE_INDEX == _coordinateIndex)return 0;

	int32_t inputsCounter = 0;
	for (int32_t index = 0; index < m_coordinatesCount; index++){
		if (_coordinateIndex == index)continue;
		if (INFINITE_DISTANCE != m_distances[index][_coordinateIndex])
			inputsCounter++;
	}

	return inputsCounter;
}

bool CDijkstra::IsIntersection(int32_t _coordinateIndex){
	if (GetOutputsNumber(_coordinateIndex) > 1)return true;
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
	double minDistance          = INFINITE_DISTANCE;
	int32_t index               = NONE_INDEX;
	int32_t minDistanceIndex    = NONE_INDEX;
	int32_t currentIndex        = NONE_INDEX;
	std::vector<int32_t> priorityQueue;

	GenerateDistancesMatrix();

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

	std::vector<bool> isVisited(m_coordinates.size(), false);
	int32_t endIndex = m_endIndex;
    //The djacent coordinates to the current coordinate have the non-infinite value on distances matrix

	//Copy entire vertices to priorityQueue
	for (index = 0; index < m_coordinatesCount; index++)
		priorityQueue.push_back(index);

	m_distances[m_startIndex][m_startIndex] = 0;

	double currentWeight	= INFINITE_DISTANCE;
	double neighborDistance	= INFINITE_DISTANCE;
	double neighborWeight	= INFINITE_DISTANCE;

	while (!priorityQueue.empty()){
		currentIndex = PopupNearestVertext(priorityQueue, isVisited);
		if (NONE_INDEX == currentIndex)break;

		//Find the nearest adjacent coordinate to current coordinate
		minDistance         = INFINITE_DISTANCE;
		minDistanceIndex    = NONE_INDEX;
		for (index = 0; index < m_coordinatesCount; index++){
			if (!IsConnected(currentIndex, index)/*true == isVisited[index] ||*/ /*index == m_startIndex*/)continue;	//! is extra : index == m_startIndex

			//Initialize and update the distance from source to current coordinate
			currentWeight		= m_distances[currentIndex][currentIndex];
			neighborDistance	= m_distances[currentIndex][index];
			neighborWeight		= m_distances[index][index];

			//Relax
			if (neighborWeight > currentWeight + neighborDistance ||
					index == m_endIndex){
				m_distances[index][index]       = currentWeight + neighborDistance;
				m_preCoordinateIndexes[index]   = currentIndex;

				if (index == m_endIndex){
					priorityQueue.clear();
					break;
				}
			}
		}
	}

	//Make the shortest path
	{
		_pathDirections.clear();
		_pathDirections.push_back(DIRECTION_GO_STRAIGHT);	//Add the end direction

		int32_t pathCounter = 0;
		_pathDistance        = 0;
		shortestPath.clear();
		endIndex = m_endIndex;
		shortestPath.push_back(m_coordinates[endIndex]);
		for (;endIndex != m_startIndex;){
			if (INFINITE_DISTANCE != m_preCoordinateIndexes[endIndex]){
				_pathDistance += GetDistanceOnEarth(
					m_coordinates[endIndex],
					m_coordinates[m_preCoordinateIndexes[endIndex]]);
				shortestPath.push_back(m_coordinates[m_preCoordinateIndexes[endIndex]]);

				//Calculate direction
				{
					if (m_preCoordinateIndexes[endIndex] == m_startIndex){
						_pathDirections.push_back(DIRECTION_GO_STRAIGHT);
					}else{
						if (IsIntersection(m_preCoordinateIndexes[endIndex])){
							//calculate direction
							double angle = CCoordinate::GetAngle(
								m_coordinates[m_preCoordinateIndexes[m_preCoordinateIndexes[endIndex]]],
								m_coordinates[m_preCoordinateIndexes[endIndex]],
								m_coordinates[endIndex]);

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

				if (m_preCoordinateIndexes[endIndex] == m_startIndex){
					endIndex = endIndex + 0;
					break;
				}
				endIndex = m_preCoordinateIndexes[endIndex];
			}else{	//The path is disconnect
				//Remove current dead end and route again
				//currentIndex = RemoveUnsuccessfullDeadEnd(shortestPath, endIndex);
				//currentIndex = m_startIndex;
				//endIndex = endIndex;
				//break;
//				removeCounter++;
//				if (removeCounter == 2){
//					removeCounter = removeCounter + 0;
//					break;
//				}

				shortestPath.clear();
				_pathDirections.clear();
				_pathDistance = 0;

//				//Initialize the unvisited vector to index
//				{
//					for (index = 0; index < m_coordinatesCount; index++)
//                        isVisited.push_back(index);

//					//Start Coordinate Initialization
//					m_preCoordinateIndexes[m_startIndex] = m_startIndex;
//                    isVisited.erase(isVisited.begin() + m_startIndex);
//					m_distances[m_startIndex][m_startIndex] = 0;
//				}

                //goto routing;
				break;
			}

			//Check is the path connected or not
			pathCounter++;
			if (pathCounter > m_coordinatesCount){      //The path is disconnect
				int x = 0 + 5;
				x = 3;
				shortestPath.clear();
				_pathDirections.clear();
				_pathDistance = 0;
				break;
			}
		}

		std::reverse(shortestPath.begin(),		shortestPath.end());
		std::reverse(_pathDirections.begin(),	_pathDirections.end());
		//! Free unnecessary memory

	}

    return shortestPath;
}

int32_t CDijkstra::RemoveUnsuccessfullDeadEnd(
	std::vector<CCoordinate> & _deadEndPath,	//This vector is reverse
	int32_t _lastIndex)
{
	int32_t preLastIndex = -1;
	preLastIndex = GetIndex(_deadEndPath[(_deadEndPath.size() - 1) - 1]);

	while ((2 > GetOutputsNumber(_lastIndex)) ||
		   (2 > GetInputsNumber(_lastIndex) &&
				INFINITE_DISTANCE != m_distances[preLastIndex][_lastIndex])){
		//! check. we have to remove the .back() or not?
		_deadEndPath.erase(_deadEndPath.end());//Remove end coorindate of dead end

		preLastIndex = GetIndex(_deadEndPath.back());
		m_distances[_lastIndex][_lastIndex]		= INFINITE_DISTANCE;

		if (NONE_INDEX == preLastIndex)return _lastIndex;
		m_distances[preLastIndex][preLastIndex]	= INFINITE_DISTANCE;

		m_distances[_lastIndex][preLastIndex]	= INFINITE_DISTANCE;
		m_distances[preLastIndex][_lastIndex]	= INFINITE_DISTANCE;

		m_preCoordinateIndexes[preLastIndex]	= NONE_INDEX;

		_lastIndex = preLastIndex;//Decrease the last index
		preLastIndex = GetIndex(_deadEndPath[(_deadEndPath.size() - 1) - 1]);
	}

	//! Remove pre of this dead end
	{
		m_distances[_lastIndex][preLastIndex]	= INFINITE_DISTANCE;
		m_distances[preLastIndex][_lastIndex]	= INFINITE_DISTANCE;

		m_distances[_lastIndex][_lastIndex]		= INFINITE_DISTANCE;
		m_distances[preLastIndex][preLastIndex]	= INFINITE_DISTANCE;
	}

	int x = 0;
	x = 3;
	return _lastIndex;//Then Reroute again
}

