#ifndef RadioButton_H__
#define RadioButton_H__

#include "Element.h"

#include <string>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
class RadioButton;
class RadioButtonGroup;

typedef RadioButton* RadioButtonPtr;
typedef RadioButtonGroup* RadioButtonGroupPtr;

class RadioButton : public Element
{
public:
    RadioButton(std::string pLabel,uint32_t pID,int pFont = 0,eui::Colour pColour = eui::COLOUR_LIGHT_GREY,float pBoarderSize = 5.0f,float pRadius = 0.1f)
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
        SetUserValue(pID);
    }

    virtual ~RadioButton(){}

    static std::string ClassID(){return "eui::RadioButton";}
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

    RadioButtonPtr SetChecked(bool pChecked){mChecked = pChecked; return this;}

private:
    bool mChecked = false;

    eui::Style mTickStyle;

};

class RadioButtonGroup : public Element
{
public:

    RadioButtonGroup(){}
    virtual ~RadioButtonGroup(){}

    static std::string ClassID(){return "eui::RadioButtonGroup";}
    virtual std::string GetClassID()const{return ClassID();}

    RadioButtonGroupPtr Add(const std::string &pLabel,uint32_t pID,int pFont = 0)
    {
        const uint32_t newRow = mNumButtons;
        mNumButtons++;
        SetGrid(1,mNumButtons);
        RadioButtonPtr but = new RadioButton(pLabel,pID,pFont);
        but->SetPos(0,newRow);
        but->SetOnTouched([this](ElementPtr pElement,float pLocalX,float pLocalY,bool pTouched)
        {
            if( pTouched && pElement->GetClassID() == RadioButton::ClassID() )
            {
                SetPressed(static_cast<RadioButtonPtr>(pElement)->GetUserValue());
                return true;
            }
            return false;
        });

        Attach(but);

        return this;
    }

    typedef std::function<void (RadioButtonPtr pElement,uint32_t pID)> OnPressed;
    RadioButtonGroupPtr SetOnPressed(OnPressed pCallback){mOnPressed = pCallback;return this;}

    RadioButtonGroupPtr Bind(uint32_t *pID){mBoundID = pID;return this;}

    RadioButtonGroupPtr SetPressed(uint32_t pID)
    {
        // Turn all the others off.
        for( auto& rb : GetChildren() )
        {
            if( rb->GetClassID() == RadioButton::ClassID() )
            {
                RadioButtonPtr p = static_cast<RadioButtonPtr>(rb);
                if( p->GetUserValue() == pID )
                {
                    p->SetChecked(true);
                    if( mOnPressed )
                    {
                        mOnPressed(p,pID);
                    }

                    if(mBoundID)
                    {
                        *mBoundID = pID;
                    }
                }
                else
                {
                    p->SetChecked(false);
                }
            }
        }
        return this;
    }

private:
    OnPressed mOnPressed = nullptr;
    uint32_t *mBoundID = nullptr;
    int mNumButtons = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif