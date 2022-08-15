#ifndef CHECKBOX_H__
#define CHECKBOX_H__

#include "Element.h"

#include <string>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class Checkbox;
typedef Checkbox* CheckboxPtr;

class Checkbox : public Element
{
public:
    Checkbox(std::string pLabel = "",int pFont = 0,eui::Colour pColour = eui::COLOUR_LIGHT_GREY,float pBoarderSize = 5.0f,float pRadius = 0.1f)
    {
        eui::Style s;

        s.mBackground = pColour;
        s.mBoarderStyle = eui::Style::BS_RAISED;
        s.mBorder = eui::COLOUR_WHITE;
        s.mThickness = pBoarderSize;
        s.mRadius = pRadius;

        mTickStyle.mThickness = 3;

        SetStyle(s);
        SetText(pLabel);
        SetFont(pFont);
    }

    virtual ~Checkbox(){};

    static std::string ClassID(){return "eui::Checkbox";}
    virtual std::string GetClassID()const{return ClassID();}

    virtual bool OnDraw(Graphics* pGraphics,const Rectangle& pContentRect)
    {
        Rectangle r = pContentRect;
        r.MakeBox();

        pGraphics->DrawRectangle(r,GetStyle());

        if( mChecked )
        {
            pGraphics->DrawTick(r,mTickStyle);
        }

        r.left += r.GetWidth() * 1.1f;
        pGraphics->FontPrint(GetFont(),r,ALIGN_LEFT_CENTER,GetStyle().mForeground,GetText());

        return true;
    }

    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched)
    {
        if( pTouched )
        {
            mChecked = !mChecked;
            if( mBoundBool )
            {
                *mBoundBool = mChecked;
            }

            if( mOnChecked )
            {
                mOnChecked(this,mChecked);
            }
        }
        return true;
    }

    typedef std::function<void (CheckboxPtr pElement,bool pChecked)> OnChecked;
    ElementPtr SetOnChecked(OnChecked pOnChecked){mOnChecked = pOnChecked;return this;}

    ElementPtr Bind(bool *pBoundBool){mBoundBool = pBoundBool;return this;}

private:
    bool mChecked = false;
    bool *mBoundBool = nullptr;
    eui::Style mTickStyle;
    OnChecked mOnChecked;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif