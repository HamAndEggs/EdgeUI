

#include "Diagnostics.h"
#include "GLIncludes.h"
#include "Graphics.h"
#include "Element.h"
#include "Application.h"

#include <assert.h>
#include <gtk/gtk.h> // sudo apt install libgtk-4-dev
//#include <gtk/gtkglarea.h>
#include <thread>

#define DESKTOP_EMULATION_WIDTH 1024
#define DESKTOP_EMULATION_HEIGHT 600

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
class PlatformInterface_GTK4
{
public:
	PlatformInterface_GTK4(Application* pApplication);
	~PlatformInterface_GTK4();

	void MainLoop();

	// Gtk signals.
	gboolean Signal_Render(GtkGLArea *area, GdkGLContext *context);
	void Signal_Realize(GtkWidget *widget);
	void Signal_Activate(GtkApplication* app);
	void Signal_Destroy(GtkApplication* app);
	
	void Signal_MouseMove(double x,double y);
	void Signal_MouseButton(double x,double y,bool Pressed);
	void Signal_KeyPressed(guint keyval,guint keycode,GdkModifierType state);
	void Signal_KeyReleased(guint keyval,guint keycode,GdkModifierType state);

private:
	Application* mUsersApplication = nullptr;
	GtkApplication *mGtk4Application = nullptr;
	GMainContext *mContext = nullptr;
	GtkWidget *mWindow = nullptr;
	GtkWidget *mGL = nullptr;
	GtkWidget *mGrid = nullptr;


	Graphics* mGraphics = nullptr;

	GtkGLArea* GetArea(){return (GtkGLArea*)mGL;}

	struct
	{
		float LastX = 0;
		float LastY = 0;
	}mMouse;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks from GTK4
static gboolean RenderCB(GtkGLArea *area, GdkGLContext *context,gpointer user_data)
{
	assert(user_data);
	return ((PlatformInterface_GTK4*)user_data)->Signal_Render(area,context);
}

static void RealizeCB(GtkWidget *widget,gpointer user_data )
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_Realize(widget);
}

static void ActivateCB(GtkApplication* app, gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_Activate(app);
}

static void DestroyCB(GtkApplication* app, gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_Destroy(app);
}

static void MouseMoveCB(GtkEventControllerMotion *controller,double x,double y,gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_MouseMove(x,y);
}

static void MouseButtonPressedCB(GtkGestureClick *gesture,int n_press,double x,double y,gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_MouseButton(x,y,true);
}

static void MouseButtonReleasedCB(GtkGestureClick *gesture,int n_press,double x,double y,gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_MouseButton(x,y,false);
}

static void KeyPressedCB(GtkEventControllerKey *controller,guint keyval,guint keycode,GdkModifierType state,gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_KeyPressed(keyval,keycode,state);
}

static void KeyReleasedCB(GtkEventControllerKey *controller,guint keyval,guint keycode,GdkModifierType state,gpointer user_data)
{
	assert(user_data);
	((PlatformInterface_GTK4*)user_data)->Signal_KeyReleased(keyval,keycode,state);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////

PlatformInterface_GTK4::PlatformInterface_GTK4(Application* pApplication) : mUsersApplication(pApplication)
{
	mGtk4Application = gtk_application_new ("com.hex-edge.ui", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(mGtk4Application, "activate", G_CALLBACK(ActivateCB), this);

	mContext = g_main_context_default ();
	if( !g_main_context_acquire(mContext) )
	{
		THROW_MEANINGFUL_EXCEPTION("Cannot acquire the default main context because it is already acquired by another thread!");
	}

	GError *error = NULL;
	if( !g_application_register(G_APPLICATION(mGtk4Application), NULL, &error) )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to register application: " + std::string(error->message) );
	}
	g_application_activate(G_APPLICATION(mGtk4Application));
}

PlatformInterface_GTK4::~PlatformInterface_GTK4()
{
	delete mGraphics;
	g_object_unref(mGtk4Application);
}

void PlatformInterface_GTK4::MainLoop()
{
	do
	{
		assert(mUsersApplication);
		assert(mGL);
		
		gtk_widget_queue_draw(mGL);

		const auto loopTime = std::chrono::system_clock::now() + std::chrono::milliseconds(mUsersApplication->GetUpdateInterval());
		// We call this as much as possible, if we call based on update rate, message response starts to behave badly.
		do
		{
			g_main_context_iteration(mContext, FALSE);
			using namespace std::chrono_literals;
			// we don't need to poll messages as fast as possible, so wait a little.
			// Saves a lot of cpu time. On an AMD Ryzan 4800 goes from 16% cpu load to 0.2% load.
			std::this_thread::sleep_for(1ms);
		}while( loopTime > std::chrono::system_clock::now() );
	}while(mUsersApplication->GetKeepGoing());

	// Clean up.
	g_settings_sync ();
	while (g_main_context_iteration (mContext, FALSE));
	g_main_context_release (mContext);

	mUsersApplication->OnClose();
}

gboolean PlatformInterface_GTK4::Signal_Render(GtkGLArea *area, GdkGLContext *context)
{
	assert(mUsersApplication);
	assert(mGraphics);
	mUsersApplication->OnFrame(mGraphics,mGraphics->GetDisplayRect());
	return TRUE;
}

void PlatformInterface_GTK4::Signal_Realize(GtkWidget *widget)
{
	gtk_gl_area_make_current(GetArea());
	mGraphics = new Graphics();

	const int width = gtk_widget_get_width(mGL);
	const int height = gtk_widget_get_height(mGL);

	std::cout << "The size we got: " << width << "X" << height << "\n";

	mGraphics->InitialiseGL(DESKTOP_EMULATION_WIDTH,DESKTOP_EMULATION_HEIGHT);
	mUsersApplication->OnOpen(mGraphics);
}

void PlatformInterface_GTK4::Signal_Activate(GtkApplication* app)
{
	mWindow = gtk_application_window_new (app);
	g_signal_connect(mWindow, "destroy", G_CALLBACK(DestroyCB), this);
	gtk_window_set_title (GTK_WINDOW (mWindow), "EdgeUI");
	gtk_window_set_default_size (GTK_WINDOW (mWindow), DESKTOP_EMULATION_WIDTH, DESKTOP_EMULATION_HEIGHT);

	mGrid = gtk_grid_new();
	gtk_window_set_child (GTK_WINDOW (mWindow), mGrid);

	mGL = gtk_gl_area_new();
    gtk_widget_set_hexpand(mGL, TRUE);
    gtk_widget_set_vexpand(mGL, TRUE);
	gtk_widget_set_size_request (mGL, DESKTOP_EMULATION_WIDTH, DESKTOP_EMULATION_HEIGHT);	
	g_object_set(mGL, "use-es", TRUE, NULL);
	g_object_set(mGL, "has-depth-buffer", TRUE, NULL);
	g_object_set(mGL, "has-stencil-buffer", TRUE, NULL);

	g_signal_connect(mGL, "render", G_CALLBACK(RenderCB), this);
	g_signal_connect(mGL, "realize", G_CALLBACK(RealizeCB), this);

	GtkEventController *controller;

	controller = gtk_event_controller_motion_new();
		g_signal_connect(controller, "motion", G_CALLBACK (MouseMoveCB), this);
		gtk_widget_add_controller(mGL, controller);

	GtkGesture *press = gtk_gesture_click_new ();
		gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (press), GDK_BUTTON_PRIMARY);
		g_signal_connect (press, "pressed", G_CALLBACK (MouseButtonPressedCB), this);
		g_signal_connect (press, "released", G_CALLBACK (MouseButtonReleasedCB), this);
		gtk_widget_add_controller (mGL, GTK_EVENT_CONTROLLER (press));

	controller = gtk_event_controller_key_new();
		g_signal_connect(controller, "key-pressed", G_CALLBACK (KeyPressedCB), this);
		g_signal_connect(controller, "key-released", G_CALLBACK (KeyReleasedCB), this);
		gtk_widget_add_controller(mWindow, controller);

	gtk_grid_attach(GTK_GRID (mGrid), mGL, 0, 0, 1, 1);
	gtk_widget_show(mWindow);
}

void PlatformInterface_GTK4::Signal_Destroy(GtkApplication* app)
{
	mUsersApplication->SetExit();
}

void PlatformInterface_GTK4::Signal_MouseMove(double x,double y)
{
	mMouse.LastX = (float)x;
	mMouse.LastY = (float)y;
}

void PlatformInterface_GTK4::Signal_MouseButton(double x,double y,bool Pressed)
{
	ElementPtr root = mUsersApplication->GetRootElement();
	assert(root);
	root->TouchEvent(x,y,Pressed);
}

void PlatformInterface_GTK4::Signal_KeyPressed(guint keyval,guint keycode,GdkModifierType state)
{
	ElementPtr root = mUsersApplication->GetRootElement();
	assert(root);

	// Keyboard event...
	root->KeyboardEvent((char)keyval,true);

	if( keyval == GDK_KEY_Escape )
	{
		mUsersApplication->SetExit();
	}
}

void PlatformInterface_GTK4::Signal_KeyReleased(guint keyval,guint keycode,GdkModifierType state)
{
	ElementPtr root = mUsersApplication->GetRootElement();
	assert(root);

	root->KeyboardEvent((char)keyval,false);
}

void Application::MainLoop(Application* pApplication)
{
	PlatformInterface_GTK4 platform(pApplication);
	platform.MainLoop();

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
