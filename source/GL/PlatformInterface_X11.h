#ifndef PLATFORM_INTERFACE_X11_H__
#define PLATFORM_INTERFACE_X11_H__

#include "Graphics.h"
#include "Diagnostics.h"
#include "GLIncludes.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
class PlatformInterface
{
public:
	PlatformInterface();
	~PlatformInterface();

	/**
	 * @brief Creates the X11 window and all the bits needed to get rendering with.
	 */
	void InitialiseDisplay();

	/**
	 * @brief Processes the X11 events then exits when all are done.
	 */
	void ProcessEvents(EventTouchScreen pTouchEvent,std::function<void()> pEventQuit);

	/**
	 * @brief Draws the frame buffer to the X11 window.
	 */
	void SwapBuffers();

	int GetWidth()const{return X11_EMULATION_WIDTH;}
	int GetHeight()const{return X11_EMULATION_HEIGHT;}

private:
	Display *mXDisplay = nullptr;
	Window mWindow = 0;
	Atom mDeleteMessage;
	GLXContext mGLXContext = 0;
	XSetWindowAttributes mWindowAttributes;
	XVisualInfo* mVisualInfo;
	bool mWindowReady;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //#ifndef PLATFORM_INTERFACE_X11_H__