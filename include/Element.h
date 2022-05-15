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

typedef std::shared_ptr<class Element> ElementPtr;

/**
 * @brief This is an interface class used to allow an application to extend the functionality of an element.
 * When the element that the events object is attached to is deleted so is this object.
 * Don't share an object with more than one element.
 * The event handlers return a bool. Return true to stop the propegation of the event to children.
 * The default code is called before the handler is. So for a graphical element it is drawn before OnDraw is called.
 */
class ElementExtension
{
public:

    ElementExtension() = default;
    virtual ~ElementExtension() = default;

    virtual bool OnDraw(ElementPtr pElement,Graphics* pGraphics){return false;}
    virtual bool OnUpdate(ElementPtr pElement){return false;}
    virtual bool OnPressed(ElementPtr pElement){return false;}
    virtual bool OnDrag(ElementPtr pElement){return false;}
    virtual bool OnKey(ElementPtr pElement){return false;}
};

/**
 * @brief 
 */
class Element : public std::enable_shared_from_this<Element>
{
public:

    static ElementPtr Create(const Style& pStyle = Style());

    /**
     * @brief 
     */
    Element();
    virtual ~Element();

    /**
     * @brief The inner Rectangle that the control uses for it's children and content.
     * @return Rectangle 
     */
    Rectangle GetContentRectangle()const;
    Rectangle GetParentRectangle()const;

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

    void SetPos(uint32_t pX,uint32_t pY);
    void SetGrid(uint32_t pWidth,uint32_t pHeight);
    void SetSpan(uint32_t pX,uint32_t pY);

    void SetPadding(float pPadding);
    void SetPadding(const Rectangle& pPadding);

    void SetID(const std::string& pID){mID = pID;}
    void SetText(const std::string& pText){mText = pText;}
    void SetTextF(const char* pFmt,...);
    void SetFont(int pFont){mFont = pFont;}
    void SetStyle(const Style& pStyle){mStyle = pStyle;}

    void SetVisible(bool pVisible){mVisible = pVisible;}
    void SetActive(bool pActive){mActive = pActive;}

    void SetExtension(ElementExtension* pExtension);

    void Attach(ElementPtr pElement);
    void Remove(ElementPtr pElement);
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

private:
    ElementPtr mParent = nullptr;
    std::list<ElementPtr> mChildren;
    std::string mID;                        //!< If set can be used to search from an element.
    std::string mText;                      //!< If set, it is displayed, based on settings in the style.
    int mFont = 0;                          //!< The font to use to render text, if zero, will use parents. Used GetFont to fetch the font to render with.
    bool mVisible = true;                   //!< Turns on and off the drawing, update will still be called if mActive is true. If false, will not be drawn and children will not be.
    bool mActive = true;                    //!< If true, will update, if false will not be updated and it's childrent will not be. May still be drawn.

    Style mStyle;
    uint32_t mX = 0;
    uint32_t mY = 0;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mSpanX = 1;
    uint32_t mSpanY = 1;

    Rectangle mPadding = {0.0f,0.0f,1.0f,1.0f};

    ElementExtension* mExtension = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif//