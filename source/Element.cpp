
#include "Element.h"
#include "LayoutGrid.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

Element::Element(GraphicsPtr pGraphics) :
    mGraphics(pGraphics),
    mArea(0,0,ELEMENT_SIZE_USE_PARENT,ELEMENT_SIZE_USE_PARENT)
{

}

void Element::SetArea(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight)
{
    mArea.x = pX;
    mArea.y = pY;
    mArea.width = pWidth;
    mArea.height = pHeight;
    mContentDirty = true;
}

void Element::SetArea(const Rectangle& pArea)
{
    mArea = pArea;
    mContentDirty = true;
}

void Element::SetPos(int32_t pX,int32_t pY)
{
    mArea.x = pX;
    mArea.x = pY;
    mContentDirty = true;
}

void Element::SetPadding(int32_t pLeft,int32_t pTop,int32_t pRight,int32_t pBottom)
{
    mPadding.left = pLeft;
    mPadding.top = pTop;
    mPadding.right = pRight;
    mPadding.bottom = pBottom;
    mContentDirty = true;
}

void Element::SetLabel(const std::string& pLabel)
{
    mLabel = pLabel;
    mContentDirty = true;
}

void Element::Attach(ElementPtr& pElement)
{
    mChildren.push_back(pElement);
}

void Element::Update(const Rectangle& pParentContectRect)
{
    if( mVisible == false )
        return;

    if( mContentDirty )
    {
        RecomputeRectangles(pParentContectRect);
        mContentDirty = false;
    }

    for( auto& e : mChildren )
    {
        e->Update(GetContentRectangle());
    }
}

void Element::Draw()
{
    if( mVisible == false )
        return;

    if( mStyle.mBackground != COLOUR_NONE )
    {
        mGraphics->DrawRectangle(mComputed.contentRect,mStyle);
    }

    if( mLabel.size() > 0 )
    {
        mGraphics->DrawFont(mStyle,mComputed.contentRect.x,mComputed.contentRect.y,mLabel);
    }

    for( auto& e : mChildren )
    {
        e->Draw();
    }
}

bool Element::TouchEvent(int32_t pX,int32_t pY,bool pTouched)
{
    if( mArea.ContainsPoint(pX,pY) )
    {
        if( pTouched )
        {
            // Is it in our rect?
            if( mEvents.OnPressed )
            {
                if( mEvents.OnPressed(*this) )
                {
                    return true;
                }
            }
        }
        else
        {

        }
    }

    for( auto& e : mChildren )
    {
        if( e->TouchEvent(pX,pY,pTouched) )
        {
            return true;
        }
    }

    return false;
}

ElementPtr Element::AddElement(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight)
{
    eui::ElementPtr elem = std::make_shared<eui::Element>(mGraphics);
    elem->SetArea(pX,pY,pWidth,pHeight);
    Attach(elem);
    return elem;
}

ElementPtr Element::AddButton(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight,const std::string& pText,ElementEvent pOnPressed)
{
    eui::ElementPtr button = std::make_shared<eui::Element>(mGraphics);
    button->SetArea(pX,pY,pWidth,pHeight);
    button->SetLabel(pText);
    button->SetOnPressed(pOnPressed);
    button->SetForground(COLOUR_WHITE);
    button->SetBackground(COLOUR_LIGHT_GREY);
    button->SetBorder(1);
    button->SetRadius(25);
    Attach(button);
    return button;
}

ElementPtr Element::AddLabel(int32_t pX,int32_t pY,const std::string& pText)
{
    eui::ElementPtr label = std::make_shared<eui::Element>(mGraphics);
    label->SetPos(pX,pY);
    label->SetLabel(pText);
    label->SetForground(COLOUR_WHITE);
    label->SetBackground(COLOUR_BLACK);
    Attach(label);
    return label;
}

ElementPtr Element::AddGridlayout(uint32_t pColumns, uint32_t pRows)
{
    eui::ElementPtr layout = std::make_shared<eui::LayoutGrid>(mGraphics,pColumns,pRows);
    layout->SetPos(0,0);
    layout->SetForground(COLOUR_WHITE);
    Attach(layout);
    return layout;
}

void Element::RecomputeRectangles(const Rectangle& pParentContectRect)
{
    const int32_t width = mArea.width == ELEMENT_SIZE_USE_PARENT ? pParentContectRect.width : mArea.width;
    const int32_t height = mArea.height == ELEMENT_SIZE_USE_PARENT ? pParentContectRect.height : mArea.height;

    mComputed.contentRect.x = pParentContectRect.x + mArea.x + mPadding.left;
    mComputed.contentRect.y = pParentContectRect.y + mArea.y + mPadding.top;
    mComputed.contentRect.width = width - mPadding.left - mPadding.right;
    mComputed.contentRect.height = height - mPadding.top - mPadding.bottom;
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{
