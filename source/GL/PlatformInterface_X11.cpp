
#include "PlatformInterface_X11.h"

#include <assert.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
PlatformInterface::PlatformInterface():
	mXDisplay(NULL),
	mWindow(0),
	mWindowReady(false)
{

}

PlatformInterface::~PlatformInterface()
{
	VERBOSE_MESSAGE("Cleaning up GL");
	mWindowReady = false;

	glXMakeCurrent(mXDisplay, 0, 0 );
	glXDestroyContext(mXDisplay,mGLXContext);

	// Do this after we have set the message pump flag to false so the events generated will case XNextEvent to return.
	XFree(mVisualInfo);
	XFreeColormap(mXDisplay,mWindowAttributes.colormap);
	XDestroyWindow(mXDisplay,mWindow);
	XCloseDisplay(mXDisplay);
	mXDisplay = nullptr;
}

void PlatformInterface::InitialiseDisplay()
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
		ProcessEvents([](int32_t pX,int32_t pY,bool pTouched){return false;},[](){});
		nanosleep(&SleepTime,NULL);
	}
}

void PlatformInterface::ProcessEvents(EventTouchScreen pTouchEvent,std::function<void()> pEventQuit)
{
	// The message pump had to be moved to the same thread as the rendering because otherwise it would fail after a little bit of time.
	// This is dispite what the documentation stated.
	if( mXDisplay == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("The X11 display object is NULL!");
	}

	if( pTouchEvent == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("The X11 touch event handler in ProcessEvents is null");
	}

	if( pEventQuit == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("The X11 quit event handler in ProcessEvents is null");
	}

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
				pEventQuit();
			}
			break;

		case KeyPress:
			// exit on ESC key press
			if ( e.xkey.keycode == 0x09 )
			{
				mWindowReady = false;
				pEventQuit();
			}
			break;

		case MotionNotify:// Mouse movement
			pTouchEvent(e.xmotion.x,e.xmotion.y,touched);
			break;

		case ButtonPress:
			touched = true;
			pTouchEvent(e.xmotion.x,e.xmotion.y,touched);
			break;

		case ButtonRelease:
			touched = false;
			pTouchEvent(e.xmotion.x,e.xmotion.y,touched);
			break;
		}
	}
}

void PlatformInterface::SwapBuffers()
{
	assert( mWindowReady );
	if( mXDisplay == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("The X11 display object is NULL!");
	}	
	glXSwapBuffers(mXDisplay,mWindow);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{
