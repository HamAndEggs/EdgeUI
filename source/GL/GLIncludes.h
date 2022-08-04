#ifndef GLIncludes_H__
#define GLIncludes_H__

#ifdef PLATFORM_WAYLAND_GL
	#include <wayland-client.h>
	#include <wayland-server.h>
	#include <wayland-client-protocol.h>
	#include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
#endif


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

#endif //#ifndef GLIncludes_H__