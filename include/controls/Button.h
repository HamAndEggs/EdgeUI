#ifndef BUTTON_H__
#define BUTTON_H__

#include "Element.h"

#include <string>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
class Button : public Element
{
public:
    typedef std::function<void ()> OnPressed;
    typedef std::function<void ()> OnReleased;

    Button( std::string pLabel,
            int pFont,
            eui::Colour pColour = eui::COLOUR_LIGHT_GREY,
            float pBoarderSize = 5.0f,
            float pRadius = 0.1f,
            OnPressed pOnPressed = nullptr,
            OnReleased pOnReleased = nullptr):
            mOnPressed(pOnPressed),
            mOnReleased(pOnReleased)
    {
        ConstructStyle(pColour,pBoarderSize,pRadius);
        SetText(pLabel);
        SetFont(pFont);
    }

    Button( std::string pLabel,int pFont,OnPressed pOnPressed):
            mOnPressed(pOnPressed),
            mOnReleased(nullptr)
    {
        ConstructStyle(eui::COLOUR_LIGHT_GREY,5.0f,0.1f);
        SetText(pLabel);
        SetFont(pFont);
    }

    virtual ~Button(){};

    static std::string ClassID(){return "eui::Button";}
    virtual std::string GetClassID()const{return ClassID();}

    void SetOnPressed(OnPressed pOnPressed){mOnPressed=pOnPressed;}
    void SetOnReleased(OnReleased pOnReleased){mOnReleased=pOnReleased;}

private:
    OnPressed mOnPressed;
    OnReleased mOnReleased;

    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched,bool pMoving)
    {
        if( pTouched )
        {
            GetStyle().mBoarderStyle = eui::BS_DEPRESSED;
            if(mOnPressed)
            {
                mOnPressed();
            }
        }
        else
        {
            GetStyle().mBoarderStyle = eui::BS_RAISED;
            if(mOnReleased)
            {
                mOnReleased();
            }
        }

        return false;
    }

    void ConstructStyle(eui::Colour pColour,float pBoarderSize,float pRadius)
    {
        eui::Style s;
        s.mBackground = pColour;
        s.mBoarderStyle = eui::BS_RAISED;
        s.mBorder = eui::COLOUR_WHITE;
        s.mThickness = pBoarderSize;
        s.mRadius = pRadius;
        SetStyle(s);
    }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif