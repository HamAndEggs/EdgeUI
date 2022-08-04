

#include "Diagnostics.h"
#include "GLIncludes.h"
#include "Element.h"
#include "Graphics.h"

#include <assert.h>
#include <gtk/gtk.h> // sudo apt install libgtk-4-dev
//#include <gtk/gtkglarea.h>
#include <thread>
static int g_argc;
static const char **g_argv;

#define DESKTOP_EMULATION_WIDTH 1024
#define DESKTOP_EMULATION_HEIGHT 600

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
class Graphics_GTK4 : public Graphics
{
public:
	Graphics_GTK4();
	virtual ~Graphics_GTK4();

	// Gtk signals.
	gboolean Signal_Render(GtkGLArea *area, GdkGLContext *context);
	void Signal_Realize(GtkWidget *widget);
	void Signal_Activate(GtkApplication* app);
	gboolean Signal_TickFrame();
	void Signal_MouseMove(double x,double y);
	void Signal_KeyPressed(guint keyval,guint keycode,GdkModifierType state);
	void Signal_KeyReleased(guint keyval,guint keycode,GdkModifierType state);

	virtual void SetUpdateFrequency(uint32_t pMilliseconds);

private:
	GtkApplication *mApp = nullptr;
	GtkWidget *mWindow = nullptr;
	GtkWidget *mGL = nullptr;
	GtkWidget *mGrid = nullptr;
	guint mTickTimer = 0;

	GtkGLArea* GetArea(){return (GtkGLArea*)mGL;}

	bool mWindowActive = false;
	eui::ElementPtr mMainScreen = nullptr;

	struct
	{
		float LastX = 0;
		float LastY = 0;
	}mMouse;
};

static gboolean RenderCB(GtkGLArea *area, GdkGLContext *context,gpointer user_data)
{
	assert(user_data);
	return ((Graphics_GTK4*)user_data)->Signal_Render(area,context);
}

static void RealizeCB(GtkWidget *widget,gpointer user_data )
{
	assert(user_data);
	((Graphics_GTK4*)user_data)->Signal_Realize(widget);
}

static void ActivateCB(GtkApplication* app, gpointer user_data)
{
	assert(user_data);
	((Graphics_GTK4*)user_data)->Signal_Activate(app);
}

static gboolean TickFrameCB(gpointer user_data)
{
	assert(user_data);
	return ((Graphics_GTK4*)user_data)->Signal_TickFrame();
}

static void MouseMoveCB(GtkEventControllerMotion *controller,double x,double y,gpointer user_data)
{
	assert(user_data);
	((Graphics_GTK4*)user_data)->Signal_MouseMove(x,y);
}

static void KeyPressedCB(GtkEventControllerKey *controller,guint keyval,guint keycode,GdkModifierType state,gpointer user_data)
{
	assert(user_data);
	((Graphics_GTK4*)user_data)->Signal_KeyPressed(keyval,keycode,state);
}

static void KeyReleasedCB(GtkEventControllerKey *controller,guint keyval,guint keycode,GdkModifierType state,gpointer user_data)
{
	assert(user_data);
	((Graphics_GTK4*)user_data)->Signal_KeyReleased(keyval,keycode,state);
}

static Graphics_GTK4* theGraphics = nullptr;
Graphics* Graphics::Get()
{
	if( theGraphics == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("Graphics engine not allocated. Please call Graphics::Open first");
	}
	return theGraphics;
}

Graphics_GTK4::Graphics_GTK4()
{
	theGraphics = this;
	mApp = gtk_application_new ("com.hex-edge.ui", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (mApp, "activate", G_CALLBACK(ActivateCB), this);
	g_application_run (G_APPLICATION (mApp), 0, nullptr);
}

Graphics_GTK4::~Graphics_GTK4()
{
	g_source_remove(mTickTimer);
	delete mMainScreen;

	g_object_unref(mApp);
}

gboolean Graphics_GTK4::Signal_Render(GtkGLArea *area, GdkGLContext *context)
{
	assert( mMainScreen );

	mMainScreen->Update();
	BeginFrame();
	mMainScreen->Draw(this);
	EndFrame();

	return TRUE;
}

void Graphics_GTK4::Signal_Realize(GtkWidget *widget)
{
  // We need to make the context current if we want to
  // call GL API
	gtk_gl_area_make_current(GetArea());
	assert( mMainScreen == nullptr);

	InitialiseGL(DESKTOP_EMULATION_WIDTH,DESKTOP_EMULATION_HEIGHT);
	SetUpdateFrequency(10);
	mMainScreen = eui::Element::AllocateUI(g_argc,g_argv,this);
}

void Graphics_GTK4::Signal_Activate(GtkApplication* app)
{
	mWindow = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (mWindow), "EdgeUI");
	gtk_window_set_default_size (GTK_WINDOW (mWindow), DESKTOP_EMULATION_WIDTH, DESKTOP_EMULATION_HEIGHT);

	mGrid = gtk_grid_new();
	gtk_window_set_child (GTK_WINDOW (mWindow), mGrid);

	mGL = gtk_gl_area_new();
    gtk_widget_set_hexpand(mGL, TRUE);
    gtk_widget_set_vexpand(mGL, TRUE);
	g_object_set(mGL, "use-es", TRUE, NULL);
	g_object_set(mGL, "has-depth-buffer", TRUE, NULL);
	g_object_set(mGL, "has-stencil-buffer", TRUE, NULL);

	g_signal_connect(mGL, "render", G_CALLBACK(RenderCB), this);
	g_signal_connect(mGL, "realize", G_CALLBACK(RealizeCB), this);

	GtkEventController *controller;

	controller = gtk_event_controller_motion_new();
		g_signal_connect(controller, "motion", G_CALLBACK (MouseMoveCB), this);
		gtk_widget_add_controller(mGL, controller);

	controller = gtk_event_controller_key_new();
		g_signal_connect(controller, "key-pressed", G_CALLBACK (KeyPressedCB), this);
		g_signal_connect(controller, "key-released", G_CALLBACK (KeyReleasedCB), this);
		gtk_widget_add_controller(mWindow, controller);

	gtk_grid_attach(GTK_GRID (mGrid), mGL, 0, 0, 1, 1);
	gtk_widget_show(mWindow);
}

gboolean Graphics_GTK4::Signal_TickFrame()
{
	gtk_widget_queue_draw(mGL);
	return TRUE;
}

void Graphics_GTK4::Signal_MouseMove(double x,double y)
{
	mMouse.LastX = (float)x;
	mMouse.LastY = (float)y;
}

void Graphics_GTK4::Signal_KeyPressed(guint keyval,guint keycode,GdkModifierType state)
{
	if( state&GDK_BUTTON1_MASK )
	{
		mMainScreen->TouchEvent(mMouse.LastX,mMouse.LastY,true);
	}
	else
	{
		// Keyboard event...
		mMainScreen->KeyboardEvent((char)keyval,true);
	}
}

void Graphics_GTK4::Signal_KeyReleased(guint keyval,guint keycode,GdkModifierType state)
{
	if( state&GDK_BUTTON1_MASK )
	{
		mMainScreen->TouchEvent(mMouse.LastX,mMouse.LastY,false);
	}
	else
	{
		// Keyboard event...
		mMainScreen->KeyboardEvent((char)keyval,false);
	}
}


void Graphics_GTK4::SetUpdateFrequency(uint32_t pMilliseconds)
{
	if( mTickTimer > 0 )
	{
		g_source_remove(mTickTimer);
	}
	mTickTimer = g_timeout_add(pMilliseconds,TickFrameCB,this);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

int main(const int argc,const char *argv[])
{
	g_argc = argc;
	g_argv = argv;

//    // Use dependency injection to pass events onto the controls.
//    // This means that we don't need a circular header dependency that can make it hard to port code.
//    // I do not want graphics.h including element.h as element.h already includes graphics.h
//    auto touchEventHandler = [mainScreen](int32_t pX,int32_t pY,bool pTouched)
//    {
//        return mainScreen->TouchEvent(pX,pY,pTouched);
//    };




//    while( graphics->ProcessSystemEvents(touchEventHandler) )
//    {
//
//        // Check again in a second. Not doing big wait here as I need to be able to quit in a timely fashion.
//        // Also OS could correct display. But one second means system not pegged 100% rendering as fast as possible.
//        sleep(1);
//    }

	eui::theGraphics = new eui::Graphics_GTK4;
	delete eui::theGraphics;
	eui::theGraphics = nullptr;

    return EXIT_SUCCESS;
}
