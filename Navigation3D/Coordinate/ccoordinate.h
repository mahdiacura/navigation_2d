#ifndef CCOORDINATE_H
#define CCOORDINATE_H

#include <cstdint>
#include <cmath>

double_t const pi = std::acos(-1);
#define EARTH_RADIUS_IN_KM 6371.0

double_t DegreeToRadian(double_t _degree);
double_t RadianToDegree(double_t _radian);

class CCoordinate{
public:
    double m_x;
    double m_y;
    double m_z;

	static double GetAngle	(CCoordinate & _coordinate1, CCoordinate & _coordinate2, CCoordinate & _coordinate3);
	static double GetAngle	(CCoordinate _vector1, CCoordinate _vector2);
	bool operator==			(const CCoordinate & _coordinate);
	void operator=			(const CCoordinate & _coordinate);
	bool IsInitialized		();
	void SetCoordinate		(double _x, double _y, double _z);
	CCoordinate				(double _x, double _y, double _z);
	CCoordinate				();
	~CCoordinate		    ();
private:
    bool m_isInitialized;
};

#endif
