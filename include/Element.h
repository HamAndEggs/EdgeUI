#ifndef ELEMENT_H__
#define ELEMENT_H__

#include <memory>
#include <string>
#include <functional>
#include <list>

#include "Style.h"
#include "Diagnostics.h"
#include "Rectangle.h"
#include "DataBinding.h"

#include "../TinyJson/TinyJson.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

class Element;
class Graphics;
struct ResouceMap;

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
    Element(const tinyjson::JsonValue &root,ResouceMap* pLoadResources);
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

    std::string GetText()const{return mText;}

    static std::string ClassID(){return "eui::element";}
    virtual std::string GetClassID()const{return ClassID();}

    const std::string GetID()const{return mID;}
    const ElementPtr GetParent()const{return mParent;}

    /**
     * @brief Finds the first set style that contains a font.
     */
    const uint32_t GetFont()const;

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
    std::list<ElementPtr>& GetChildren(){return mChildren;}
    size_t GetNumChildren(){return mChildren.size();}

    uint32_t GetUserValue()const{return mUserValue;}

    ElementPtr SetPos(uint32_t pX,uint32_t pY);
    ElementPtr SetGrid(uint32_t pWidth,uint32_t pHeight);
    ElementPtr SetAutoGrid(bool pHorizontal = false);       // Will cause the system to use a 1XN grid where N is the number of children.
    ElementPtr SetSpan(uint32_t pX,uint32_t pY);

    ElementPtr SetPadding(float pPadding);
    ElementPtr SetPadding(float pX,float pY);
    ElementPtr SetPadding(float pLeft,float pRight,float pTop,float pBottom);
    ElementPtr SetPadding(const Rectangle& pPadding);

    ElementPtr SetID(const std::string& pID){mID = pID;return this;}
    ElementPtr SetText(const std::string& pText){mText = pText;return this;}
    ElementPtr SetTextF(const char* pFmt,...);

    ElementPtr SetStyle(const Style& pStyle){mStyle = pStyle;return this;}
    ElementPtr SetStyle(const tinyjson::JsonValue &root,ResouceMap* pLoadResources);
    ElementPtr SetStyle(eui::Colour pColour,BoarderStyle pBoarderStyle,float pBoarderSize,float pRadius,uint32_t pFont);


    ElementPtr SetVisible(bool pVisible){mVisible = pVisible;return this;}
    ElementPtr SetActive(bool pActive){mActive = pActive;return this;}

    ElementPtr SetUserValue(uint32_t pUserValue){mUserValue = pUserValue;return this;}

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
     * Called a cursor event as the cursor can either be a mouse, screen or joypad.
     * Using the name cursor to be more implemention agnostic.
     * Could also be used for automated testing with playback of events.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     * This is called by the graphics backend. Application does not need to call this.
     */
    bool CursorEvent(float pX,float pY,bool pTouched,bool pMoving);

    /**
     * @brief When a key is pressed or released.
     * TODO: Needs to deal with 'selected' controls.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     */
    bool KeyboardEvent(char pCharacter,bool pPressed);

    /**
     * @brief Override these functions to extend the functionality of a control to build your own element types.
     * Return false to stop propagation of events to children.
     */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual bool OnDraw(Graphics* pGraphics,const Rectangle& pContentRect);
    virtual bool OnUpdate(const Rectangle& pContentRect){return false;}
    virtual bool OnTouched(float pLocalX,float pLocalY,bool pTouched,bool pMoving){return false;}
    virtual bool OnKeyboard(char pCharacter,bool pPressed){return false;}
#pragma GCC diagnostic pop

    /**
     * @brief As well as using inheritance to change an elements behaviour, we can use dependency injection for more simple control customisation.
     */
    typedef std::function<bool (ElementPtr pElement,Graphics* pGraphics,const Rectangle& pContentRect)>         OnDrawCB;
    typedef std::function<bool (ElementPtr pElement,const Rectangle& pContentRect)>                             OnUpdateCB;
    typedef std::function<bool (ElementPtr pElement,float pLocalX,float pLocalY,bool pTouched,bool pMoving)>    OnTouchedCB;
    typedef std::function<bool (ElementPtr pElement,char pCharacter,bool pPressed)>                             OnKeyboardCB;

    ElementPtr SetOnDraw(OnDrawCB pOnDrawCB){mOnDrawCB = pOnDrawCB;return this;}
    ElementPtr SetOnUpdate(OnUpdateCB pOnUpdateCB){mOnUpdateCB = pOnUpdateCB;return this;}
    ElementPtr SetOnTouched(OnTouchedCB pOnTouchedCB){mOnTouchedCB = pOnTouchedCB;return this;}
    ElementPtr SetOnKeyboard(OnKeyboardCB pOnKeyboardCB){mOnKeyboardCB = pOnKeyboardCB;return this;}

protected:
    void DrawRectangle(Graphics* pGraphics,const Rectangle& pRect,const Style& pStyle,bool pUseForground = false);

private:

    ElementPtr mParent = nullptr;
    std::list<ElementPtr> mChildren;
    std::string mID;                        //!< If set can be used to search for an element.
    std::string mText;                      //!< If set, it is displayed, based on settings in the style.

    DataBinding mDataBinding;               //!< Adds the usual expected data binding functionality expected on a UI system.

    bool mVisible = true;                   //!< Turns on and off the drawing, update will still be called if mActive is true. If false, will not be drawn and children will not be.
    bool mActive = true;                    //!< If true, will update, if false will not be updated and it's children will not be. May still be drawn.
    uint32_t mUserValue = 0;                //!< Allows a user to attach a value then read it later. This helps a lot for custom controls.
    bool mAlreadyDrawing = false;           //!< Used to catch unintentional recursion. Will one day change API so can not happen.
    bool mAutoGrid = false;                 //!< If true the grid size is based on the number of children.
    bool mAutoGridHorizontal = false;       //!< States if the grid is horizontal or vertical.

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
    ElementPtr LoadControl(const tinyjson::JsonValue &root,ResouceMap* pLoadResources);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif