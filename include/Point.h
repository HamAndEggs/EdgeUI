#ifndef Point_H__
#define Point_H__

#include <stdint.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
struct Point
{
    Point() = default;
    Point(const Point& pPoint)
    {
        *this = pPoint;
    }

    Point(float pX,float pY)
    {
        x = pX;
        y = pY;
    }

    const Point& operator = (const Point& pPoint)
    {
        x = pPoint.x;
        y = pPoint.y;
        return pPoint;
    }

    float x,y;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{

#endif //#ifndef Point_H__