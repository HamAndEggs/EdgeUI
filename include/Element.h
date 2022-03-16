#ifndef ELEMENT_H__
#define ELEMENT_H__

#include <memory>
#include <string>
#include <functional>
#include <list>

#include "Graphics.h"
#include "Style.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

class Element;
typedef std::shared_ptr<Element> ElementPtr;
typedef std::function<bool(Element& pElement)> ElementEvent;

static const int32_t ELEMENT_SIZE_USE_PARENT = -1;

/**
 * @brief The base element that all renderable parts of the UI are constructed from.
 * The rendering of the UI is done anew everyframe. This saves on work having to compute posible changes. Shifts the work onto the GPU.
 * Some members of this type are calculated when their dependencies change. This will save some cpu when rendering.
 */
class Element
{
public:
    Element(GraphicsPtr pGraphics);
    virtual ~Element(){}

    GraphicsPtr GetGraphics(){return mGraphics;}
    /**
     * @brief Get the rectangle of the entire element.
     * @return Rectangle 
     */
    Rectangle GetArea()const{return mArea;}

    /**
     * @brief The inner Rectangle that the control uses for it's children and content.
     * This is computed and so a copy is returned. Also makes code safer.
     * @return Rectangle 
     */
    Rectangle GetContentRectangle()const{return mComputed.contentRect;}

    void SetArea(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight);
    void SetArea(const Rectangle& pArea);
    void SetPos(int32_t pX,int32_t pY);
    void SetSize(int32_t pWidth,int32_t pHeight);

    void SetPadding(int32_t pLeft,int32_t pTop,int32_t pRight,int32_t pBottom);
    void SetPadding(int32_t pSpace){SetPadding(pSpace,pSpace,pSpace,pSpace);}

    void SetID(const std::string& pID){mID = pID;}

    /**
     * @brief Set the content to be text
     * Pass empty string to remove content.
     */
    void SetLabel(const std::string& pLabel);

    void SetVisible(bool pVisible = true){mVisible = pVisible;}
    void SetEnabled(bool pEnabled = true){mEnabled = pEnabled;}
    
    void SetFont(uint32_t pFont){mStyle.mFont = pFont;}

    void SetForground(Colour pColour){mStyle.mForground = pColour;}
    void SetBackground(Colour pColour){mStyle.mBackground = pColour;}
    void SetBorder(uint32_t pSize,Colour pColour = COLOUR_BLACK){mStyle.mBorderSize = pSize; mStyle.mBorder = pColour;}
    
    void SetRadius(uint32_t pRadius){mStyle.mRadius = pRadius;}
    void SetBorderSize(uint32_t pBorderSize){mStyle.mBorderSize = pBorderSize;}

    void SetOnPressed(ElementEvent pHandler){mEvents.OnPressed = pHandler;}
    void SetOnDrag(ElementEvent pHandler){mEvents.OnDrag = pHandler;}
    void SetOnKey(ElementEvent pHandler){mEvents.OnKey = pHandler;}

    void Attach(ElementPtr& pElement);

    /**
     * @brief Updates all elements in the tree, if they are visible.
     * Doing full update before the draw allows all dependacies to have the correct data for rendering.
     */
    void Update(const Rectangle& pParentContectRect = Rectangle());

    /**
     * @brief Renders the element and then it's children, if they are visible.
     * Children position is rendered relitive to their parent.
     */
    void Draw();

    /**
     * @brief Will activate the control under the screen location and deal with being touched or released.
     * The application will need to call this from the graphics touch event processing callback.
     * Could also be used for automated testing with playback of events.
     * Will return true if handled, will call all children until one handles it if it did not handle it.
     */
    bool TouchEvent(int32_t pX,int32_t pY,bool pTouched);

    // Factory functions, items made here will be attached to this.
    ElementPtr AddElement(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight);
    ElementPtr AddButton(int32_t pX,int32_t pY,int32_t pWidth,int32_t pHeight,const std::string& pText,ElementEvent pOnPressed);
    ElementPtr AddLabel(int32_t pX,int32_t pY,const std::string& pText);
    ElementPtr AddGridlayout(uint32_t pColumns, uint32_t pRows);

protected:
    virtual void RecomputeRectangles(const Rectangle& pParentContectRect);

private:
    GraphicsPtr mGraphics;
    std::list<ElementPtr> mChildren;
    Rectangle mArea;                        //!< This is the area of the element, it's boarder, excluding any margin.
    std::string mID;                        //!< If set can be used to search from an element.
    std::string mLabel;                     //!< If set, is rendered with the elements font.
    bool mVisible = true;                   //!< If true, the element and it's children will receive messages and be drawn. If not it would be as if they did not exists.
    bool mEnabled = true;                   //!< If true, will be drawn in an active state. If false will be drawn in an inactive state. Unlike visible, will receive all messages.
    bool mContentDirty = true;              //!< If some part of the content has changed this will be true. Update uses this.

    Style mStyle;

    /**
     * @brief The margin is the space around an element’s border. The padding is the space between an element’s border and the element’s content
     * https://blog.hubspot.com/website/css-margin-vs-padding
     */
    struct
    {
        int32_t left = 0;
        int32_t right = 0;
        int32_t top = 0;
        int32_t bottom = 0;
    }mPadding;

    /**
     * @brief This is a copy of computed values. They are updated when one of their dependacies are altered.
     */
    struct
    {
        Rectangle contentRect;  //!< The rectangle around the content of the control and it's children.
    }mComputed;

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