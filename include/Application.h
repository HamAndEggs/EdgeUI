#ifndef APPLICATION_H__
#define APPLICATION_H__

#include <assert.h>

#include "Graphics.h"
#include "Element.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    


/**
 * @brief This is the abstract based class of the application.
 * One of the main goes of this code base is to avoid the use of singletons.
 * Having Singletons makes it hard to port to some platforms as well as restricts you to just one display window.
 */
class Application
{
public:
    Application() = default;
    virtual ~Application() = default;

    // You implement these.
    virtual void OnOpen(Graphics* pGraphics) = 0;
    virtual void OnClose() = 0;
    virtual void OnUpdate(){};

    // This is used to fetch the current root UI element.
    // The platform code does not and should not retain this pointer as the app may change it.
    virtual Element* GetRootElement() = 0; 

    // Overload this for application specific custom rendering logic.
    // For most applications this is not required because a custom control will be more suitible.
    virtual void OnFrame(Graphics* pGraphics,const Rectangle& pDisplayRectangle)
    {
        assert(pGraphics);
        Element* root = GetRootElement();
        if( root )
        {
            OnUpdate(); // Tick that app, if it wants it.
            root->Layout(pDisplayRectangle);
            root->Update();
            pGraphics->BeginFrame();
            root->Draw(pGraphics);
            pGraphics->EndFrame();
        }
    }

    // The minimum rate that the OnFrame is called, in milliseconds, if zero, will go as fast as possible.
    // This is very handy for when you don't need high speed rate and so want to save CPU cycles.
    virtual uint32_t GetUpdateInterval()const{return 0;}

    // Used by the platform specific code to know when to exit. 
    bool GetKeepGoing()const{return mKeepGoing;}
    void SetExit(){mKeepGoing = false;}

    // The platform you're running on implements this but you call it in your main function.
    // Will return when the app is done, after OnClose is called. Just delete your object and return.
    static void MainLoop(Application* pApplication);

    virtual int GetEmulatedWidth()const{return 1024;}
    virtual int GetEmulatedHeight()const{return 600;}

private:
    bool mKeepGoing = true;

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////    
}//namespace eui{

#endif // #ifndef APPLICATION_H__
