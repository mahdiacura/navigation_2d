#ifndef CDIJKSTRA_H
#define CDIJKSTRA_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cmath>

#include "../Coordinate/ccoordinate.h"
#include "../Coordinate/cbox.h"
#include "../Coordinate/cway.h"
#include "ShapeLib/shapefil.h"

#define INFINITE_DISTANCE	-1
#define NONE_INDEX			-1

double_t const pi = std::acos(-1);
#define EARTH_RADIUS_IN_KM 6371.0

double_t deg2rad(double_t _degree);
double_t rad2deg(double_t _radian);

class CDijkstra{
public:
	void LoadShapeFile(std::string _shapeFilePath);
    void AddWay(CWay _coordinate);
	std::vector<CCoordinate> FindShortestPath(
		CCoordinate _source,
		CCoordinate _destination,
		double & _pathDistance);
	int32_t FindNearestCoordinate(CCoordinate & _coordinate);
	double_t GetDistanceOnEarth(CCoordinate & _source, CCoordinate & _destination);

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
	CCoordinate m_centerOfMap;

    bool IsConnected(int32_t _startIndex, int32_t _endIndex);
    int32_t GetIndex(CCoordinate & _coordinate);
    void SetDistance(CWay & _way);
    void AddCoordinate(CCoordinate _coordinate);
	void GenerateDistancesMatrix();
    void FreeMemory();
};

#endif
