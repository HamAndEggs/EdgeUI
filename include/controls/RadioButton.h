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
    RadioButton(const tinyjson::JsonValue &root,ResouceMap* pLoadResources):Element(root,pLoadResources)
    {
        if( root.HasValue("style") == false )
        {
            SetStyle(eui::COLOUR_LIGHT_GREY,eui::BS_RAISED,5.0f,0.1f,0);
        }
        mTickStyle.mThickness = 3;
    }

    RadioButton(std::string pLabel,uint32_t pID,int pFont = 0,eui::Colour pColour = eui::COLOUR_LIGHT_GREY,float pBoarderSize = 5.0f,float pRadius = 0.1f)
    {
        SetStyle(pColour,eui::BS_RAISED,pBoarderSize,pRadius,pFont);


        SetText(pLabel);
        SetUserValue(pID);
    }

    virtual ~RadioButton(){}

    static std::string ClassID(){return "eui::radio-button";}
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

    RadioButtonPtr SetChecked(bool pChecked){mChecked = pChecked; return this;}

private:
    bool mChecked = false;

    eui::Style mTickStyle;

};

class RadioButtonGroup : public Element
{
public:

    RadioButtonGroup(){SetAutoGrid();}
    virtual ~RadioButtonGroup(){}

    static std::string ClassID(){return "eui::radio-button-group";}
    virtual std::string GetClassID()const{return ClassID();}

    RadioButtonGroup(const tinyjson::JsonValue &root,ResouceMap* pLoadResources):Element(root,pLoadResources)
    {
    }

    RadioButtonGroupPtr Add(const std::string &pLabel,uint32_t pID,int pFont = 0)
    {
        RadioButtonPtr but = new RadioButton(pLabel,pID,pFont);
        but->SetPos(0,GetNumChildren());
        but->SetOnTouched([this](ElementPtr pElement,float pLocalX,float pLocalY,bool pTouched,bool pMoving)
        {
            if( pTouched && pMoving == false && pElement->GetClassID() == RadioButton::ClassID() )
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
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif