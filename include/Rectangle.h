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
        *this = pRect;
    }

    Rectangle(float pLeft,float pTop,float pRight,float pBottom)
    {
        left = pLeft;
        top = pTop;
        right = pRight;
        bottom = pBottom;
    }

    const Rectangle& operator = (const Rectangle& pRect)
    {
        left = pRect.left;
        top = pRect.top;
        right = pRect.right;
        bottom = pRect.bottom;

        return pRect;
    }

    bool ContainsPoint(float pX,float pY)const
    {
        return pX >= left && pX <= right &&
               pY >= top && pY <= bottom;
    }

    void GetQuad(float *pQuad)const
    {
        pQuad[0] = left;       pQuad[1] = top;
        pQuad[2] = right;      pQuad[3] = top;
        pQuad[4] = right;      pQuad[5] = bottom;
        pQuad[6] = left;       pQuad[7] = bottom;
    };

    Rectangle GetShrunk(float pX,float pY)const
    {
        return Rectangle(left + pX,top + pY,right - pX,bottom - pY);
    }

    void Shrink(float pX,float pY)
    {
        left += pX;
        right -= pX;
        top += pY;
        bottom-= pY;
    }

    /**
     * @brief Get the Scaled rect, 1 == orginal size, 0 == zero point at the center of orginal.
     */
    Rectangle GetScaled(float pScale)const
    {
        const float scale = pScale*0.5f;// Make it work for our lerp.
        return Rectangle(
            GetX(0.5f - scale),
            GetY(0.5f - scale),
            GetX(0.5f + scale),
            GetY(0.5f + scale)
            );
    }

    /**
     * @brief Enlarges the rect so that the point is contained
     */
    void AddPoint(float pX,float pY)
    {
        left = std::min(left,pX);
        right = std::max(right,pX);
        top = std::min(top,pY);
        bottom = std::max(bottom,pY);

        assert(ContainsPoint(pX,pY));
    }

    float GetCenterX()const
    {
        return (left + right) / 2.0f;
    }

    float GetCenterY()const
    {
        return (top + bottom) / 2.0f;
    }

    float GetWidth()const
    {
        return right - left;
    }

    float GetHeight()const
    {
        return bottom - top;
    }

    float GetX(float pTween)const
    {
        return (left * (1.0f - pTween)) + (right * pTween);
    }

    float GetY(float pTween)const
    {
        return (top * (1.0f - pTween)) + (bottom * pTween);
    }


    float left = 0;
    float top = 0;
    float right = 0;
    float bottom = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{

#endif //#ifndef Rectangle_H__