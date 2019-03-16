#ifndef CRECTANGLE_H
#define CRECTANGLE_H

#include <cstdint>

struct SBox{
    //X
    double m_left;
    double m_right;
    //Y
    double m_top;
    double m_bottom;
    //Z
    double m_back;
    double m_front;

    SBox();
};

#endif
