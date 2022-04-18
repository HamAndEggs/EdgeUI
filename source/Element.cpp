
#include "Element.h"
#include "LayoutGrid.h"


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

Element* Element::Create(float pWidth,float pHeight)
{
    Element* root = new Element();
    root->SetSize(pWidth,pHeight);
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

RectangleF Element::GetContentRectangle()
{
    const RectangleF parentContentRect = GetParentRectangle();

    RectangleF contentRect;

    contentRect.x = parentContentRect.x + (parentContentRect.width * mPos.x);
    contentRect.y = parentContentRect.y + (parentContentRect.height * mPos.y);
    contentRect.width = parentContentRect.width * mSize.x;
    contentRect.height = parentContentRect.height * mSize.y;

    return contentRect;
}

RectangleF Element::GetParentRectangle()
{
    if( mParent )
    {
        return mParent->GetContentRectangle();
    }
    return Graphics::Get()->GetDisplayRectF();
}


void Element::SetPos(const PointF& pPos)
{
    mPos = pPos;
}

void Element::SetSize(const PointF& pSize)
{
    mSize = pSize;
}

void Element::Attach(Element* pElement)
{
    VERBOSE_MESSAGE("Attaching " + pElement->GetID() + " to " + mID);
    mChildren.push_back(pElement);
    pElement->mParent = this;
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

    RectangleF contentRect = GetContentRectangle();

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
    const RectangleF contentRect = GetContentRectangle();    
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
