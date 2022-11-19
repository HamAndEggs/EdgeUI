#ifndef Slider_H__
#define Slider_H__

#include "Element.h"

#include <string>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class Slider;
typedef Slider* SliderPtr;

class Slider : public Element
{
    using BaseClass = Element;
public:
    Slider(int min,int max,int step):
        mMin(min),mMax(max),mStep(step),
        mValue(min + ((max - min) / 2))
    {
        assert(min < max);
        GetStyle().mBackground = COLOUR_BLUE;
        GetStyle().mForeground = COLOUR_DARK_GREY;
        GetStyle().mAlignment = ALIGN_LEFT_TOP;

        mKnobStyle.mForeground = COLOUR_LIGHT_GREY;
        mKnobStyle.mBorder = COLOUR_BLACK;
        mKnobStyle.mThickness = 2;
        mKnobStyle.mRadius = 0.3f;
    }

    virtual ~Slider(){};

    static std::string ClassID(){return "eui::Slider";}
    virtual std::string GetClassID()const{return ClassID();}

    virtual bool OnUpdate(const Rectangle& pContentRect)
    {
        BaseClass::OnUpdate(pContentRect);
        mSliderRect = pContentRect.GetScaled(0.95f,0.1f);
        return false;
    }

    virtual bool OnDraw(Graphics* pGraphics,const Rectangle& pContentRect)
    {
        // Draw background and anything else the parent can do for us.
        BaseClass::OnDraw(pGraphics,pContentRect);

        // Draw the slider track
        DrawRectangle(pGraphics,mSliderRect,GetStyle(),true);

        // Draw the slider knob.
        const float knobW = pContentRect.GetWidth()*0.1f;
        const float knobMovementW = pContentRect.GetWidth() - knobW;
        float XPercent = (float)(mValue - mMin) / (float)(mMax - mMin);

        Rectangle knobRect = 
        {
            knobMovementW * XPercent,
            pContentRect.top,
            (knobMovementW * XPercent)+knobW,
            pContentRect.bottom
        };
        DrawRectangle(pGraphics,knobRect,mKnobStyle,true);

        return false;
    }

    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched,bool pMoving)
    {
        if( pTouched && GetWidth() > 0 )
        {
            // Slider rect is smaller than the control rect, so clamp localX.
            pLocalX = mSliderRect.ClampX(pLocalX);
            float XPercent = pLocalX / mSliderRect.GetWidth();

            mValue = (int)(mMin + ((mMax - mMin) * XPercent));

            return true;
        }
        return false;
    }

private:
    int mMin,mMax,mStep;
    int mValue;

    Style mKnobStyle;
    Rectangle mSliderRect;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif