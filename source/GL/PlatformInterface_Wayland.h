#ifndef PLATFORM_INTERFACE_WAYLAND_H__
#define PLATFORM_INTERFACE_WAYLAND_H__

#include "Graphics.h"
#include "Diagnostics.h"
#include "GLIncludes.h"

#include <EGL/egl.h>
#include <EGL/eglplatform.h>
// I used the code in the following repo to get me started.
// https://gist.github.com/Miouyouyou/ca15af1c7f2696f66b0e013058f110b4

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

	int GetWidth()const{return DESKTOP_EMULATION_WIDTH;}
	int GetHeight()const{return DESKTOP_EMULATION_HEIGHT;}

private:

	struct
	{
		EGLNativeDisplayType native_display;
		EGLNativeWindowType native_window;
		EGLDisplay display = nullptr;				//!<GL display
		EGLSurface surface = nullptr;				//!<GL rendering surface
		EGLContext context = nullptr;				//!<GL rendering context
		EGLConfig config = nullptr;				//!<Configuration of the display.
	}mEGL;

	struct
	{
		struct wl_compositor *compositor = nullptr;
		struct wl_surface *surface = nullptr;
//		struct wl_shell *shell = nullptr;
//		struct wl_shell_surface *shell_surface = nullptr;
		struct wl_egl_window *EGLWindow = nullptr;
		struct wl_region *region = nullptr;
	}mWayland;

	bool mWindowReady = false;

	friend void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
	friend void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id);

	void RegistryHandler(struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
	void RegistryRemover(struct wl_registry *registry, uint32_t id);
	void CreateEGLContext();
	void FindEGLConfiguration();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //#ifndef PLATFORM_INTERFACE_WAYLAND_H__