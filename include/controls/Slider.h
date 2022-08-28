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
public:
    Slider(int min,int max,int step):
        mMin(min),mMax(max),mStep(step)
    {
        eui::Style s;

        s.mBackground = eui::COLOUR_BLACK;
        s.mRadius = 0.1f;
    
        SetStyle(s);
        SetPadding(0.04f);
    }

    virtual ~Slider(){};

    static std::string ClassID(){return "eui::Slider";}
    virtual std::string GetClassID()const{return ClassID();}

    virtual bool OnDraw(Graphics* pGraphics,const Rectangle& pContentRect)
    {
        return false;
    }


private:
    int mMin,mMax,mStep;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif