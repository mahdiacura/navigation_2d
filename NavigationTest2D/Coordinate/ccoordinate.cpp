#include "ccoordinate.h"

CCoordinate::CCoordinate(double _x, double _y, double _z){
    m_isInitialized = true;
    m_x             = _x;
    m_y             = _y;
    m_z             = _z;
}

void CCoordinate::SetCoordinate(double _x, double _y, double _z){
    m_isInitialized = true;
    m_x             = _x;
    m_y             = _y;
    m_z             = _z;
}

CCoordinate::~CCoordinate(){

}
