#ifndef BUTTON_H__
#define BUTTON_H__

#include "Element.h"

#include <string>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
class Button : public Element
{
public:
    Button(std::string pLabel,int pFont,eui::Colour pColour = eui::COLOUR_LIGHT_GREY,float pBoarderSize = 5.0f,float pRadius = 0.1f)
    {
        eui::Style s;

        s.mBackground = pColour;
        s.mBoarderStyle = eui::Style::BS_RAISED;
        s.mBorder = eui::COLOUR_WHITE;
        s.mBorderSize = pBoarderSize;
        s.mRadius = pRadius;

        SetStyle(s);
        SetText(pLabel);
        SetFont(pFont);
    }

    virtual ~Button(){};

    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched)
    {
        if( pTouched )
        {
            GetStyle().mBoarderStyle = eui::Style::BS_DEPRESSED;
        }
        else
        {
            GetStyle().mBoarderStyle = eui::Style::BS_RAISED;
        }
        return false;
    }

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif