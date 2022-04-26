
#include "Element.h"
#include "LayoutGrid.h"


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

Element* Element::Create(float pWidth,float pHeight)
{
    Element* root = new Element();
    root->SetRightBottom(pWidth,pHeight);
    return root;
}

Element::Element()
{
    SET_DEFAULT_ID();
}

Element::~Element()
{
    VERBOSE_MESSAGE("Freeing Element: " + mID);
    for( auto e : mChildren )
    {
        delete e;
    }
}

Rectangle Element::GetContentRectangle()
{
    const Rectangle parentContentRect = GetParentRectangle();

    const Rectangle contentRect(parentContentRect.GetX(mRect.left),parentContentRect.GetY(mRect.top),
                                parentContentRect.GetX(mRect.right),parentContentRect.GetY(mRect.bottom));

//    return contentRect;
    return Rectangle(contentRect.GetX(mPadding.left),contentRect.GetY(mPadding.top),contentRect.GetX(mPadding.right),contentRect.GetY(mPadding.bottom));
}

Rectangle Element::GetParentRectangle()
{
    if( mParent )
    {
        return mParent->GetContentRectangle();
    }
    return Graphics::Get()->GetDisplayRect();
}


void Element::SetLeftTop(const Point& pPos)
{
    mRect.left = pPos.x;
    mRect.top = pPos.y;
}

void Element::SetRightBottom(const Point& pPos)
{
    mRect.right = pPos.x;
    mRect.bottom = pPos.y;

}

void Element::SetPadding(float pPadding)
{
    mPadding.left = pPadding;
    mPadding.right = 1.0f - pPadding;
    mPadding.top = pPadding;
    mPadding.bottom = 1.0f - pPadding;
}


void Element::Attach(Element* pElement)
{
    VERBOSE_MESSAGE("Attaching " + pElement->GetID() + " to " + mID);
    mChildren.push_back(pElement);
    pElement->mParent = this;
}

void Element::Remove(Element* pElement)
{
    pElement->mParent = nullptr;
    mChildren.remove(pElement);
}

void Element::Update()
{
    if( mVisible == false )
        return;

    for( auto e : mChildren )
    {
        e->Update();
    }
}

void Element::Draw()
{
    if( mVisible == false )
        return;

    Rectangle contentRect = GetContentRectangle();

    if( mStyle.mBackground != COLOUR_NONE )
    {
        Graphics::Get()->DrawRectangle(contentRect,mStyle);
    }

    for( auto& e : mChildren )
    {
        e->Draw();
    }
}

bool Element::TouchEvent(float pX,float pY,bool pTouched)
{
    const Rectangle contentRect = GetContentRectangle();    
    if( contentRect.ContainsPoint(pX,pY) )
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{
