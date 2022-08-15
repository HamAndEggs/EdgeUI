#ifndef GRAPHICS_H__
#define GRAPHICS_H__

#include "GraphicsTypes.h"
#include "Rectangle.h"
#include "Point.h"

#include <memory>
#include <map>
#include <functional>

#include <freetype2/ft2build.h> //sudo apt install libfreetype6-dev
#include FT_FREETYPE_H


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

/**
 * @brief The texture formats that I expose and support. Don't want to get too silly here, these are more than enough.
 */
enum struct TextureFormat
{
	FORMAT_RGBA,
	FORMAT_RGB,
	FORMAT_ALPHA    //<! Alpha only, mainly used for font rendering.
};

struct Style;
struct FreeTypeFont;
class GLTexture;
class GLShader;

typedef GLShader* GLShaderPtr;

/**
 * @brief This is the interface definition to a facade this is implemented by the hardware (GPU) renderer chosen, for example GL. 
 */
class Graphics
{
public:

    /**
     * @brief Sets the rotation of the display, rotates clockwise.
     */
    enum DisplayRotation
    {
        ROTATE_FRAME_BUFFER_0,
        ROTATE_FRAME_BUFFER_90,
        ROTATE_FRAME_BUFFER_180,
        ROTATE_FRAME_BUFFER_270,

        ROTATE_FRAME_PORTRAIT,		//!< If the hardware reports a landscape mode (width > height)  will apply a 90 degree rotation
        ROTATE_FRAME_LANDSCAPE,		//!< If the hardware reports a portrait mode (width < height) will apply a 90 degree rotation
    };

    Graphics();
    virtual ~Graphics();
	void InitialiseGL(int pWidth,int pHeight);

	/**
	 * @brief Marks the start of the frame.
	 */
	void BeginFrame();

	/**
	 * @brief Called at the end of the rendering phase. Normally last part line in the while loop.
	 */
	void EndFrame();

    /**
     * @brief Get the display rectangle
     */
    Rectangle GetDisplayRect()const;

    int32_t GetDisplayWidth()const;

    int32_t GetDisplayHeight()const;

	bool GetIsPortrait(){return mDisplayRotation == ROTATE_FRAME_BUFFER_90 || mDisplayRotation == ROTATE_FRAME_BUFFER_270;}

	DisplayRotation GetDisplayRotation()const{return mDisplayRotation;}
	void GetDisplayRotatedXY(float &x,float &y)const;

    /**
     * @brief Builds a set of points that can be used for drawing a rounded rectangle with lines or polygons.
     */
    void GetRoundedRectanglePoints(const Rectangle& pRect,VertXY::Buffer& rBuffer,float pRadius);

    /**
     * @brief Builds a set of points that can be used for drawing a boarder around a rounded rectangle.
     */
    void GetRoundedRectangleBoarderPoints(const Rectangle& pRect,VertXY::Buffer& rBuffer,float pRadius,float pSize);

	void SetDisplayRotation(DisplayRotation pDisplayRotation);

	/**
	 * @brief Sets the flag for the main loop to false and fires the SYSTEM_EVENT_EXIT_REQUEST
	 * You would typically call this from a UI button to quit the app.
	 */
	void SetExitRequest(){mExitRequest = true;};

    uint32_t FontLoad(const std::string& pFontName,int pPixelHeight = 40);
    void FontDelete(uint32_t pFont);
    void FontPrint(uint32_t pFont,float pX,float pY,Colour pColour,const std::string_view& pText);
    void FontPrintf(uint32_t pFont,float pX,float pY,Colour pColour,const char* pFmt,...);
    void FontPrint(uint32_t pFont,const Rectangle& pRect,const Alignment pAlignment,Colour pColour,const std::string_view& pText);
    Rectangle FontGetRect(uint32_t pFont,const std::string_view& pText)const;
	void FontSetMaximumAllowedGlyph(int pMaxSize){mMaximumAllowedGlyph = pMaxSize;} // The default size is 128 per character. Any bigger will throw an exception, this allows you to go bigger, but kiss good by to vram. Really should do something else instead!

	/**
	 * @brief Draws the rectangle based on style.
	 * If texture is set, will fill with texture, otherwise
	 * If background colour set, will fill rectangle.
	 * Will not draw anything if no colour / texture set and no boarder defined.
	 */
    void DrawRectangle(const Rectangle& pRect,const Style& pStyle);

	void DrawTick(const Rectangle& pRect,const Style& pStyle);

    void DrawLine(float pFromX,float pFromY,float pToX,float pToY,Colour pColour,float pWidth = 1);

	void DrawTexture(const Rectangle& pRect,uint32_t pTexture,Colour pColour = COLOUR_WHITE);

	uint32_t TextureLoadPNG(const std::string& pFilename,bool pFiltered = false,bool pGenerateMipmaps = false);

	/**
	 * @brief Create a Texture object with the size passed in and a given name. 
	 * pPixels is either RGB format 24bit or RGBA 32bit format is pHasAlpha is true.
	 * pPixels can be null if you're going to use FillTexture later to set the image data.
	 * But there is a GL gotcha with passing null, if you don't write to ALL the pixels the texture will not work. So if you're texture is always black you may not have filled it all.
	 */
	uint32_t TextureCreate(int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat,bool pFiltered = false,bool pGenerateMipmaps = false);

	/**
	 * @brief Fill a sub rectangle, or the whole texture. Pixels is expected to be a continuous image data. So it's size is WidthxHeight of the region being updated.
	 * Pixels must be in the format that the texture was originally created with.
	 */
	void TextureFill(uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat = TextureFormat::FORMAT_RGB,bool pGenerateMips = false);

	/**
	 * @brief Delete the texture, will throw an exception is texture not found.
	 * All textures are deleted when the GLES context is torn down so you only need to use this if you need to reclaim some memory.
	 */
	void TextureDelete(uint32_t pTexture);

	/**
	 * @brief Gets the width of the texture. Not recommended that this is called 1000's of times in a frame as it has to search a std::map for the object.
	 */
	int TextureGetWidth(uint32_t pTexture)const;


	/**
	 * @brief Gets the height of the texture. Not recommended that this is called 1000's of times in a frame as it has to search a std::map for the object.
	 */
	int TextureGetHeight(uint32_t pTexture)const;

	/**
	 * @brief Get the diagnostics texture for use to help with finding issues.
	 */
	uint32_t TextureGetDiagnostics()const{return mDiagnostics.texture;}

private:
    bool mExitRequest = false;
	DisplayRotation mDisplayRotation = ROTATE_FRAME_BUFFER_0;

	// mPhysical is the actual width / height of the display, we maybe applying a rotation. Well tell the app the size using mReported.
	struct
	{
		int Width = 0;
		int Height = 0;
	}mPhysical,mReported;

	struct
	{//!< Handy set of internal work buffers used when rendering so we don't blow the stack or thrash the heap. Easy speed up.
		ScratchBuffer<uint8_t,128,16,512*512*4> scratchRam;// gets used for some temporary texture operations.
		VertXY::Buffer vertices;
		VertXY::Buffer uvs;
	}mWorkBuffers;

	std::unique_ptr<struct PNG_LOADER>mPNG;

	std::map<uint32_t,std::unique_ptr<GLTexture>> mTextures; 	//!< Our textures. I reuse the GL texture index (handle) for my own. A handy value and works well.

	/**
	 * @brief Some data used for diagnostics/
	 */
	struct
	{
		uint32_t texture = 0; //!< A handy texture used in debugging. 16x16 check board.
		uint32_t frameNumber = 0; //!< What frame we're on. incremented in BeginFrame() So first frame will be 1
	}mDiagnostics;

	struct
	{
		GLShaderPtr ColourOnly;
		GLShaderPtr TextureColour;
		GLShaderPtr TextureAlphaOnly;
		GLShaderPtr RectangleBorder;

		GLShaderPtr CurrentShader;
	}mShaders;

	struct
	{
		float projection[4][4];
		float transform[4][4];
		bool transformIsIdentity = false;

		float textureTransform[4][4];
		bool textureTransformIsIdentity = false;
	}mMatrices;

	struct RoundedRectData
	{
		static const int NUM_POINTS_PER_CORNER = 31;
		static const int NUM_QUADRANTS = 4;
		static const int NUM_VERTICES = NUM_POINTS_PER_CORNER * NUM_QUADRANTS;
		static const int NUM_BOARDER_VERTICES = ((NUM_POINTS_PER_CORNER * NUM_QUADRANTS * 2) + 2);
		const float ANGLE_INC = GetRadian() / ((float)(NUM_POINTS_PER_CORNER-1) * NUM_QUADRANTS);

		std::vector<eui::Colour> BoarderRaised,BoarderDepressed,BoarderWhite;
	}mRoundedRect;

	uint32_t mNextFontID = 1;
	int mMaximumAllowedGlyph = 128;
	std::map<uint32_t,std::unique_ptr<FreeTypeFont>> mFreeTypeFonts;

	FT_Library mFreetype = nullptr;

	/**
	 * @brief Sets some common rendering states for a nice starting point.
	 */
	void SetRenderingDefaults();

	void BuildDebugTexture();
	void InitFreeTypeFont();
	void InitRoundedRect();

	/**
	 * @brief Set the Projection for 2D rendering.
	 * This is how we rotate the screen for free.
	 */
    void SetProjection2D();

	/**
	 * @brief Set the transform matrix, to use to identity use SetTransformIdentity.
	 * 
	 * @param pTransform 
	 */
	void SetTransform(const float pTransform[4][4]);

	/**
	 * @brief Set the Transform to an identity
	 * Because most of the time the transform is an identity,
	 * we us a bool to only set it when it needs to be.
	 * We're on low end system, sending 16 floats to the gpu for every render is felt.
	 */
	void SetTransformIdentity();

	void SetTextureTransform(const float pTransform[4][4]);
	void SetTextureTransformIdentity();

	/**
	 * @brief Build the shaders that we need for basic rendering. If you need more copy the code and go multiply :)
	 */
	void BuildShaders();

	void EnableShader(GLShaderPtr pShader);
	void VertexPtr(int pNum_coord, uint32_t pType,const void* pPointer);


};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif //GRAPHICS_H__