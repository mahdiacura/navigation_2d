#include "cshapefile.h"

CShapeFile::CShapeFile(){

}

CShapeFile::~CShapeFile(){

}

void CShapeFile::OpenFile(std::string _filePath){

}

void CShapeFile::CloseFile(){

}

CCoordinate CShapeFile::GetCoordinate(int32_t _coordinateIndex){
    return m_coordinates[_coordinateIndex];
}

