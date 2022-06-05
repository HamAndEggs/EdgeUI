#ifndef PLATFORM_INTERFACE_DRM_H__
#define PLATFORM_INTERFACE_DRM_H__

#include "Graphics.h"
#include "Diagnostics.h"
#include "GLIncludes.h"

// This is for linux systems that have no window manager. Like RPi4 running their light version of raspbian or a distro built with Yocto.
// sudo apt install libdrm libdrm-dev libegl-dev libgles2-mesa-dev

#include <xf86drm.h> // sudo apt install libdrm-dev
#include <xf86drmMode.h>
#include <gbm.h>	// sudo apt install libgbm-dev // This is used to get the egl stuff going. DRM is used to do the page flip to the display. Goes.. DRM -> GDM -> GLES (I think)
#include <drm_fourcc.h>
#include "EGL/egl.h" // sudo apt install libegl-dev
#include "GLES2/gl2.h" // sudo apt install libgles2-mesa-dev

#define EGL_NO_X11
#define MESA_EGL_NO_X11_HEADERS


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

	int GetWidth()const{assert(mModeInfo);if(mModeInfo){return mModeInfo->hdisplay;}return 0;}
	int GetHeight()const{assert(mModeInfo);if(mModeInfo){return mModeInfo->vdisplay;}return 0;}

private:

	bool mIsFirstFrame = true;
	int mDRMFile = -1;

	// This is used for the EGL bring up and getting GLES going along with DRM.
	struct gbm_device *mBufferManager = nullptr;
	struct gbm_bo *mCurrentFrontBufferObject = nullptr;

	drmModeEncoder *mModeEncoder = nullptr;
	drmModeConnector* mConnector = nullptr;
	drmModeModeInfo* mModeInfo = nullptr;
	uint32_t mFOURCC_Format = DRM_FORMAT_INVALID;
	uint32_t mCurrentFrontBufferID = 0;

	EGLDisplay mDisplay = nullptr;				//!<GL display
	EGLSurface mSurface = nullptr;				//!<GL rendering surface
	EGLContext mContext = nullptr;				//!<GL rendering context
	EGLConfig mConfig = nullptr;				//!<Configuration of the display.

	struct gbm_surface *mNativeWindow = nullptr;

	/**
	 * @brief Information about the mouse driver
	 */
	struct
	{
		int mDevice = 0; //!< File handle to /dev/input/mice

		/**
		 * @brief Maintains the current known values. Because we get many messages.
		 */
		struct
		{
			bool touched = false;
			int x = 0;
			int y = 0;
		}mCurrent;
	}mPointer;

	/**
	 * @brief Looks for a mouse device we can used for touch screen input.
	 * 
	 * @return int 
	 */
	int FindMouseDevice();

	void FindEGLConfiguration();
	void UpdateCurrentBuffer();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //#ifndef PLATFORM_INTERFACE_DRM_H__