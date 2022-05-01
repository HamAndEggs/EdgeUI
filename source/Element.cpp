
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

Element* Element::Create(const Style& pStyle,float pWidth,float pHeight)
{
    Element* root = Create(pWidth,pHeight);
    root->SetStyle(pStyle);
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

Rectangle Element::GetContentRectangle()const
{
    const Rectangle parentContentRect = GetParentRectangle();

    const Rectangle contentRect(parentContentRect.GetX(mRect.left),parentContentRect.GetY(mRect.top),
                                parentContentRect.GetX(mRect.right),parentContentRect.GetY(mRect.bottom));

//    return contentRect;
    return Rectangle(contentRect.GetX(mPadding.left),contentRect.GetY(mPadding.top),contentRect.GetX(mPadding.right),contentRect.GetY(mPadding.bottom));
}

Rectangle Element::GetParentRectangle()const
{
    if( mParent )
    {
        return mParent->GetContentRectangle();
    }
    return Graphics::Get()->GetDisplayRect();
}

int Element::GetFont()const
{
    if( mFont > 0 )
    {
        return mFont;
    }
    else if( mParent )
    {
        return mParent->GetFont();
    }
    return 0;
}

Element* Element::GetChildByID(const std::string_view& pID)
{
    // First look for it in my children, if not, then ask them to look for it.
    for( auto child : mChildren )
    {
        if( child->GetID() == pID )
            return child;

        Element* found = child->GetChildByID(pID);
        if( found )
            return found;
    }
    return nullptr;
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

void Element::SetTextF(const char* pFmt,...)
{
    assert(pFmt);
    char buf[1024];
	va_list args;
	va_start(args, pFmt);
	vsnprintf(buf, sizeof(buf), pFmt, args);
	va_end(args);
	SetText(buf);
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
    if( mActive )
    {
        if( mEvents.OnUpdate )
        {
            mEvents.OnUpdate(this);
        }

        for( auto e : mChildren )
        {
            e->Update();
        }
    }
}

void Element::Draw(Graphics* pGraphics)
{
    if( mVisible )
    {
        const Rectangle contentRect = GetContentRectangle();

        pGraphics->DrawRectangle(contentRect,mStyle);

        if( mText.size() > 0 )
        {
            const int font = GetFont();
            if( font > 0 )
            {
                pGraphics->FontPrint(font,contentRect,mStyle.mAlignment,mStyle.mForground,mText);
            }
        }

        if( mEvents.OnDraw )
        {
            mEvents.OnDraw(this);
        }

        for( auto& e : mChildren )
        {
            e->Draw(pGraphics);
        }
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
                if( mEvents.OnPressed(this) )
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
