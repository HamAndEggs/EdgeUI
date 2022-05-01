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
typedef std::function<bool(Element* pElement)> ElementEvent;

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
    virtual Rectangle GetContentRectangle()const;
    virtual Rectangle GetParentRectangle()const;

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
    virtual void SetLeftTop(const Point& pPos);
    virtual void SetLeftTop(float pX,float pY){SetLeftTop(Point(pX,pY));}
    virtual void SetRightBottom(const Point& pSize);
    virtual void SetRightBottom(float pX,float pY){SetRightBottom(Point(pX,pY));}
    virtual void SetPadding(float pPadding);

    virtual void SetID(const std::string& pID){mID = pID;}
    virtual void SetText(const std::string& pText){mText = pText;}
    virtual void SetTextF(const char* pFmt,...);
    virtual void SetFont(int pFont){mFont = pFont;}
    virtual void SetStyle(const Style& pStyle){mStyle = pStyle;}

    virtual void SetVisible(bool pVisible){mVisible = pVisible;}
    virtual void SetActive(bool pActive){mActive = pActive;}

    virtual void SetOnUpdate(ElementEvent pHandler){mEvents.OnUpdate = pHandler;}
    virtual void SetOnDraw(ElementEvent pHandler){mEvents.OnDraw = pHandler;}
    virtual void SetOnPressed(ElementEvent pHandler){mEvents.OnPressed = pHandler;}
    virtual void SetOnDrag(ElementEvent pHandler){mEvents.OnDrag = pHandler;}
    virtual void SetOnKey(ElementEvent pHandler){mEvents.OnKey = pHandler;}

    virtual void Attach(Element* pElement);
    virtual void Remove(Element* pElement);
    /**
     * @brief Updates all elements in the tree, if they are visible.
     * Doing full update before the draw allows all dependacies to have the correct data for rendering.
     */
    virtual void Update();

    virtual void Draw(Graphics* pGraphics);

    /**
     * @brief Will activate the control under the screen location and deal with being touched or released.
     * The application will need to call this from the graphics touch event processing callback.
     * Could also be used for automated testing with playback of events.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     */
    virtual bool TouchEvent(float pX,float pY,bool pTouched);

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

    struct
    {
        ElementEvent OnUpdate = nullptr;
        ElementEvent OnDraw = nullptr;
        ElementEvent OnPressed = nullptr;
        ElementEvent OnDrag = nullptr;
        ElementEvent OnKey = nullptr;
    }mEvents;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif//