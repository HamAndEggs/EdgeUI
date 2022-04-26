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
typedef std::function<bool(Element& pElement)> ElementEvent;

/**
 * @brief The base element that all renderable parts of the UI are constructed from. It is a container, does not render anything but does pass on rendering to children.
 * The rendering of the UI is done everyframe. This saves on work having to compute posible changes. Shifts the work onto the GPU.
 * Some members of this type are calculated when their dependencies change. This will save some cpu when rendering.
 */
class Element
{
public:

    /**
     * @brief Width and Height are fraction of parent.
     */
    static Element* Create(float pWidth = 1.0f,float pHeight = 1.0f);

    virtual ~Element();

    /**
     * @brief The inner Rectangle that the control uses for it's children and content.
     * @return Rectangle 
     */
    virtual Rectangle GetContentRectangle();

    virtual Rectangle GetParentRectangle();

    const std::string& GetID(){return mID;}
    Element* GetParent(){return mParent;}

    virtual bool GetIsVisible()const{return mVisible;}
    virtual bool GetIsEnabled()const{return mEnabled;}

    /**
     * @brief Set the position based on fraction of the width of the parent and relitive to it's x.
     */
    virtual void SetLeftTop(const Point& pPos);
    virtual void SetLeftTop(float pX,float pY){SetLeftTop(Point(pX,pY));}
    virtual void SetRightBottom(const Point& pSize);
    virtual void SetRightBottom(float pX,float pY){SetRightBottom(Point(pX,pY));}
    virtual void SetPadding(float pPadding);

    virtual void SetID(const std::string& pID){mID = pID;}

    virtual void SetOnPressed(ElementEvent pHandler){mEvents.OnPressed = pHandler;}
    virtual void SetOnDrag(ElementEvent pHandler){mEvents.OnDrag = pHandler;}
    virtual void SetOnKey(ElementEvent pHandler){mEvents.OnKey = pHandler;}

    virtual void SetForground(Colour pColour){mStyle.mForground = pColour;}
    virtual void SetBackground(Colour pColour){mStyle.mBackground = pColour;}
    virtual void SetBackground(Colour pFromGradient,Colour pToGradient,float pGradientDirection = 0){mStyle.mBackground = pFromGradient;mStyle.mBackgroundGradient = pToGradient; mStyle.mGradientDirection = pGradientDirection;}
    virtual void SetBorder(float pSize,Colour pColour = COLOUR_BLACK){mStyle.mBorderSize = pSize; mStyle.mBorder = pColour;}
    
    virtual void SetRadius(float pRadius){mStyle.mRadius = pRadius;}
    virtual void SetBorderSize(float pBorderSize){mStyle.mBorderSize = pBorderSize;}

    virtual void Attach(Element* pElement);
    virtual void Remove(Element* pElement);
    /**
     * @brief Updates all elements in the tree, if they are visible.
     * Doing full update before the draw allows all dependacies to have the correct data for rendering.
     */
    virtual void Update();

    virtual void Draw();

    /**
     * @brief Will activate the control under the screen location and deal with being touched or released.
     * The application will need to call this from the graphics touch event processing callback.
     * Could also be used for automated testing with playback of events.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     */
    virtual bool TouchEvent(float pX,float pY,bool pTouched);

protected:
    /**
     * @brief Force the creation from the factory functions.
     * This is because elements have shadered pointers or can have shadered pointers to each other.
     * If an element is created on the stack this can cause an object to be deleted twice.
     */
    Element();

private:
    Element* mParent = nullptr;
    std::list<Element*> mChildren;
    std::string mID;                        //!< If set can be used to search from an element.
    bool mVisible = true;                   //!< If true, the element and it's children will receive messages and be drawn. If not it would be as if they did not exists.
    bool mEnabled = true;                   //!< If true, will be drawn in an active state. If false will be drawn in an inactive state. Unlike visible, will receive all messages.

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