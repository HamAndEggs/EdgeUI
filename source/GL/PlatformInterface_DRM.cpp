#include "GLDiagnostics.h"
#include "Graphics.h"
#include "Application.h"
#include "Element.h"


#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#include <chrono>
#include <thread>

// This is for linux systems that have no window manager. Like RPi4 running their light version of raspbian or a distro built with Yocto.
// sudo apt install libdrm libdrm-dev libegl-dev libgles2-mesa-dev

#include <xf86drm.h> // sudo apt install libdrm-dev
#include <xf86drmMode.h>
#include <gbm.h>	// sudo apt install libgbm-dev // This is used to get the egl stuff going. DRM is used to do the page flip to the display. Goes.. DRM -> GDM -> GLES (I think)
#include <drm_fourcc.h>
#include "EGL/egl.h" // sudo apt install libegl-dev
//#include "GLES2/gl2.h" // sudo apt install libgles2-mesa-dev

#define EGL_NO_X11
#define MESA_EGL_NO_X11_HEADERS

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
class PlatformInterface_DRM : public Graphics
{
public:
	PlatformInterface_DRM(Application* pApplication);
	virtual ~PlatformInterface_DRM();

	void MainLoop();

	void InitialiseDisplay();

	int GetWidth()const{assert(mModeInfo);if(mModeInfo){return mModeInfo->hdisplay;}return 0;}
	int GetHeight()const{assert(mModeInfo);if(mModeInfo){return mModeInfo->vdisplay;}return 0;}


private:
	Application* mUsersApplication = nullptr;
	Graphics* mGraphics = nullptr;

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
	uint32_t mUpdateFrequency = 0;

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
	void ProcessEvents();
	void SwapBuffers();

};


PlatformInterface_DRM::PlatformInterface_DRM(Application* pApplication):
	mUsersApplication(pApplication)
{
	mPointer.mDevice = FindMouseDevice();

	if( drmAvailable() == 0 )
	{
		THROW_MEANINGFUL_EXCEPTION("Kernel DRM driver not loaded");
	}

	// Lets go searching for a connected direct render manager device.
	// Later I could add a param to allow user to specify this.
	drmDevicePtr devices[8] = { NULL };
	int num_devices = drmGetDevices2(0, devices, 8);
	if (num_devices < 0)
	{
		THROW_MEANINGFUL_EXCEPTION("drmGetDevices2 failed: " + std::string(strerror(-num_devices)) );
	}

	mDRMFile = -1;
	for( int n = 0 ; n < num_devices && mDRMFile < 0 ; n++ )
	{
		if( devices[n]->available_nodes&(1 << DRM_NODE_PRIMARY) )
		{
			// See if we can open it...
			VERBOSE_MESSAGE("Trying DRM device " << std::string(devices[n]->nodes[DRM_NODE_PRIMARY]));
			mDRMFile = open(devices[n]->nodes[DRM_NODE_PRIMARY], DRM_RDWR);
		}
	}
	drmFreeDevices(devices, num_devices);

	if( mDRMFile < 0 )
	{
		THROW_MEANINGFUL_EXCEPTION("DirectRenderManager: Failed to find and open direct rendering manager device" );
	}
	drmModeRes* resources = drmModeGetResources(mDRMFile);
	if( resources == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("DirectRenderManager: Failed get mode resources");
	}

	drmModeConnector* connector = nullptr;
	for(int n = 0 ; n < resources->count_connectors && connector == nullptr ; n++ )
	{
		connector = drmModeGetConnector(mDRMFile, resources->connectors[n]);
		if( connector && connector->connection != DRM_MODE_CONNECTED )
		{// Not connected, check next one...
			drmModeFreeConnector(connector);
			connector = nullptr;
		}
	}
	if( connector == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("DirectRenderManager: Failed get mode connector");
	}
	mConnector = connector;

	for( int i = 0 ; i < connector->count_modes && mModeInfo == nullptr ; i++ )
	{
		if( connector->modes[i].type & DRM_MODE_TYPE_PREFERRED )
		{// DRM really wants us to use this, this should be the best option for LCD displays.
			mModeInfo = &connector->modes[i];
			VERBOSE_MESSAGE("Preferred screen mode found");
		}
	}

	// Did'nt find a 'preferred' type, so pick first one.
	if( mModeInfo == nullptr && connector->count_modes > 0 )
	{
		mModeInfo = &connector->modes[0];
		VERBOSE_MESSAGE("NON-Preferred screen mode used");
	}

	if( mModeInfo == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("DirectRenderManager: Failed to find mode info");
	}

	if( GetWidth() == 0 || GetHeight() == 0 )
	{
		THROW_MEANINGFUL_EXCEPTION("DirectRenderManager: Failed to find screen mode");
	}

	// Now grab the encoder, we need it for the CRTC ID. This is display connected to the connector.
	for( int n = 0 ; n < resources->count_encoders && mModeEncoder == nullptr ; n++ )
	{
		drmModeEncoder *encoder = drmModeGetEncoder(mDRMFile, resources->encoders[n]);
		if( encoder->encoder_id == connector->encoder_id )
		{
			mModeEncoder = encoder;
		}
		else
		{
			drmModeFreeEncoder(encoder);
		}
	}

	drmModeFreeResources(resources);

	InitialiseDisplay();
	mGraphics = new Graphics();
	mGraphics->InitialiseGL(GetWidth(), GetHeight());
	mUsersApplication->OnOpen(mGraphics);

}

PlatformInterface_DRM::~PlatformInterface_DRM()
{
	delete mGraphics;
	mGraphics = nullptr;

	VERBOSE_MESSAGE("Destroying context");
	eglDestroyContext(mDisplay, mContext);
    eglDestroySurface(mDisplay, mSurface);
    eglTerminate(mDisplay);

	VERBOSE_MESSAGE("Cleaning up DRM");
	gbm_surface_destroy(mNativeWindow);
	gbm_device_destroy(mBufferManager);
	drmModeFreeEncoder(mModeEncoder);
	drmModeFreeConnector(mConnector);
	close(mDRMFile);
	close(mPointer.mDevice);
}

void PlatformInterface_DRM::MainLoop()
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

int PlatformInterface_DRM::FindMouseDevice()
{
	for( int n = 0 ; n < 16 ; n++ )
	{
		const std::string devName = "/dev/input/event" + std::to_string(n);

		int device = open(devName.c_str(),O_RDONLY|O_NONBLOCK);
		if( device >  0 )
		{
			VERBOSE_MESSAGE("Opened input device: " + devName);

			// Get it's version.
			int version = 0;
			if( ioctl(device, EVIOCGVERSION, &version) == 0 )
			{	// That worked, keep going. Get it's ID
				VERBOSE_MESSAGE("Input driver version is " << (version >> 16) << "." << ((version >> 8)&0xff) << "." << (version&0xff) );
				struct input_id id;
				if( ioctl(device, EVIOCGID, &id) == 0 )
				{// Get the name
					VERBOSE_MESSAGE("Input device ID: bus 0x" << std::hex << id.bustype << " vendor 0x" << id.vendor << " product 0x" << id.product << " version 0x" << id.version << std::dec);
					char name[256] = "Unknown";
					if( ioctl(device, EVIOCGNAME(sizeof(name)), name) > 0 )
					{// Get control bits.
						VERBOSE_MESSAGE("Input device name: " << name);
						auto test_bit = [](uint32_t bits[],uint32_t bit)
						{
							return (bits[bit/32] & (1UL<<(bit%32)));
						};

						uint32_t EV_KEYbits[(KEY_MAX/32) + 1];
						uint32_t EV_ABSbits[(KEY_MAX/32) + 1];
						memset(EV_KEYbits, 0, sizeof(EV_KEYbits));
						memset(EV_ABSbits, 0, sizeof(EV_ABSbits));
						if( ioctl(device, EVIOCGBIT(EV_KEY, KEY_MAX), EV_KEYbits) > 0 )
						{
							if( ioctl(device, EVIOCGBIT(EV_ABS, KEY_MAX), EV_ABSbits) > 0 )
							{
								// See if it has the control bits we want.
								if( test_bit(EV_KEYbits,BTN_TOUCH) &&
									test_bit(EV_ABSbits,ABS_X) &&
									test_bit(EV_ABSbits,ABS_X) )
								{
									// We'll have this one please
									return device;
								}
							}
							else
							{
								VERBOSE_MESSAGE("Failed to read EVIOCGBIT EV_ABS");
							}
						}
						else
						{
							VERBOSE_MESSAGE("Failed to read EVIOCGBIT EV_KEY");
						}
					}
				}
			}
			// Get here, no luck, close device check next one.
			close(device);
			VERBOSE_MESSAGE("Input device is not the one we want");
		}
	}

	return 0;
}

void PlatformInterface_DRM::ProcessEvents()
{
	// We don't bother to read the mouse if no pEventHandler has been registered. Would be a waste of time.
	if( mPointer.mDevice > 0 )
	{
		ElementPtr root = mUsersApplication->GetRootElement();
		assert(root);

		struct input_event ev;
		// Grab all messages and process befor going to next frame.
		while( read(mPointer.mDevice,&ev,sizeof(ev)) > 0 )
		{
			// EV_SYN is a seperator of events.
#ifdef VERBOSE_BUILD
			if( ev.type != EV_ABS && ev.type != EV_KEY && ev.type != EV_SYN )
			{// Anything I missed? 
				std::cout << std::hex << ev.type << " " << ev.code << " " << ev.value  << std::dec << "\n";
			}
#endif
			switch( ev.type )
			{
			case EV_KEY:
				switch (ev.code)
				{
				case BTN_TOUCH:
					mPointer.mCurrent.touched = (ev.value != 0);
					root->TouchEvent(mPointer.mCurrent.x,mPointer.mCurrent.y,mPointer.mCurrent.touched);
					break;
				}
				break;

			case EV_ABS:
				switch (ev.code)
				{
				case ABS_X:
					mPointer.mCurrent.x = ev.value;
					break;

				case ABS_Y:
					mPointer.mCurrent.y = ev.value;
					break;
				}
				root->TouchEvent(mPointer.mCurrent.x,mPointer.mCurrent.y,mPointer.mCurrent.touched);
				break;
			}
		}
	}
	
}

void PlatformInterface_DRM::InitialiseDisplay()
{
	VERBOSE_MESSAGE("Calling DRM InitialiseDisplay");

	mBufferManager = gbm_create_device(mDRMFile);
	mDisplay = eglGetDisplay(mBufferManager);
	if( !mDisplay )
	{
		THROW_MEANINGFUL_EXCEPTION("Couldn\'t open the EGL default display");
	}

	//Now we have a display lets initialize it.
	EGLint majorVersion,minorVersion;
	if( !eglInitialize(mDisplay, &majorVersion, &minorVersion) )
	{
		THROW_MEANINGFUL_EXCEPTION("eglInitialize() failed");
	}
	CHECK_OGL_ERRORS();
	VERBOSE_MESSAGE("EGL version " << majorVersion << "." << minorVersion);
	eglBindAPI(EGL_OPENGL_ES_API);
	CHECK_OGL_ERRORS();

	FindEGLConfiguration();

	//We have our display and have chosen the config so now we are ready to create the rendering context.
	VERBOSE_MESSAGE("Creating context");
	EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	mContext = eglCreateContext(mDisplay,mConfig,EGL_NO_CONTEXT,ai32ContextAttribs);
	if( !mContext )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to get a rendering context");
	}

	mNativeWindow = gbm_surface_create(mBufferManager,GetWidth(), GetHeight(),mFOURCC_Format,GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
	mSurface = eglCreateWindowSurface(mDisplay,mConfig,mNativeWindow,0);
	CHECK_OGL_ERRORS();

	eglMakeCurrent(mDisplay, mSurface, mSurface, mContext );
	CHECK_OGL_ERRORS();
}

void PlatformInterface_DRM::FindEGLConfiguration()
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
		if( !eglChooseConfig(mDisplay,attrib_list,&mConfig,1, &numConfigs) )
		{
			THROW_MEANINGFUL_EXCEPTION("Error: eglGetConfigs() failed");
		}

		if( numConfigs > 0 )
		{
			EGLint bufSize,r,g,b,a,z,s = 0;

			eglGetConfigAttrib(mDisplay,mConfig,EGL_BUFFER_SIZE,&bufSize);

			eglGetConfigAttrib(mDisplay,mConfig,EGL_RED_SIZE,&r);
			eglGetConfigAttrib(mDisplay,mConfig,EGL_GREEN_SIZE,&g);
			eglGetConfigAttrib(mDisplay,mConfig,EGL_BLUE_SIZE,&b);
			eglGetConfigAttrib(mDisplay,mConfig,EGL_ALPHA_SIZE,&a);

			eglGetConfigAttrib(mDisplay,mConfig,EGL_DEPTH_SIZE,&z);
			eglGetConfigAttrib(mDisplay,mConfig,EGL_STENCIL_SIZE,&s);

			CHECK_OGL_ERRORS();

			// Get the format we need DRM buffers to match.
			if( r == 8 && g == 8 && b == 8 )
			{
				if( a == 8 )
				{
					mFOURCC_Format = DRM_FORMAT_ARGB8888;
				}
				else
				{
					mFOURCC_Format = DRM_FORMAT_RGB888;
				}
			}
			else
			{
				mFOURCC_Format = DRM_FORMAT_RGB565;
			}

			VERBOSE_MESSAGE("Config found:");
			VERBOSE_MESSAGE("\tFrame buffer size " << bufSize);
			VERBOSE_MESSAGE("\tRGBA " << r << "," << g << "," << b << "," << a);
			VERBOSE_MESSAGE("\tZBuffer " << z+s << "Z " << z << "S " << s);

			return;// All good :)
		}
	}
	THROW_MEANINGFUL_EXCEPTION("No matching EGL configs found");
}

static void drm_fb_destroy_callback(struct gbm_bo *bo, void *data)
{
	uint32_t* user_data = (uint32_t*)data;
	delete user_data;
}

void PlatformInterface_DRM::UpdateCurrentBuffer()
{
	assert(mNativeWindow);
	mCurrentFrontBufferObject = gbm_surface_lock_front_buffer(mNativeWindow);
	if( mCurrentFrontBufferObject == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to lock front buffer from native window.");
	}

	uint32_t* user_data = (uint32_t*)gbm_bo_get_user_data(mCurrentFrontBufferObject);
	if( user_data == nullptr )
	{
		// Annoying JIT allocation. Should only happen twice.
		// Should look at removing the need for the libgbm

		const uint32_t handles[4] = {gbm_bo_get_handle(mCurrentFrontBufferObject).u32,0,0,0};
		const uint32_t strides[4] = {gbm_bo_get_stride(mCurrentFrontBufferObject),0,0,0};
		const uint32_t offsets[4] = {0,0,0,0};

		user_data = new uint32_t;
		int ret = drmModeAddFB2(mDRMFile, GetWidth(), GetHeight(), mFOURCC_Format,handles, strides, offsets, user_data, 0);
		if (ret)
		{
			THROW_MEANINGFUL_EXCEPTION("failed to create frame buffer " + std::string(strerror(ret)) + " " + std::string(strerror(errno)) );
		}
		gbm_bo_set_user_data(mCurrentFrontBufferObject,user_data, drm_fb_destroy_callback);
		VERBOSE_MESSAGE("JIT allocating drm frame buffer " << (*user_data));
	}
	mCurrentFrontBufferID = *user_data;
}

static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
	/* suppress 'unused parameter' warnings */
	(void)fd, (void)frame, (void)sec, (void)usec;
	*((bool*)data) = 0;	// Set flip flag to false
}

void PlatformInterface_DRM::SwapBuffers()
{
	eglSwapBuffers(mDisplay,mSurface);

	UpdateCurrentBuffer();

	if( mIsFirstFrame )
	{
		mIsFirstFrame = false;
		assert(mModeEncoder);
		assert(mConnector);
		assert(mModeInfo);

		int ret = drmModeSetCrtc(mDRMFile, mModeEncoder->crtc_id, mCurrentFrontBufferID, 0, 0,&mConnector->connector_id, 1, mModeInfo);
		if (ret)
		{
			THROW_MEANINGFUL_EXCEPTION("drmModeSetCrtc failed to set mode" + std::string(strerror(ret)) + " " + std::string(strerror(errno)) );
		}
	}

	// Using DRM_MODE_PAGE_FLIP_EVENT as some devices don't support DRM_MODE_PAGE_FLIP_ASYNC.
	bool waiting_for_flip = 1;
	int ret = drmModePageFlip(mDRMFile, mModeEncoder->crtc_id, mCurrentFrontBufferID,DRM_MODE_PAGE_FLIP_EVENT,&waiting_for_flip);
	if (ret)
	{
		THROW_MEANINGFUL_EXCEPTION("drmModePageFlip failed to queue page flip " + std::string(strerror(errno)) );
	}

	while (waiting_for_flip)
	{
		drmEventContext evctx;
		memset(&evctx,0,sizeof(evctx));

		evctx.version = 2;
		evctx.page_flip_handler = page_flip_handler;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(mDRMFile, &fds);

		// For some reason this fails when we do ctrl + c dispite hooking into the interrupt.
		ret = select(mDRMFile + 1, &fds, NULL, NULL, NULL);
		if( ret < 0 )
		{
			// I wanted this to be an exception but could not, see comment on the select. So just cout::error for now...	
			std::cerr << "PlatformInterface_DRM::SwapBuffer select on DRM file failed to queue page flip " << std::string(strerror(errno)) << "\n";
		}

		drmHandleEvent(mDRMFile, &evctx);
	}

	gbm_surface_release_buffer(mNativeWindow,mCurrentFrontBufferObject);
}

void Application::MainLoop(Application* pApplication)
{
	PlatformInterface_DRM platform(pApplication);
	platform.MainLoop();

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
