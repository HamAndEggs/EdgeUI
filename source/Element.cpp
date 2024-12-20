
#include "Element.h"
#include "ResourceMap.h"
#include "Graphics.h"

// Controls that we can load from a file.
#include "controls/Controls.h"


#include <memory>
#include <cstdarg>

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

Element::Element(const tinyjson::JsonValue &root,ResouceMap* pLoadResources)
{
    if(pLoadResources == nullptr)
    {
        THROW_MEANINGFUL_EXCEPTION("Resouce map is null");
    }

    for(const auto &child : root)
    {
    // First check for the properties we know about, and then scan for child objects.
        if( child.first == "resource" )
        {// Skip, will have been loaded by the resource map.
        }
        else if( child.first == "pos" )
        {
            const tinyjson::JsonValue &pos = child.second;
            if( pos.GetType() != tinyjson::JsonValueType::ARRAY )
            {
                THROW_MEANINGFUL_EXCEPTION("Elements position data in json file is not an array");
            }

            if( pos.mArray.size() != 2 )
            {
                THROW_MEANINGFUL_EXCEPTION("Elements position array is not two just two elements");
            }

            SetPos(pos[0],pos[1]);
        }
        else if( child.first == "grid" )
        {
            const tinyjson::JsonValue &grid = child.second;
            if( grid.GetType() == tinyjson::JsonValueType::ARRAY )
            {
                if( grid.mArray.size() != 2 )
                {
                    THROW_MEANINGFUL_EXCEPTION("Elements position array is not two just two elements");
                }
                SetGrid(grid[0],grid[1]);
            }
            else if( grid.GetType() == tinyjson::JsonValueType::BOOLEAN )
            {
                SetAutoGrid(grid.GetBoolean());
            }
            else
            {
                THROW_MEANINGFUL_EXCEPTION("Elements grid data in json file is not an array or a boolean");
            }
        }
        else if( child.first == "span" )
        {
            const tinyjson::JsonValue &span = child.second;
            if( span.GetType() != tinyjson::JsonValueType::ARRAY )
            {
                THROW_MEANINGFUL_EXCEPTION("Elements span data in json file is not an array");
            }

            if( span.mArray.size() != 2 )
            {
                THROW_MEANINGFUL_EXCEPTION("Elements span array is not two just two elements");
            }

            SetSpan(span[0],span[1]);
        }
        else if( child.first == "pad" )
        {
            const tinyjson::JsonValue &padding = child.second;
            if( padding.GetType() == tinyjson::JsonValueType::ARRAY )
            {
                if( padding.mArray.size() == 2 )
                {
                    SetPadding(padding[0],padding[1]);
                }
                else if( padding.mArray.size() == 4 )
                {
                    SetPadding(padding[0],padding[1],padding[2],padding[3]);
                }
                else
                {
                    THROW_MEANINGFUL_EXCEPTION("Elements position array is not two just two elements");
                }

            }
            else if( padding.GetType() == tinyjson::JsonValueType::NUMBER )
            {
                SetPadding(padding);
            }
            else
            {
                THROW_MEANINGFUL_EXCEPTION("Elements padding data in json file is not an array or a number");
            }
        }
        else if( child.first == "text" )
        {
            SetText(child.second);
        }
        else if( child.first == "visible" )
        {
            SetVisible(child.second);
        }
        else if( child.first == "active" )
        {
            SetActive(child.second);
        }
        else if( child.first == "user_value" )
        {
    //        ElementPtr SetUserValue(uint32_t pUserValue){mUserValue = pUserValue;return this;}
        }
        else if( child.first == "style" )
        {
            SetStyle(child.second,pLoadResources);
        }
        else if( child.first == "control" )
        {
            // skip this.
        }
        else
        {// Assume all else is a new child element.
            // Check to see if it's got a child stating a control, if not use normal element.
            ElementPtr e = nullptr;
            if( child.second.HasValue("control") )
            {
                VERBOSE_MESSAGE("Found control: " << child.second["control"].GetString());
                e = LoadControl(child.second,pLoadResources);
            }
            else
            {
                e = new Element(child.second,pLoadResources);
            }
            e->SetID(child.first);
            Attach(e);
            VERBOSE_MESSAGE("Added child:" << child.first);
        }
    }
}

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

const uint32_t Element::GetFont()const
{
    if( GetStyle().mFont != 0 )
    {
        return GetStyle().mFont;
    }

    if( mParent == nullptr )
    {
        THROW_MEANINGFUL_EXCEPTION("No font set for this UI");
    }
    return mParent->GetFont();
}

ElementPtr Element::SetPos(uint32_t pX,uint32_t pY)
{
    mX = pX;
    mY = pY;
    return this;
}

ElementPtr Element::SetGrid(uint32_t pWidth,uint32_t pHeight)
{
    mAutoGrid = false;
    mWidth = pWidth;
    mHeight = pHeight;
    return this;
}

ElementPtr Element::SetAutoGrid(bool pHorizontal)
{
    mAutoGrid = true;
    mAutoGridHorizontal = pHorizontal;
    return this;
}

ElementPtr Element::SetSpan(uint32_t pX,uint32_t pY)
{
    mAutoGrid = false;
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

ElementPtr Element::SetPadding(float pX,float pY)
{
    mPadding.left = pX;
    mPadding.right = 1.0f - pX;
    mPadding.top = pY;
    mPadding.bottom = 1.0f - pY;
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

ElementPtr Element::SetStyle(const tinyjson::JsonValue &fStyle,ResouceMap* pLoadResources)
{
    if(pLoadResources == nullptr)
    {
        THROW_MEANINGFUL_EXCEPTION("Resouce map is null whilst bu8ilding style");
    }

    Style aStyle;

    if( fStyle.HasValue("foreground") )
    {
        aStyle.mForeground = MakeColour(fStyle["foreground"]);
    }

    if( fStyle.HasValue("background") )
    {
        aStyle.mBackground = MakeColour(fStyle["background"]);
    }

    if( fStyle.HasValue("border") )
    {
        aStyle.mBorder = MakeColour(fStyle["border"]);
        VERBOSE_MESSAGE("thickness " << aStyle.mBorder);
    }

    if( fStyle.HasValue("radius") )
    {
        aStyle.mRadius = fStyle["radius"];
        VERBOSE_MESSAGE("radius " << aStyle.mRadius);
    }

    if( fStyle.HasValue("thickness") )
    {
        aStyle.mThickness = fStyle["thickness"];
        VERBOSE_MESSAGE("thickness " << aStyle.mThickness);
    }

    if( fStyle.HasValue("font") )
    {
        aStyle.mFont = pLoadResources->get(fStyle["font"]);
    }

    if( fStyle.HasValue("texture") )
    {
        aStyle.mTexture = pLoadResources->get(fStyle["texture"]);
    }

    if( fStyle.HasValue("boarder_style") )
    {
        const tinyjson::JsonValue &boarder_style = fStyle["boarder_style"];
        if(boarder_style.GetType() == tinyjson::JsonValueType::STRING)
        {
            const std::string bs = boarder_style;
            VERBOSE_MESSAGE("boarder style " << bs);
            if( bs == "SOLID" )
            {
                aStyle.mBoarderStyle = BS_SOLID;
            }
            else if( bs == "RAISED" )
            {
                aStyle.mBoarderStyle = BS_RAISED;
            }
            else if( bs == "DEPRESSED" )
            {
                aStyle.mBoarderStyle = BS_DEPRESSED;
            }
        }
        else
        {
            THROW_MEANINGFUL_EXCEPTION("Boarder style in style is not a string");
        }
    }

    if( fStyle.HasValue("alignment") )
    {
        const tinyjson::JsonValue &alignment = fStyle["alignment"];
        if(alignment.GetType() == tinyjson::JsonValueType::STRING)
        {
            aStyle.mAlignment = StringToAlignment(fStyle["alignment"]);
        }
        else
        {
            THROW_MEANINGFUL_EXCEPTION("Boarder style in style is not a string");
        }
    }

    SetStyle(aStyle);
    return this;
}


ElementPtr Element::SetStyle(eui::Colour pColour,BoarderStyle pBoarderStyle,float pBoarderSize,float pRadius,uint32_t pFont)
{
    eui::Style s;
    s.mBackground = pColour;
    s.mBoarderStyle = pBoarderStyle;
    s.mBorder = eui::COLOUR_WHITE;
    s.mThickness = pBoarderSize;
    s.mRadius = pRadius;
    s.mFont = pFont;
    SetStyle(s);
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
    if( mAutoGrid && mChildren.size() > 0 )
    {
        if( mAutoGridHorizontal )
        {
            mWidth = mChildren.size();
            mHeight = 1;
        }
        else
        {
            mWidth = 1;
            mHeight = mChildren.size();
        }
    }

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
        // I do not like the logic here.
        bool propagateToChildren = OnUpdate(mContentRectangle) == false;
        if( mOnUpdateCB && mOnUpdateCB(this,mContentRectangle) == false )
        {
            propagateToChildren = true;
        }
        
        if( propagateToChildren )
        {
            for( auto e : mChildren )
            {
                e->Update();
            }
        }
    }
}

void Element::Draw(Graphics* pGraphics)
{
    // Catch people causing infinite loops by calling draw within other draw functions / overloads / callbacks.
    if( mAlreadyDrawing )
    {
        THROW_MEANINGFUL_EXCEPTION("Draw called when already drawing. You have a recusion error.");
    }

    mAlreadyDrawing = true;
    if( mVisible )
    {
        // I do not like the logic here.
        bool propagateToChildren = OnDraw(pGraphics,mContentRectangle) == false;
        if( mOnDrawCB && mOnDrawCB(this,pGraphics,mContentRectangle) == false )
        {
            propagateToChildren = true;
        }

        if( propagateToChildren )
        {
            for( auto& e : mChildren )
            {
                e->Draw(pGraphics);
            }
        }
    }
    mAlreadyDrawing = false;
}

bool Element::CursorEvent(float pX,float pY,bool pTouched,bool pMoving)
{
    if( mContentRectangle.ContainsPoint(pX,pY) )
    {
        const float localX = pX - mContentRectangle.left;
        const float localY = pY - mContentRectangle.top;
        if( OnTouched(localX,localY,pTouched,pMoving) )
        {
            return true;
        }

        if( mOnTouchedCB )
        {
            if( mOnTouchedCB(this,localX,localY,pTouched,pMoving) )
                return true;
        }
    }

    for( auto& e : mChildren )
    {
        if( e->CursorEvent(pX,pY,pTouched,pMoving) )
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

bool Element::OnDraw(Graphics* pGraphics,const Rectangle& pContentRect)
{
    DrawRectangle(pGraphics,mContentRectangle,mStyle);

    if( mText.size() > 0 )
    {
        const uint32_t font = GetFont();
        if( font != 0 )
        {
            pGraphics->FontPrint(font,mContentRectangle,mStyle.mAlignment,mStyle.mForeground,mText);
        }
        else
        {
            THROW_MEANINGFUL_EXCEPTION("Element has no fonts to render text with");
        }
    }
    return false;
}

void Element::DrawRectangle(Graphics* pGraphics,const Rectangle& pRect,const Style& pStyle,bool pUseForground)
{
    assert(pGraphics);
    pGraphics->DrawRectangle(pRect,
						pUseForground ? pStyle.mForeground:pStyle.mBackground,
						pStyle.mBorder,
						pStyle.mRadius,
						pStyle.mThickness,
						pStyle.mTexture,
						pStyle.mBoarderStyle);

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

ElementPtr Element::LoadControl(const tinyjson::JsonValue &root,ResouceMap* pLoadResources)
{
    const std::string type = root["control"];
    if( type == Button::ClassID() )
    {
        return new Button(root,pLoadResources);
    }
    else if( type == Checkbox::ClassID() )
    {
        return new Checkbox(root,pLoadResources);
    }
    else if( type == RadioButton::ClassID() )
    {
        return new RadioButton(root,pLoadResources);
    }
    else if( type == Slider::ClassID() )
    {
        return new Slider(root,pLoadResources);
    }

    THROW_MEANINGFUL_EXCEPTION("Unknown control type:" + type);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
