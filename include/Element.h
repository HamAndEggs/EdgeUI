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

/**
 * @brief This is an interface class used to allow an application to extend the functionality of an element.
 * When the element that the events object is attached to is deleted so is this object.
 * Don't share an object with more than one element.
 * The event handlers return a bool. Return true to stop the propegation of the event to children.
 * The default code is called before the handler is. So for a graphical element it is drawn before OnDraw is called.
 */
class ElementEvents
{
public:

    ElementEvents() = default;
    virtual ~ElementEvents() = default;

    virtual bool OnDraw(Element* pElement,Graphics* pGraphics){return false;}
    virtual bool OnUpdate(Element* pElement){return false;}
    virtual bool OnPressed(Element* pElement){return false;}
    virtual bool OnDrag(Element* pElement){return false;}
    virtual bool OnKey(Element* pElement){return false;}
};

/**
 * @brief 
 */
class Element
{
public:

    /**
     * @brief Width and Height are fraction of parent.
     */
    static Element* Create(float pWidth = 1.0f,float pHeight = 1.0f);
    static Element* Create(const Style& pStyle,float pWidth = 1.0f,float pHeight = 1.0f);

    virtual ~Element();

    /**
     * @brief The inner Rectangle that the control uses for it's children and content.
     * @return Rectangle 
     */
    Rectangle GetContentRectangle()const;
    Rectangle GetParentRectangle()const;

    const std::string GetID()const{return mID;}
    const Element* GetParent()const{return mParent;}

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

    Element* GetChildByID(const std::string_view& pID);

    /**
     * @brief Set the position based on fraction of the width of the parent and relitive to it's x.
     */
    void SetLeftTop(const Point& pPos);
    void SetLeftTop(float pX,float pY){SetLeftTop(Point(pX,pY));}
    void SetRightBottom(const Point& pSize);
    void SetRightBottom(float pX,float pY){SetRightBottom(Point(pX,pY));}
    void SetPadding(float pPadding);
    void SetPadding(const Rectangle& pPadding);

    void SetID(const std::string& pID){mID = pID;}
    void SetText(const std::string& pText){mText = pText;}
    void SetTextF(const char* pFmt,...);
    void SetFont(int pFont){mFont = pFont;}
    void SetStyle(const Style& pStyle){mStyle = pStyle;}

    void SetVisible(bool pVisible){mVisible = pVisible;}
    void SetActive(bool pActive){mActive = pActive;}

    void SetEventHandler(ElementEvents* pHandlerClass){mEvents = pHandlerClass;}

    void Attach(Element* pElement);
    void Remove(Element* pElement);
    /**
     * @brief Updates all elements in the tree, if they are visible.
     * Doing full update before the draw allows all dependacies to have the correct data for rendering.
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

protected:
    /**
     * @brief 
     */
    Element();

private:
    Element* mParent = nullptr;
    std::list<Element*> mChildren;
    std::string mID;                        //!< If set can be used to search from an element.
    std::string mText;                      //!< If set, it is displayed, based on settings in the style.
    int mFont = 0;                          //!< The font to use to render text, if zero, will use parents. Used GetFont to fetch the font to render with.
    bool mVisible = true;                   //!< Turns on and off the drawing, update will still be called if mActive is true. If false, will not be drawn and children will not be.
    bool mActive = true;                    //!< If true, will update, if false will not be updated and it's childrent will not be. May still be drawn.

    Style mStyle;
    Rectangle mRect = {0.0f,0.0f,1.0f,1.0f};    //!< Rect is expressed as fraction of parent size. If no parent then the display size is used, again a fraction off.
    Rectangle mPadding = {0.0f,0.0f,1.0f,1.0f};

    ElementEvents* mEvents = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif//