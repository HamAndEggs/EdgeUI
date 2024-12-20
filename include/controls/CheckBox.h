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
    Checkbox(const tinyjson::JsonValue &root,ResouceMap* pLoadResources):Element(root,pLoadResources)
    {
        if( root.HasValue("style") == false )
        {
            SetStyle(eui::COLOUR_LIGHT_GREY,eui::BS_RAISED,5.0f,0.1f,0);
        }
        mTickStyle.mThickness = 3;
    }

    Checkbox(std::string pLabel = "",int pFont = 0,eui::Colour pColour = eui::COLOUR_LIGHT_GREY,float pBoarderSize = 5.0f,float pRadius = 0.1f)
    {
        SetStyle(pColour,eui::BS_RAISED,pBoarderSize,pRadius,pFont);

        mTickStyle.mThickness = 3;

        SetText(pLabel);
    }

    virtual ~Checkbox(){};

    static std::string ClassID(){return "eui::check-box";}
    virtual std::string GetClassID()const{return ClassID();}

    virtual bool OnDraw(Graphics* pGraphics,const Rectangle& pContentRect)
    {
        Rectangle r = pContentRect;
        r.MakeBox();

        DrawRectangle(pGraphics,r,GetStyle());

        if( mChecked )
        {
            pGraphics->DrawTick(r,mTickStyle.mForeground,mTickStyle.mThickness);
        }

        r.left += r.GetWidth() * 1.1f;
        pGraphics->FontPrint(GetFont(),r,ALIGN_LEFT_CENTER,GetStyle().mForeground,GetText());

        return true;
    }

    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched,bool pMoving)
    {
        if( pTouched && pMoving == false )
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