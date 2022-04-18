#ifndef Rectangle_H__
#define Rectangle_H__

#include <stdint.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
template <typename RECT_TYPE>struct RectangleTemplate
{
    RectangleTemplate(){};
    RectangleTemplate(const RectangleTemplate<RECT_TYPE>& pRect)
    {
        x = pRect.x;
        y = pRect.y;
        width = pRect.width;
        height = pRect.height;
    }

    RectangleTemplate(RECT_TYPE pX,RECT_TYPE pY,RECT_TYPE pWidth,RECT_TYPE pHeight)
    {
        x = pX;
        y = pY;
        width = pWidth;
        height = pHeight;
    }

    const RectangleTemplate<RECT_TYPE>& operator = (const RectangleTemplate<RECT_TYPE>& pRect)
    {
        x = pRect.x;
        y = pRect.y;
        width = pRect.width;
        height = pRect.height;
        return pRect;
    }

    bool ContainsPoint(RECT_TYPE pX,RECT_TYPE pY)const
    {
        return pX >= x && pX <= x + width &&
               pY >= y && pY <= y + height;
    }

    void GetQuad(int16_t pQuad[8])const
    {
        pQuad[0] = (int16_t)x;              pQuad[1] = (int16_t)y;
        pQuad[2] = (int16_t)x + width;      pQuad[3] = (int16_t)y;
        pQuad[4] = (int16_t)x + width;      pQuad[5] = (int16_t)y + height;
        pQuad[6] = (int16_t)x;              pQuad[7] = (int16_t)y + height;
    };

    RectangleTemplate<RECT_TYPE> GetShrunk(RECT_TYPE pX,RECT_TYPE pY)const
    {
        return RectangleTemplate<RECT_TYPE>(x + pX,y + pY,width - pX - pX,height - pY - pY);
    }

    void Shrink(RECT_TYPE pX,RECT_TYPE pY)
    {
        x += pX;
        y += pY;
        width -= pX + pX;
        height-= pY + pY;
    }

    RECT_TYPE GetCenterX()const
    {
        return x + (width / 2);
    }

    RECT_TYPE GetCenterY()const
    {
        return y + (height / 2);
    }

    RECT_TYPE x = 0;
    RECT_TYPE y = 0;
    RECT_TYPE width = 0;
    RECT_TYPE height = 0;
};

typedef RectangleTemplate<int32_t> Rectangle;
typedef RectangleTemplate<float> RectangleF;

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{

#endif //#ifndef Rectangle_H__