#ifndef GLIncludes_H__
#define GLIncludes_H__

#ifdef PLATFORM_X11_GL
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glu.h>

/**
 * @brief The EdgeUI is targeting systems without a desktop, but sometimes we want to develop on a system with it.
 * This define allows that. But is expected to be used ONLY for development.
 * To set window draw size define X11_EMULATION_WIDTH and X11_EMULATION_HEIGHT in your build settings.
 * These below are here for default behaviour.
 * Doing this saves on params that are not needed 99% of the time. Only has an affect when building for X11 and GLES emulation.
 */
#ifdef PLATFORM_X11_GL
	#ifndef X11_EMULATION_WIDTH
		#define X11_EMULATION_WIDTH 1024
	#endif

	#ifndef X11_EMULATION_HEIGHT
		#define X11_EMULATION_HEIGHT 600
	#endif
#endif

#endif //#ifndef GLIncludes_H__