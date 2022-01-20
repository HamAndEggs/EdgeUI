#ifndef GRAPHICS_GL_H__
#define GRAPHICS_GL_H__

#include "Graphics.h"
#include "Style.h"
#include "ScratchBuffer.h"
#include "FreeTypeFont.h"

#include "GLIncludes.h"
#include "GLShader.h"
#include "GLTexture.h"

#ifdef PLATFORM_X11_GL
    #include "PlatformInterface_X11.h"
#endif


#include <memory>
#include <map>


namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief This is the interface definition to a facade this is implemented by the hardware (GPU) renderer chosen, for example GL. 
 */
class Graphics_GL : public Graphics
{
public:

    Graphics_GL(DisplayRotation pDisplayRotation);
    virtual ~Graphics_GL();

    virtual Rectangle GetDisplayRect()const;
    virtual int32_t GetDisplayWidth()const;
    virtual int32_t GetDisplayHeight()const;

	virtual void BeginFrame();
	virtual void EndFrame();
    virtual bool ProcessSystemEvents(EventTouchScreen mTouchEvent);

    virtual void GetFontRect(uint32_t pFont,Rectangle& rRect);
    virtual void DrawFont(const Style& pStyle,int32_t pX,int32_t pY,const std::string& pText);


    virtual void DrawRectangle(const Rectangle& pRect,const Style& pStyle);
    virtual void DrawLine(int pFromX,int pFromY,int pToX,int pToY,Colour pColour,uint32_t pWidth);

private:
	uint32_t mCreateFlags;
	bool mKeepGoing = true;								//!< Set to false by the application requesting to exit or the user doing ctrl + c.

	// mPhysical is the atchal width / height of the display, we maybe applying a rotation. Well tell the app the size using mReported.
	struct
	{
		int Width = 0;
		int Height = 0;
	}mPhysical,mReported;

	struct
	{//!< Handy set of internal work buffers used when rendering so we don't blow the stack or thrash the heap. Easy speed up.
		ScratchBuffer<uint8_t,128,16,512*512*4> scratchRam;// gets used for some temporary texture operations.
		VertInt16XY::Buffer verticesInt16XY;
		VertInt32XY::Buffer verticesInt32XY;
		VertFloatXY::Buffer verticesFloatXY;

		Vert2DShortScratchBuffer vertices2DShort;
		Vert2DShortScratchBuffer uvShort;
	}mWorkBuffers; 

	std::unique_ptr<PlatformInterface> mPlatform;				//!< This is all the data needed to drive the rendering platform that this code sits on and used to render with.
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
		GLShaderPtr ColourOnly2D;
		GLShaderPtr TextureColour2D;
		GLShaderPtr TextureAlphaOnly2D;
		GLShaderPtr SpriteShader2D;
		GLShaderPtr QuadBatchShader2D;

		GLShaderPtr ColourOnly3D;
		GLShaderPtr TextureOnly3D;

		GLShaderPtr CurrentShader;
	}mShaders;

	struct
	{
		float projection[4][4];
		float transform[4][4];
	}mMatrices;

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

	/**
	 * @brief Set the Projection for 2D rendering.
	 * This is how we rotate the screen for free.
	 */
    void SetProjection2D();

	void SetTransform(float pTransform[4][4]);

	/**
	 * @brief Build the shaders that we need for basic rendering. If you need more copy the code and go multiply :)
	 */
	void BuildShaders();

	void EnableShader(GLShaderPtr pShader);
	void VertexPtr(int pNum_coord, uint32_t pType,const void* pPointer);

	/**
	 * @brief Create a Texture object with the size passed in and a given name. 
	 * pPixels is either RGB format 24bit or RGBA 32bit format is pHasAlpha is true.
	 * pPixels can be null if you're going to use FillTexture later to set the image data.
	 * But there is a GL gotcha with passing null, if you don't write to ALL the pixels the texture will not work. So if you're texture is always black you may not have filled it all.
	 */
	uint32_t CreateTexture(int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat,bool pFiltered = false,bool pGenerateMipmaps = false);

	/**
	 * @brief Fill a sub rectangle, or the whole texture. Pixels is expected to be a continuous image data. So it's size is WidthxHeight of the region being updated.
	 * Pixels must be in the format that the texture was originally created with.
	 */
	void FillTexture(uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat = TextureFormat::FORMAT_RGB,bool pGenerateMips = false);

	/**
	 * @brief Delete the texture, will throw an exception is texture not found.
	 * All textures are deleted when the GLES context is torn down so you only need to use this if you need to reclaim some memory.
	 */
	void DeleteTexture(uint32_t pTexture);

	/**
	 * @brief Gets the width of the texture. Not recommended that this is called 1000's of times in a frame as it has to search a std::map for the object.
	 */
	int GetTextureWidth(uint32_t pTexture)const;


	/**
	 * @brief Gets the height of the texture. Not recommended that this is called 1000's of times in a frame as it has to search a std::map for the object.
	 */
	int GetTextureHeight(uint32_t pTexture)const;

	/**
	 * @brief Get the diagnostics texture for use to help with finding issues.
	 */
	uint32_t GetDiagnosticsTexture()const{return mDiagnostics.texture;}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //GRAPHICS_GL_H__