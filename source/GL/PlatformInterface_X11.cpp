

#include "Graphics.h"
#include "Diagnostics.h"
#include "GLIncludes.h"
#include "Application.h"
#include "Element.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glu.h>

#include <assert.h>
#include <thread>

#ifndef DESKTOP_EMULATION_WIDTH
	#define DESKTOP_EMULATION_WIDTH 1024
	#define DESKTOP_EMULATION_HEIGHT 600
#endif
namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
class PlatformInterface_X11 : public Graphics
{
public:
	PlatformInterface_X11(Application* pApplication);
	~PlatformInterface_X11();

	void MainLoop();

	/**
	 * @brief Creates the X11 window and all the bits needed to get rendering with.
	 */
	void InitialiseDisplay();

	/**
	 * @brief Draws the frame buffer to the X11 window.
	 */
	void SwapBuffers();

	int GetWidth()const{return DESKTOP_EMULATION_WIDTH;}
	int GetHeight()const{return DESKTOP_EMULATION_HEIGHT;}


private:
	Application* mUsersApplication = nullptr;
	Graphics* mGraphics = nullptr;

	Display *mXDisplay = nullptr;
	Window mWindow = 0;
	Atom mDeleteMessage;
	GLXContext mGLXContext = 0;
	XSetWindowAttributes mWindowAttributes;
	XVisualInfo* mVisualInfo;
	bool mWindowReady = false;


	uint32_t mUpdateFrequency = 0;

	void ProcessEvents();
};

PlatformInterface_X11::PlatformInterface_X11(Application* pApplication):
	mUsersApplication(pApplication),
	mXDisplay(NULL),
	mWindow(0),
	mWindowReady(false)
{
	InitialiseDisplay();
}

PlatformInterface_X11::~PlatformInterface_X11()
{
	VERBOSE_MESSAGE("Cleaning up GL");
	mWindowReady = false;

	delete mGraphics;
	mGraphics = nullptr;

	glXMakeCurrent(mXDisplay, 0, 0 );
	glXDestroyContext(mXDisplay,mGLXContext);

	// Do this after we have set the message pump flag to false so the events generated will case XNextEvent to return.
	XFree(mVisualInfo);
	XFreeColormap(mXDisplay,mWindowAttributes.colormap);
	XDestroyWindow(mXDisplay,mWindow);
	XCloseDisplay(mXDisplay);
	mXDisplay = nullptr;
}

void PlatformInterface_X11::InitialiseDisplay()
{
	VERBOSE_MESSAGE("Making X11 window for GLES emulation");

	mXDisplay = XOpenDisplay(NULL);
	if( mXDisplay == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to open X display");
	}

	int glx_major, glx_minor;

	// FBConfigs were added in GLX version 1.3.
	if( glXQueryVersion(mXDisplay, &glx_major, &glx_minor) == GL_FALSE )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to fetch glx version information");
	}
	VERBOSE_MESSAGE("GLX version " << glx_major << "." << glx_minor);

	static int visual_attribs[] =
	{
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		//GLX_SAMPLE_BUFFERS  , 1,
		//GLX_SAMPLES         , 4,
		None
	};

	int numConfigs;
	GLXFBConfig* fbc = glXChooseFBConfig(mXDisplay, DefaultScreen(mXDisplay), visual_attribs, &numConfigs);
	if( fbc == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to retrieve a framebuffer config");
	}

	VERBOSE_MESSAGE("Found " << numConfigs << " matching FB configs, picking first one");
	const GLXFBConfig bestFbc = fbc[0];
	XFree(fbc);

	mVisualInfo = glXGetVisualFromFBConfig( mXDisplay, bestFbc );
	VERBOSE_MESSAGE("Chosen visual ID = " << mVisualInfo->visualid );

	VERBOSE_MESSAGE("Creating colormap");
	mWindowAttributes.colormap = XCreateColormap(mXDisplay,RootWindow(mXDisplay,0),mVisualInfo->visual, AllocNone );	;
	mWindowAttributes.background_pixmap = None ;
	mWindowAttributes.border_pixel      = 0;
	mWindowAttributes.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask ;

	mWindow = XCreateWindow(
					mXDisplay,
					RootWindow(mXDisplay, mVisualInfo->screen),
					10, 10,
					DESKTOP_EMULATION_WIDTH, DESKTOP_EMULATION_HEIGHT,
					0, mVisualInfo->depth, InputOutput, mVisualInfo->visual,
					CWBorderPixel|CWColormap|CWEventMask,
					&mWindowAttributes );
	if( !mWindow )
	{
		THROW_MEANINGFUL_EXCEPTION("Falid to create XWindow for our GL application");
	}

	XStoreName(mXDisplay, mWindow, "Tiny GLES");
	XMapWindow(mXDisplay, mWindow);

	mGLXContext = glXCreateNewContext( mXDisplay, bestFbc, GLX_RGBA_TYPE, 0, True );
	XSync(mXDisplay,False);

	VERBOSE_MESSAGE("Making context current");
	glXMakeCurrent(mXDisplay,mWindow,mGLXContext);

	// So I can exit cleanly if the user uses the close window button.
	mDeleteMessage = XInternAtom(mXDisplay, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(mXDisplay, mWindow, &mDeleteMessage, 1);

	// wait for the expose message.
  	timespec SleepTime = {0,1000000};
	while( !mWindowReady )
	{
		ProcessEvents();
		nanosleep(&SleepTime,NULL);
	}

	mGraphics = new Graphics();
	mGraphics->InitialiseGL(GetWidth(), GetHeight());
	mUsersApplication->OnOpen(mGraphics);

}

void PlatformInterface_X11::ProcessEvents()
{
	// The message pump had to be moved to the same thread as the rendering because otherwise it would fail after a little bit of time.
	// This is despite what the documentation stated.
	if( mXDisplay == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("The X11 display object is NULL!");
	}
	ElementPtr root = mUsersApplication->GetRootElement();

	static bool touched = false;
	while( XPending(mXDisplay) )
	{
		XEvent e;
		XNextEvent(mXDisplay,&e);
		switch( e.type )
		{
		case Expose:
			mWindowReady = true;
			break;

		case ClientMessage:
			// All of this is to stop and error when we try to use the display but has been disconnected.
			// Snip from X11 docs.
			// 	Clients that choose not to include WM_DELETE_WINDOW in the WM_PROTOCOLS property
			// 	may be disconnected from the server if the user asks for one of the
			//  client's top-level windows to be deleted.
			// 
			// My note, could have been avoided if we just had something like XDisplayStillValid(my display)
			if (static_cast<Atom>(e.xclient.data.l[0]) == mDeleteMessage)
			{
				mWindowReady = false;
				mUsersApplication->SetExit();
			}
			break;

		case KeyPress:
			// exit on ESC key press
			if ( e.xkey.keycode == 0x09 )
			{
				mWindowReady = false;
				mUsersApplication->SetExit();
			}
			break;

		case MotionNotify:// Mouse movement
			root->CursorEvent(e.xmotion.x,e.xmotion.y,touched,true);
			break;

		case ButtonPress:
			touched = true;
			root->CursorEvent(e.xmotion.x,e.xmotion.y,touched,false);
			break;

		case ButtonRelease:
			touched = false;
			root->CursorEvent(e.xmotion.x,e.xmotion.y,touched,false);
			break;
		}
	}
}

void PlatformInterface_X11::SwapBuffers()
{
	assert( mWindowReady );
	if( mXDisplay == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("The X11 display object is NULL!");
	}	
	glXSwapBuffers(mXDisplay,mWindow);
}

void PlatformInterface_X11::MainLoop()
{
	do
	{
		assert(mUsersApplication);
		assert(mGraphics);
		
		const auto loopTime = std::chrono::system_clock::now() + std::chrono::milliseconds(mUsersApplication->GetUpdateInterval());
		
		mUsersApplication->OnFrame(mGraphics,mGraphics->GetDisplayRect());

		SwapBuffers();

		// We call this as much as possible, if we call based on update rate, message response starts to behave badly.
		do
		{
			ProcessEvents();
			using namespace std::chrono_literals;
			// we don't need to poll messages as fast as possible, so wait a little.
			// Saves a lot of cpu time. On an AMD Ryzan 4800 goes from 16% cpu load to 0.2% load.
			std::this_thread::sleep_for(1ms);
		}while( loopTime > std::chrono::system_clock::now() );
	}while(mUsersApplication->GetKeepGoing());

	mUsersApplication->OnClose();
}

void Application::MainLoop(Application* pApplication)
{
	PlatformInterface_X11 platform(pApplication);
	platform.MainLoop();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
