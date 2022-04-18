#ifndef GRAPHICS_H__
#define GRAPHICS_H__

#include "GraphicsTypes.h"
#include "Rectangle.h"
#include "Point.h"

#include <memory>
#include <functional>


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

struct Style;
class Graphics;
typedef std::function<bool(int32_t pX,int32_t pY,bool pTouched)> EventTouchScreen;

/**
 * @brief This is the interface definition to a facade this is implemented by the hardware (GPU) renderer chosen, for example GL. 
 */
class Graphics
{
public:

    /**
     * @brief Sets the rotation of the display, rotates clockwise.
     */
    enum DisplayRotation
    {
        ROTATE_FRAME_BUFFER_0,
        ROTATE_FRAME_BUFFER_90,
        ROTATE_FRAME_BUFFER_180,
        ROTATE_FRAME_BUFFER_270,

        ROTATE_FRAME_PORTRATE,		//!< If the hardware reports a landscape mode (width > height)  will apply a 90 degree rotation
        ROTATE_FRAME_LANDSCAPE,		//!< If the hardware reports a portrate mode (width < height) will apply a 90 degree rotation
    };

    Graphics();
    virtual ~Graphics();

    static Graphics* Open(DisplayRotation pDisplayRotation = ROTATE_FRAME_PORTRATE);
    static void      Close();
    static Graphics* Get(); // Will throw an exception if open has not been called.

    /**
     * @brief Get the display rectangle
     */
    virtual Rectangle GetDisplayRect()const = 0;
    virtual RectangleF GetDisplayRectF()const = 0;

    virtual int32_t GetDisplayWidth()const = 0;

    virtual int32_t GetDisplayHeight()const = 0;

    /**
     * @brief Builds a set of points that can be used for drawing a rounded rectangle with lines or polygons.
     */
    void GetRoundedRectanglePoints(const RectangleF& pRect,VertFloatXY::Buffer& rBuffer,float pRadius);

	/**
	 * @brief Sets the flag for the main loop to false and fires the SYSTEM_EVENT_EXIT_REQUEST
	 * You would typically call this from a UI button to quit the app.
	 */
	void SetExitRequest();

	/**
	 * @brief Marks the start of the frame.
	 */
	virtual void BeginFrame() = 0;

	/**
	 * @brief Called at the end of the rendering phase. Normally last part line in the while loop.
	 */
	virtual void EndFrame() = 0;

    /**
     * @brief Processes system events and call the touch event when the display is 'touched'
     * For X11 implemention touch is emulated with the mouse.
	 * @return true All is ok, so keep running.
	 * @return false Either the user requested an exit with ctrl+c or there was an error.
     */
    virtual bool ProcessSystemEvents(EventTouchScreen mTouchEvent) = 0;

    virtual void GetFontRect(uint32_t pFont,RectangleF& rRect) = 0;
    virtual void DrawFont(const Style& pStyle,int32_t pX,int32_t pY,const std::string& pText) = 0;

    virtual void DrawRectangle(const RectangleF& pRect,const Style& pStyle) = 0;

    virtual void DrawLine(int pFromX,int pFromY,int pToX,int pToY,Colour pColour,uint32_t pWidth = 1) = 0;

protected:
    bool mExitRequest = false;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //GRAPHICS_H__