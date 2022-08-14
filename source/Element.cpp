
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

Element::Element(const Style& pStyle)
{
    SET_DEFAULT_ID();
    COUNT_ALLOCATION();
    SetStyle(pStyle);
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

ElementPtr Element::SetPos(uint32_t pX,uint32_t pY)
{
    mX = pX;
    mY = pY;
    return this;
}

ElementPtr Element::SetGrid(uint32_t pWidth,uint32_t pHeight)
{
    mWidth = pWidth;
    mHeight = pHeight;
    return this;
}

ElementPtr Element::SetSpan(uint32_t pX,uint32_t pY)
{
    mSpanX = pX;
    mSpanY = pY;
    return this;
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

ElementPtr Element::SetPadding(float pPadding)
{
    mPadding.left = pPadding;
    mPadding.right = 1.0f - pPadding;
    mPadding.top = pPadding;
    mPadding.bottom = 1.0f - pPadding;
    return this;
}

ElementPtr Element::SetPadding(float pLeft,float pRight,float pTop,float pBottom)
{
    mPadding.left = pLeft;
    mPadding.right = pRight;
    mPadding.top = pTop;
    mPadding.bottom = pBottom;
    return this;
}

ElementPtr Element::SetPadding(const Rectangle& pPadding)
{
    mPadding = pPadding;
    return this;
}


ElementPtr Element::SetTextF(const char* pFmt,...)
{
    assert(pFmt);
    char buf[1024];
	va_list args;
	va_start(args, pFmt);
	vsnprintf(buf, sizeof(buf), pFmt, args);
	va_end(args);
	SetText(buf);
    return this;
}

ElementPtr Element::Attach(ElementPtr pElement)
{
    VERBOSE_MESSAGE("Attaching " + pElement->GetID() + " to " + mID);
    mChildren.push_back(pElement);
    pElement->mParent = this;
    return this;
}

ElementPtr Element::Remove(ElementPtr pElement)
{
    pElement->mParent = nullptr;
    mChildren.remove(pElement);
    return this;
}

void Element::Layout(const Rectangle& pParentRect)
{
    CalculateContentRectangle(pParentRect);
    for( auto& e : mChildren )
    {
        e->Layout(mContentRectangle);
    }
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
        pGraphics->DrawRectangle(mContentRectangle,mStyle);

        if( mText.size() > 0 )
        {
            const int font = GetFont();
            if( font > 0 )
            {
                pGraphics->FontPrint(font,mContentRectangle,mStyle.mAlignment,mStyle.mForeground,mText);
            }
        }

        if( OnDraw(pGraphics,mContentRectangle) )
            return;

        if( mOnDrawCB )
        {
            if( mOnDrawCB(this,pGraphics,mContentRectangle) )
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
    if( mContentRectangle.ContainsPoint(pX,pY) )
    {
        const float localX = pX - mContentRectangle.left;
        const float localY = pY - mContentRectangle.top;
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

bool Element::KeyboardEvent(char pCharacter,bool pPressed)
{
//    if( mContentRectangle.ContainsPoint(pX,pY) )// TODO: Make it send this char to selected element...
    {
        if( OnKeyboard(pCharacter,pPressed) )
        {
            return true;
        }

        if( mOnKeyboardCB )
        {
            if( mOnKeyboardCB(this,pCharacter,pPressed) )
                return true;
        }
    }

    for( auto& e : mChildren )
    {
        if( e->KeyboardEvent(pCharacter,pPressed) )
        {
            return true;
        }
    }

    return false;
}

void Element::CalculateContentRectangle(const Rectangle& pParentRect)
{
    const float cellWidth = 1.0f / GetParentWidth();
    const float cellHeight = 1.0f / GetParentHeight();
    const Rectangle rect = {cellWidth * mX,cellHeight*mY,cellWidth * (mX + mSpanX),cellHeight * (mY + mSpanY)};

    const Rectangle contentRect(pParentRect.GetX(rect.left),pParentRect.GetY(rect.top),
                                pParentRect.GetX(rect.right),pParentRect.GetY(rect.bottom));

    mContentRectangle.Set(
        contentRect.GetX(mPadding.left),
        contentRect.GetY(mPadding.top),
        contentRect.GetX(mPadding.right),
        contentRect.GetY(mPadding.bottom));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
