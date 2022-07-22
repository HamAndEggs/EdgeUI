#include "PlatformInterface_Wayland.h"

#include <assert.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
	if( data == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("global_registry_handler called with NULL data");
	}
	((PlatformInterface*)data)->RegistryHandler(registry,id,interface,version);
}

void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
	if( data == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("global_registry_remover called with NULL data");
	}
	((PlatformInterface*)data)->RegistryRemover(registry,id);
}

const struct wl_registry_listener WaylandListeners = {
  global_registry_handler,
  global_registry_remover
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
PlatformInterface::PlatformInterface()
{

}

PlatformInterface::~PlatformInterface()
{
	VERBOSE_MESSAGE("Cleaning up GL");
	mWindowReady = false;

//	eglDestroySurface(mEGL.display, mEGL.surface);
//	wl_egl_window_destroy(mEGL.native_window);
//	wl_shell_surface_destroy(mWayland.shell_surface);
//	wl_surface_destroy(mWayland.surface);
//	eglDestroyContext(mEGL.display, mEGL.context);

	wl_display_disconnect(mEGL.native_display);
	VERBOSE_MESSAGE("Display disconnected");
}

void PlatformInterface::InitialiseDisplay()
{
	VERBOSE_MESSAGE("Making Wayland window for GLES emulation");

	mEGL.native_display = wl_display_connect(NULL);
	if (mEGL.native_display == NULL)
	{
		THROW_MEANINGFUL_EXCEPTION("Can't connect to wayland display !?");
	}
	VERBOSE_MESSAGE("Got wayland display !");

	struct wl_registry *wl_registry = wl_display_get_registry(mEGL.native_display);
	wl_registry_add_listener(wl_registry, &WaylandListeners, this);

	// This call the attached WaylandListeners global_registry_handler
	wl_display_dispatch(mEGL.native_display);
	wl_display_roundtrip(mEGL.native_display);

	// If at this point, global_registry_handler didn't set the 
	// compositor, nor the shell, bailout !
	// mWayland.compositor is set in the registered handler.
	if (mWayland.compositor == NULL ) /* || mWayland.shell == NULL)*/
	{
		THROW_MEANINGFUL_EXCEPTION("No compositor !? No Shell !! There's NOTHING in here !\n");
	}

	VERBOSE_MESSAGE("Okay, we got wayland compositor and a shell... That's something !");

	mWayland.surface = wl_compositor_create_surface(mWayland.compositor);
	if (mWayland.surface == NULL)
	{
		THROW_MEANINGFUL_EXCEPTION("No Compositor surface ! Yay....");
	}

	VERBOSE_MESSAGE("Got a compositor surface !");
	/*
	mWayland.shell_surface = wl_shell_get_shell_surface(mWayland.shell, mWayland.surface);
	wl_shell_surface_set_toplevel(mWayland.shell_surface);*/

	mWayland.region = wl_compositor_create_region(mWayland.compositor);

	wl_region_add(mWayland.region, 0, 0, GetWidth(), GetHeight());
	wl_surface_set_opaque_region(mWayland.surface, mWayland.region);

	wl_surface_commit(mWayland.surface);
	wl_display_roundtrip(mEGL.native_display);

//	wl_surface_attach(mWayland.surface, buffer, 0, 0);
//	wl_surface_commit(mWayland.surface);


	mEGL.native_window = wl_egl_window_create(mWayland.surface, GetWidth(), GetHeight());

	if( mEGL.native_window == EGL_NO_SURFACE )
	{
		THROW_MEANINGFUL_EXCEPTION("wl_egl_window_create: No window !?");
	}

	CreateEGLContext();

}

void PlatformInterface::ProcessEvents(EventTouchScreen pTouchEvent,std::function<void()> pEventQuit)
{
	if( wl_display_dispatch(mEGL.native_display) == -1 )
	{
		pEventQuit();
	}
}

void PlatformInterface::SwapBuffers()
{
	eglSwapBuffers(mEGL.display, mEGL.surface);
}

void PlatformInterface::RegistryHandler(struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version)
{
	if( strcmp(interface,wl_compositor_interface.name) == 0)
	{
		mWayland.compositor = (wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
		if( mWayland.compositor == NULL )
		{
			THROW_MEANINGFUL_EXCEPTION("wl_registry_bind failed for wl_compositor_interface");
		}
		else
		{
			VERBOSE_MESSAGE("We have the compositor");
		}
	}
	else if( strcmp(interface,wl_shell_interface.name) == 0 )
	{
/*		mWayland.shell = (wl_shell*)wl_registry_bind(registry, id, &wl_shell_interface, 1);
		if( mWayland.shell == NULL )
		{
			THROW_MEANINGFUL_EXCEPTION("wl_registry_bind failed for wl_shell_interface");
		}
		else
		{
			VERBOSE_MESSAGE("We have the shell");
		}*/
	}
	else
	{
		VERBOSE_MESSAGE("RegistryHandler: event " << interface << " for ID " << id);
	}
}

void PlatformInterface::RegistryRemover(struct wl_registry *registry, uint32_t id)
{
	VERBOSE_MESSAGE("Got a registry losing event for " << id);
}


void PlatformInterface::CreateEGLContext()
{
	EGLint majorVersion;
	EGLint minorVersion;

	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
	mEGL.display = eglGetDisplay( mEGL.native_display );
	if ( mEGL.display == EGL_NO_DISPLAY )
	{
		THROW_MEANINGFUL_EXCEPTION("No EGL Display...\n");
	}

	// Initialize EGL
	if ( !eglInitialize(mEGL.display, &majorVersion, &minorVersion) )
	{
		THROW_MEANINGFUL_EXCEPTION("No Initialisation...\n");
	}

	VERBOSE_MESSAGE("EGL version " << majorVersion << "." << minorVersion);

	FindEGLConfiguration();

	// Create a surface
	mEGL.surface = eglCreateWindowSurface(mEGL.display, mEGL.config, mEGL.native_window, NULL);
	if ( mEGL.surface == EGL_NO_SURFACE )
	{
		THROW_MEANINGFUL_EXCEPTION("No surface...\n");
	}

	// Create a GL context
	mEGL.context = eglCreateContext(mEGL.display, mEGL.config, EGL_NO_CONTEXT, contextAttribs );
	if ( mEGL.context == EGL_NO_CONTEXT )
	{
		THROW_MEANINGFUL_EXCEPTION("No context...\n");
	}

	// Make the context current
	if ( !eglMakeCurrent(mEGL.display, mEGL.surface, mEGL.surface, mEGL.context) )
	{
		THROW_MEANINGFUL_EXCEPTION("Could not make the current window current !\n");
	}
}

void PlatformInterface::FindEGLConfiguration()
{
	int depths_32_to_16[3] = {32,24,16};

	for( int c = 0 ; c < 3 ; c++ )
	{
		const EGLint attrib_list[] =
		{
			EGL_RED_SIZE,			8,
			EGL_GREEN_SIZE,			8,
			EGL_BLUE_SIZE,			8,
			EGL_ALPHA_SIZE,			8,
			EGL_DEPTH_SIZE,			depths_32_to_16[c],
			EGL_STENCIL_SIZE,		EGL_DONT_CARE,
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
			EGL_NONE,				EGL_NONE
		};

		EGLint numConfigs;
		if( !eglChooseConfig(mEGL.display,attrib_list,&mEGL.config,1, &numConfigs) )
		{
			THROW_MEANINGFUL_EXCEPTION("Error: eglGetConfigs() failed");
		}

		if( numConfigs > 0 )
		{
			EGLint bufSize,r,g,b,a,z,s = 0;

			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_BUFFER_SIZE,&bufSize);

			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_RED_SIZE,&r);
			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_GREEN_SIZE,&g);
			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_BLUE_SIZE,&b);
			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_ALPHA_SIZE,&a);

			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_DEPTH_SIZE,&z);
			eglGetConfigAttrib(mEGL.display,mEGL.config,EGL_STENCIL_SIZE,&s);

			VERBOSE_MESSAGE("Config found:");
			VERBOSE_MESSAGE("\tFrame buffer size " << bufSize);
			VERBOSE_MESSAGE("\tRGBA " << r << "," << g << "," << b << "," << a);
			VERBOSE_MESSAGE("\tZBuffer " << z+s << "Z " << z << "S " << s);

			return;// All good :)
		}
	}
	THROW_MEANINGFUL_EXCEPTION("No matching EGL configs found");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{
