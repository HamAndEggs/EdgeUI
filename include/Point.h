#ifndef Point_H__
#define Point_H__

#include <stdint.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
template <typename POINT_TYPE>struct PointTemplate
{
    PointTemplate() = default;
    PointTemplate(const PointTemplate<POINT_TYPE>& pPoint)
    {
        x = pPoint.x;
        y = pPoint.y;
    }

    PointTemplate(POINT_TYPE pX,POINT_TYPE pY)
    {
        x = pX;
        y = pY;
    }

    const PointTemplate<POINT_TYPE>& operator = (const PointTemplate<POINT_TYPE>& pPoint)
    {
        x = pPoint.x;
        y = pPoint.y;
        return pPoint;
    }

    POINT_TYPE x,y;
};

typedef PointTemplate<int32_t> Point;
typedef PointTemplate<float> PointF;

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{

#endif //#ifndef Point_H__