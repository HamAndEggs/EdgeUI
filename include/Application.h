#ifndef APPLICATION_H__
#define APPLICATION_H__

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

class Graphics;
class Element;

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

    // This is used to fetch the current root UI element.
    // The platform code does not and should not retain this pointer as the app may change it.
    virtual Element* GetRootElement() = 0; 

    // The platform you're running on implements this but you call it in your main function.
    // Will return when the app is done, after OnClose is called. Just delete your object and return.
    static void MainLoop(Application* pApplication);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{

#endif // #ifndef APPLICATION_H__
