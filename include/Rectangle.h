#ifndef Rectangle_H__
#define Rectangle_H__

#include <stdint.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

struct Rectangle
{
    Rectangle(){};
    Rectangle(const Rectangle& pRect)
    {
        x = pRect.x;
        y = pRect.y;
        width = pRect.width;
        height = pRect.height;
    }

    Rectangle(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight)
    {
        x = pX;
        y = pY;
        width = pWidth;
        height = pHeight;
    }

    const Rectangle& operator = (const Rectangle& pRect)
    {
        x = pRect.x;
        y = pRect.y;
        width = pRect.width;
        height = pRect.height;
        return *this;
    }

    bool ContainsPoint(int32_t pX,int32_t pY)
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

    Rectangle GetShrunk(int32_t pX,int32_t pY)const
    {
        return Rectangle(x + pX,y + pY,width - pX - pX,height - pY - pY);
    }

    void Shrink(int32_t pX,int32_t pY)
    {
        x += pX;
        y += pY;
        width -= pX + pX;
        height-= pY + pY;
    }

    int32_t GetCenterX()const
    {
        return x + (width / 2);
    }

    int32_t GetCenterY()const
    {
        return y + (height / 2);
    }

    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{

#endif //#ifndef Rectangle_H__