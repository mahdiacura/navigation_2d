#include "ccoordinate.h"

CCoordinate::CCoordinate(double _x, double _y, double _z){
    m_isInitialized = true;
    m_x             = _x;
    m_y             = _y;
    m_z             = _z;
}

CCoordinate::CCoordinate(){
    m_isInitialized = false;
    m_x             = 0;
    m_y             = 0;
    m_z             = 0;
}

void CCoordinate::SetCoordinate(double _x, double _y, double _z){
    m_isInitialized = true;
    m_x             = _x;
    m_y             = _y;
    m_z             = _z;
}

CCoordinate::~CCoordinate(){

}
