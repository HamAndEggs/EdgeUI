
#include "Element.h"
#include <memory>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
#ifdef VERBOSE_BUILD
    struct ElementAllocationCounter
    {    
        ~ElementAllocationCounter()
        {
            VERBOSE_MESSAGE("ElementAllocationCounter: count == " << count);
            VERBOSE_MESSAGE("ElementAllocationCounter: max == " << max);
            assert(count == 0);
        }

        void Inc()
        {
            count++;
            if( max < count )
            {
                max = count;
            }
        }

        void Dev()
        {
            count--;
        }

    private:
        int count = 0;
        int max = 0;
    };
    ElementAllocationCounter Count;
    #define COUNT_ALLOCATION() {Count.Inc();}
    #define COUNT_DELETE() {Count.Dev();}
#else
    #define COUNT_ALLOCATION()
    #define COUNT_DELETE()
#endif

ElementPtr Element::Create(const Style& pStyle)
{
    ElementPtr root = new Element();
    root->SetStyle(pStyle);
    return root;
}

Element::Element()
{
    SET_DEFAULT_ID();
    COUNT_ALLOCATION();
}

Element::~Element()
{
    VERBOSE_MESSAGE("Freeing Element: " + mID);
    for( auto child : mChildren )
    {
        delete child;
    }
    COUNT_DELETE();
}

Rectangle Element::GetContentRectangle()const
{
    const Rectangle parentContentRect = GetParentRectangle();
    const float cellWidth = 1.0f / GetParentWidth();
    const float cellHeight = 1.0f / GetParentHeight();
    const Rectangle rect = {cellWidth * mX,cellHeight*mY,cellWidth * (mX + mSpanX),cellHeight * (mY + mSpanY)};

    const Rectangle contentRect(parentContentRect.GetX(rect.left),parentContentRect.GetY(rect.top),
                                parentContentRect.GetX(rect.right),parentContentRect.GetY(rect.bottom));

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

uint32_t Element::GetParentWidth()const
{
    if( mParent )
    {
        return mParent->GetWidth();
    }
    return 1;
}

uint32_t Element::GetParentHeight()const
{
    if( mParent )
    {
        return mParent->GetHeight();
    }
    return 1;
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

void Element::SetPos(uint32_t pX,uint32_t pY)
{
    mX = pX;
    mY = pY;
}

void Element::SetGrid(uint32_t pWidth,uint32_t pHeight)
{
    mWidth = pWidth;
    mHeight = pHeight;
}

void Element::SetSpan(uint32_t pX,uint32_t pY)
{
    mSpanX = pX;
    mSpanY = pY;
}

ElementPtr Element::GetChildByID(const std::string_view& pID)
{
    // First look for it in my children, if not, then ask them to look for it.
    for( auto child : mChildren )
    {
        if( child->GetID() == pID )
            return child;

        ElementPtr found = child->GetChildByID(pID);
        if( found )
            return found;
    }
    return nullptr;
}

void Element::SetPadding(float pPadding)
{
    mPadding.left = pPadding;
    mPadding.right = 1.0f - pPadding;
    mPadding.top = pPadding;
    mPadding.bottom = 1.0f - pPadding;
}

void Element::SetPadding(const Rectangle& pPadding)
{
    mPadding = pPadding;
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

void Element::Attach(ElementPtr pElement)
{
    VERBOSE_MESSAGE("Attaching " + pElement->GetID() + " to " + mID);
    mChildren.push_back(pElement);
    pElement->mParent = this;
}

void Element::Remove(ElementPtr pElement)
{
    pElement->mParent = nullptr;
    mChildren.remove(pElement);
}

void Element::Update()
{
    if( mActive )
    {
        if( OnUpdate() )
            return;

        if( mOnUpdateCB )
        {
            if( mOnUpdateCB(this) )
                return;
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

        if( OnDraw(pGraphics) )
            return;

        if( mOnDrawCB )
        {
            if( mOnDrawCB(this,pGraphics) )
                return;
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
        const float localX = pX - contentRect.left;
        const float localY = pY - contentRect.top;
        if( OnTouched(localX,localY,pTouched) )
        {
            return true;
        }

        if( mOnTouchedCB )
        {
            if( mOnTouchedCB(this,localX,localY,pTouched) )
                return true;
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
