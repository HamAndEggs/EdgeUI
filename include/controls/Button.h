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

    Button(const tinyjson::JsonValue &root,ResouceMap* pLoadResources):Element(root,pLoadResources)
    {
        if( root.HasValue("style") == false )
        {
            SetStyle(eui::COLOUR_LIGHT_GREY,eui::BS_RAISED,5.0f,0.1f,0);
        }
    }

    Button( std::string pLabel,
            uint32_t pFont,
            eui::Colour pColour = eui::COLOUR_LIGHT_GREY,
            float pBoarderSize = 5.0f,
            float pRadius = 0.1f,
            OnPressed pOnPressed = nullptr,
            OnReleased pOnReleased = nullptr):
            mOnPressed(pOnPressed),
            mOnReleased(pOnReleased)
    {
        SetStyle(pColour,eui::BS_RAISED,pBoarderSize,pRadius,pFont);
        SetText(pLabel);
    }

    Button( std::string pLabel,uint32_t pFont,OnPressed pOnPressed):
            mOnPressed(pOnPressed),
            mOnReleased(nullptr)
    {
        SetStyle(eui::COLOUR_LIGHT_GREY,eui::BS_RAISED,5.0f,0.1f,pFont);
        SetText(pLabel);
    }

    virtual ~Button(){};

    static std::string ClassID(){return "eui::button";}
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
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif