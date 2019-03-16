#include "ccoordinate.h"

// This function converts decimal degrees to radians
double_t DegreeToRadian(double_t _degree)
{
	return (_degree * pi / 180);
};

//  This function converts radians to decimal degrees
double_t RadianToDegree(double_t _radian)
{
	return (_radian * 180 / pi);
};

CCoordinate::CCoordinate (double _x, double _y, double _z)
{
    m_isInitialized = true;
    m_x             = _x;
    m_y             = _y;
    m_z             = _z;
}

CCoordinate::CCoordinate ()
{
    m_isInitialized = false;
    m_x             = 0;
    m_y             = 0;
    m_z             = 0;
}

void CCoordinate::SetCoordinate (double _x, double _y, double _z)
{
    m_isInitialized = true;
    m_x             = _x;
    m_y             = _y;
    m_z             = _z;
}

CCoordinate::~CCoordinate ()
{

}

bool CCoordinate::operator== (const CCoordinate & _coordinate)
{
    return (m_x == _coordinate.m_x &&
            m_y == _coordinate.m_y &&
            m_z == _coordinate.m_z);
}

void CCoordinate::operator= (const CCoordinate & _coordinate)
{
    m_x = _coordinate.m_x;
    m_y = _coordinate.m_y;
    m_z = _coordinate.m_z;
}

double CCoordinate::GetAngle(CCoordinate & _coordinate1,
							 CCoordinate & _coordinate2,
							 CCoordinate & _coordinate3)
{
	return	GetAngle(
				CCoordinate(_coordinate2.m_x - _coordinate1.m_x,
							_coordinate2.m_y - _coordinate1.m_y,
							_coordinate2.m_z - _coordinate1.m_z),
				CCoordinate(_coordinate3.m_x - _coordinate2.m_x,
							_coordinate3.m_y - _coordinate2.m_y,
							_coordinate3.m_z - _coordinate2.m_z));
}

//Returns the angle between two vectors in radian
double CCoordinate::GetAngle(CCoordinate _vector1, CCoordinate _vector2)
{
	double angle = std::atan2(_vector2.m_y, _vector2.m_x) -
		std::atan2(_vector1.m_y, _vector1.m_x);

	if (angle < 0)
		angle += 2 * pi;

	if (angle > pi)
		angle -= 2 * pi;
	else if (angle <= -pi)
		angle += 2 * pi;

	return angle;
}
