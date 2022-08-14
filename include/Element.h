#ifndef ELEMENT_H__
#define ELEMENT_H__

#include <memory>
#include <string>
#include <functional>
#include <list>

#include "Graphics.h"
#include "Style.h"
#include "Diagnostics.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

class Element;
class Graphics;

typedef Element* ElementPtr;

/**
 * @brief 
 */
class Element
{
public:
    /**
     * @brief 
     */
    Element(const Style& pStyle = eui::Style());
    virtual ~Element();

    /**
     * @brief The inner Rectangle that the control uses for it's children and content.
     * @return Rectangle 
     */
    Rectangle GetContentRectangle()const{return mContentRectangle;}

    uint32_t GetParentWidth()const;
    uint32_t GetParentHeight()const;
    uint32_t GetWidth()const{return mWidth;}
    uint32_t GetHeight()const{return mHeight;}

    const std::string GetID()const{return mID;}
    const ElementPtr GetParent()const{return mParent;}

    int GetFont()const;

    /**
     * @brief Gets a reference to the internal style so you can modify it.
     */
    Style& GetStyle(){return mStyle;}

    /**
     * @brief For const objects, returns a copy.
     * You not allowed to modify const object.
     */
    Style GetStyle()const{return mStyle;}

    bool GetIsVisible()const{return mVisible;}
    bool GetIsActive()const{return mActive;}

    ElementPtr GetChildByID(const std::string_view& pID);

    ElementPtr SetPos(uint32_t pX,uint32_t pY);
    ElementPtr SetGrid(uint32_t pWidth,uint32_t pHeight);
    ElementPtr SetSpan(uint32_t pX,uint32_t pY);

    ElementPtr SetPadding(float pPadding);
    ElementPtr SetPadding(float pLeft,float pRight,float pTop,float pBottom);
    ElementPtr SetPadding(const Rectangle& pPadding);

    ElementPtr SetID(const std::string& pID){mID = pID;return this;}
    ElementPtr SetText(const std::string& pText){mText = pText;return this;}
    ElementPtr SetTextF(const char* pFmt,...);
    ElementPtr SetFont(int pFont){mFont = pFont;return this;}
    ElementPtr SetStyle(const Style& pStyle){mStyle = pStyle;return this;}

    ElementPtr SetVisible(bool pVisible){mVisible = pVisible;return this;}
    ElementPtr SetActive(bool pActive){mActive = pActive;return this;}

    ElementPtr Attach(ElementPtr pElement);
    ElementPtr Remove(ElementPtr pElement);

    /**
     * @brief Calculates it's content rect based on it's parents.
     * For correct updating and rendering, call before update.
     * @param pParentRect 
     */
    void Layout(const Rectangle& pParentRect);

    /**
     * @brief Updates all elements in the tree, if they are visible.
     * For correct rendering, call before draw.
     */
    void Update();

    void Draw(Graphics* pGraphics);

    /**
     * @brief Will activate the control under the screen location and deal with being touched or released.
     * The application will need to call this from the graphics touch event processing callback.
     * Could also be used for automated testing with playback of events.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     */
    bool TouchEvent(float pX,float pY,bool pTouched);

    /**
     * @brief When a key is pressed or released.
     * TODO: Needs to deal with 'selected' controls.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     */
    bool KeyboardEvent(char pCharacter,bool pPressed);

    /**
     * @brief Override these functions to extend the functionality of a control to build your own element types.
     * Return true to stop propagation of events to children.
     */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual bool OnDraw(Graphics* pGraphics,const Rectangle& pContentRect){return false;}
    virtual bool OnUpdate(){return false;}
    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched){return false;}
    virtual bool OnKeyboard(char pCharacter,bool pPressed){return false;}
#pragma GCC diagnostic pop

    /**
     * @brief As well as using inheritance to change an elements behaviour, we can use dependency injection for more simple control customisation.
     */
    typedef std::function<bool (ElementPtr pElement,Graphics* pGraphics,const Rectangle& pContentRect)> OnDrawCB;
    typedef std::function<bool (ElementPtr pElement)>                                                   OnUpdateCB;
    typedef std::function<bool (ElementPtr pElement,float pLocalX,float pLocalY,bool pTouched)>         OnTouchedCB;
    typedef std::function<bool (ElementPtr pElement,char pCharacter,bool pPressed)>                     OnKeyboardCB;

    ElementPtr SetOnDraw(OnDrawCB pOnDrawCB){mOnDrawCB = pOnDrawCB;return this;}
    ElementPtr SetOnUpdate(OnUpdateCB pOnUpdateCB){mOnUpdateCB = pOnUpdateCB;return this;}
    ElementPtr SetOnTouched(OnTouchedCB pOnTouchedCB){mOnTouchedCB = pOnTouchedCB;return this;}
    ElementPtr SetOnKeyboard(OnKeyboardCB pOnKeyboardCB){mOnKeyboardCB = pOnKeyboardCB;return this;}

private:
    ElementPtr mParent = nullptr;
    std::list<ElementPtr> mChildren;
    std::string mID;                        //!< If set can be used to search from an element.
    std::string mText;                      //!< If set, it is displayed, based on settings in the style.
    int mFont = 0;                          //!< The font to use to render text, if zero, will use parents. Used GetFont to fetch the font to render with.
    bool mVisible = true;                   //!< Turns on and off the drawing, update will still be called if mActive is true. If false, will not be drawn and children will not be.
    bool mActive = true;                    //!< If true, will update, if false will not be updated and it's children will not be. May still be drawn.

    Style mStyle;
    uint32_t mX = 0;
    uint32_t mY = 0;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mSpanX = 1;
    uint32_t mSpanY = 1;

    Rectangle mPadding = {0.0f,0.0f,1.0f,1.0f};
    Rectangle mContentRectangle = {0.0f,0.0f,1.0f,1.0f};

    OnDrawCB mOnDrawCB = nullptr;
    OnUpdateCB mOnUpdateCB = nullptr;
    OnTouchedCB mOnTouchedCB = nullptr;
    OnKeyboardCB mOnKeyboardCB = nullptr;

    void CalculateContentRectangle(const Rectangle& pParentRect);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif//