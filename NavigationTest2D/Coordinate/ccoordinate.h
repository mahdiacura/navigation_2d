#ifndef CCOORDINATE_H
#define CCOORDINATE_H

class CCoordinate{
public:
    double m_x;
    double m_y;
    double m_z;

    bool operator==     (const CCoordinate & _coordinate);
    void operator=      (const CCoordinate & _coordinate);
    bool IsInitialized  ();
    void SetCoordinate  (double _x, double _y, double _z);
    CCoordinate         (double _x, double _y, double _z);
    CCoordinate         ();
    ~CCoordinate        ();
private:
    bool m_isInitialized;
};

#endif
