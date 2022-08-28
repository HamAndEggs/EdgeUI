
#include "Graphics.h"
#include "GLDiagnostics.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "Style.h"
#include "FreeTypeFont.h"
#include "../TinyPNG.h"

#include <math.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
struct PNG_LOADER
{
	PNG_LOADER():loader(false)
	{

	}
	tinypng::Loader loader;
	std::vector<uint8_t> pixelBuffer;
};

Graphics::Graphics()
{
	mPNG = std::make_unique<PNG_LOADER>();
}

Graphics::~Graphics()
{
	VERBOSE_MESSAGE("GLES destructor called");

	VERBOSE_MESSAGE("On exit the following scratch memory buffers reached the sizes of...");
	VERBOSE_MESSAGE("    mWorkBuffers.vertices " << mWorkBuffers.vertices.MemoryUsed() << " bytes");
	VERBOSE_MESSAGE("    mWorkBuffers.uvs " << mWorkBuffers.uvs.MemoryUsed() << " bytes");

	glBindTexture(GL_TEXTURE_2D,0);
	CHECK_OGL_ERRORS();

	// Kill shaders.
	VERBOSE_MESSAGE("Deleting shaders");

	glUseProgram(0);
	CHECK_OGL_ERRORS();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

	delete mShaders.ColourOnly;
	delete mShaders.TextureColour;
	delete mShaders.TextureAlphaOnly;
	delete mShaders.RectangleBorder;

	// delete all free type fonts.
	mFreeTypeFonts.clear();
	if( mFreetype != nullptr )
	{
		if( FT_Done_FreeType(mFreetype) == FT_Err_Ok )
		{
			mFreetype = nullptr;
			VERBOSE_MESSAGE("Freetype font library deleted");
		}
	}

	// delete all textures.
	for( auto& t : mTextures )
	{
		glDeleteTextures(1,&t.first);
		CHECK_OGL_ERRORS();
	}

	VERBOSE_MESSAGE("All done");
}


Rectangle Graphics::GetDisplayRect()const
{
    return Rectangle(0,0,GetDisplayWidth(),GetDisplayHeight());
}

int32_t Graphics::GetDisplayWidth()const
{
    return mReported.Width;
}

int32_t Graphics::GetDisplayHeight()const
{
    return mReported.Height;
}

void Graphics::GetRoundedRectanglePoints(const Rectangle& pRect,VertXY::Buffer& rBuffer,float pRadius)
{
	VertXY* verts = rBuffer.Restart(mRoundedRect.NUM_VERTICES);

	float A = 0.0f;// This starts the circle at the top, so the first corner is the right top one.
	const float AD = mRoundedRect.ANGLE_INC;

	const float size = std::min(pRect.GetWidth()*pRadius,pRect.GetHeight()*pRadius);
	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++, verts++, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (1.0f - sA) * size;
		const float y = (1.0f - cA) * size;

		verts->x = pRect.right - x;
		verts->y = pRect.top +   y;
	}
	A -= AD;

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ , verts++, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (1.0f - sA) * size;
		const float y = (cA + 1.0f) * size;

		verts->x = pRect.right -  x;
		verts->y = pRect.bottom - y;
	}
	A -= AD;

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ , verts++, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (sA + 1.0f) * size;
		const float y = (cA + 1.0f) * size;

		verts->x = pRect.left +   x;
		verts->y = pRect.bottom - y;
	}
	A -= AD;

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ , verts++, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (sA + 1.0f) * size;
		const float y = (1.0f - cA) * size;

		verts->x = pRect.left + x;
		verts->y = pRect.top +  y;
	}
}

void Graphics::GetRoundedRectangleBoarderPoints(const Rectangle& pRect,VertXY::Buffer& rBuffer,float pRadius,float pThickness)
{
	VertXY* verts = rBuffer.Restart(mRoundedRect.NUM_BOARDER_VERTICES);
	VertXY* first = verts;

	const float outerSize = std::min(pRect.GetWidth()*pRadius,pRect.GetHeight()*pRadius);

	float A = 0.0f;// This starts the circle at the top, so the first corner is the right top one.
	const float AD = GetRadian() / ((float)(mRoundedRect.NUM_POINTS_PER_CORNER-1) * mRoundedRect.NUM_QUADRANTS);

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++, verts += 2, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (1.0f - sA) * outerSize;
		const float y = (1.0f - cA) * outerSize;

		verts[1].x = pRect.right - x;
		verts[1].y = pRect.top +   y;

		verts[0].x = verts[1].x - (sA * pThickness);
		verts[0].y = verts[1].y + (cA * pThickness);
	}
	A -= AD;

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ , verts += 2, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (1.0f - sA) * outerSize;
		const float y = (cA + 1.0f) * outerSize;

		verts[1].x = pRect.right -  x;
		verts[1].y = pRect.bottom - y;

		verts[0].x = verts[1].x - (sA * pThickness);
		verts[0].y = verts[1].y + (cA * pThickness);

	}
	A -= AD;

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ , verts += 2, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (sA + 1.0f) * outerSize;
		const float y = (cA + 1.0f) * outerSize;

		verts[1].x = pRect.left +   x;
		verts[1].y = pRect.bottom - y;

		verts[0].x = verts[1].x - (sA * pThickness);
		verts[0].y = verts[1].y + (cA * pThickness);
	}
	A -= AD;

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ , verts += 2, A += AD )
	{
		const float sA = sin(A);
		const float cA = cos(A);

		const float x = (sA + 1.0f) * outerSize;
		const float y = (1.0f - cA) * outerSize;

		verts[1].x = pRect.left + x;
		verts[1].y = pRect.top +  y;

		verts[0].x = verts[1].x - (sA * pThickness);
		verts[0].y = verts[1].y + (cA * pThickness);
	}

	verts[0] = first[0];
	verts[1] = first[1];
}

void Graphics::GetDisplayRotatedXY(float &x,float &y)const
{
	//GetDisplayWidth(),GetDisplayHeight()
	if( mDisplayRotation == Graphics::ROTATE_FRAME_BUFFER_90 )
	{
		std::swap(x,y);
		y = GetDisplayHeight() - y;
	}
	else if( mDisplayRotation == Graphics::ROTATE_FRAME_BUFFER_180 )
	{
		x = GetDisplayWidth() - x;
		y = GetDisplayHeight() - y;
	}
	else if( mDisplayRotation == Graphics::ROTATE_FRAME_BUFFER_270 )
	{
		std::swap(x,y);
		x = GetDisplayWidth() - x;
	}
	else// what ever the HW is with no rotation.
	{
	}

}

void Graphics::SetDisplayRotation(DisplayRotation pDisplayRotation)
{
	// They want portrait, if hardware display is landscape, rotate 90, else don't.
	if( pDisplayRotation == ROTATE_FRAME_PORTRAIT )
	{
		mDisplayRotation = ROTATE_FRAME_BUFFER_0;// Assume is portrait by default.
		if( mPhysical.Width > mPhysical.Height )
		{
			mDisplayRotation = ROTATE_FRAME_BUFFER_90;// hardware is landscape
		}
	}
	else if( pDisplayRotation == ROTATE_FRAME_LANDSCAPE )
	{
		mDisplayRotation = ROTATE_FRAME_BUFFER_0; // Assume is landscape by default.
		if( mPhysical.Width < mPhysical.Height )
		{
			mDisplayRotation = ROTATE_FRAME_BUFFER_90;// hardware is portrait
		}
	}
	else
	{
		// Ok, they are forcing rotation.
		mDisplayRotation = pDisplayRotation;
	}

	if( GetIsPortrait() )
	{
		mReported.Width = mPhysical.Height;
		mReported.Height = mPhysical.Width;
	}
	else
	{
		mReported.Width = mPhysical.Width;
		mReported.Height = mPhysical.Height;
	}

    SetProjection2D();

}

uint32_t Graphics::FontLoad(const std::string& pFontName,int pPixelHeight)
{
	FT_Face loadedFace;
	if( FT_New_Face(mFreetype,pFontName.c_str(),0,&loadedFace) != 0 )
	{
		std::cerr << "Failed to load true type font " << pFontName << "\n";
		THROW_MEANINGFUL_EXCEPTION("Failed to load true type font " + pFontName);
	}

	const uint32_t fontID = mNextFontID++;
	mFreeTypeFonts[fontID] = std::make_unique<FreeTypeFont>(loadedFace,pPixelHeight);

	// Now we need to prepare the texture cache.
	auto& font = mFreeTypeFonts.at(fontID);
	font->BuildTexture(
		mMaximumAllowedGlyph,
		[this](int pWidth,int pHeight)
		{
			// Because the glyph rending to texture does not fill the whole texture the GL texture will not be created.
			// Do I have to make a big memory buffer, fill it with zero, then free the memory.
			auto zeroMemory = std::make_unique<uint8_t[]>(pWidth * pHeight);
			memset(zeroMemory.get(),0,pWidth * pHeight);

			return TextureCreate(pWidth,pHeight,zeroMemory.get(),TextureFormat::FORMAT_ALPHA);			
		},
		[this](uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels)
		{
			TextureFill(pTexture,pX,pY,pWidth,pHeight,pPixels,TextureFormat::FORMAT_ALPHA);
		}
	);

	VERBOSE_MESSAGE("Free type font loaded: " << pFontName << " with internal ID of " << fontID << " Using texture " << font->mTexture);

	return fontID;
}

void Graphics::FontDelete(uint32_t pFont)
{
	mFreeTypeFonts.erase(pFont);
}


void Graphics::FontPrint(uint32_t pFont,float pX,float pY,Colour pColour,const std::string_view& pText)
{
	auto& font = mFreeTypeFonts.at(pFont);

	mWorkBuffers.vertices.Restart();
	mWorkBuffers.uvs.Restart();

	// Get where the uvs will be written too.
	const char* ptr = pText.data();

	font->BuildQuads(ptr,pX,pY,mWorkBuffers.vertices,mWorkBuffers.uvs);

	assert(font->mTexture);
	EnableShader(mShaders.TextureAlphaOnly);

	mShaders.CurrentShader->SetTexture(font->mTexture);
	mShaders.CurrentShader->SetGlobalColour(pColour);

	// how many?
	const int numVerts = mWorkBuffers.vertices.Used();

	glVertexAttribPointer(
				(GLuint)StreamIndex::TEXCOORD,
				2,
				GL_FLOAT,
				GL_TRUE,
				8,mWorkBuffers.uvs.Data());

	VertexPtr(2,GL_FLOAT,mWorkBuffers.vertices.Data());
	glDrawArrays(GL_TRIANGLES,0,numVerts);
	CHECK_OGL_ERRORS();
}

void Graphics::FontPrintf(uint32_t pFont,float pX,float pY,Colour pColour,const char* pFmt,...)
{
	char buf[1024];
	va_list args;
	va_start(args, pFmt);
	vsnprintf(buf, sizeof(buf), pFmt, args);
	va_end(args);
	FontPrint(pFont,pX,pY,pColour, buf);
}

void Graphics::FontPrint(uint32_t pFont,const Rectangle& pRect,const Alignment pAlignment,Colour pColour,const std::string_view& pText)
{
	// First we need to get the rect of the text to be rendered.
	const Rectangle fontRect = FontGetRect(pFont,pText);

	const Alignment AX = GET_X_ALIGNMENT(pAlignment);
	const Alignment AY = GET_Y_ALIGNMENT(pAlignment);

	float X = pRect.left - fontRect.left;
	if( AX == ALIGN_CENTER )
	{
		X += (pRect.GetWidth() * 0.5f) - (fontRect.GetWidth() * 0.5f);
	}
	else if( AX == ALIGN_MAX_EDGE )
	{
		X += pRect.GetWidth() - fontRect.GetWidth();
	}

	float Y = pRect.top - fontRect.top;
	if( AY == ALIGN_CENTER )
	{
		Y += (pRect.GetHeight() * 0.5f) - (fontRect.GetHeight() * 0.5f);
	}
	else if( AY == ALIGN_MAX_EDGE )
	{
		Y += pRect.GetHeight() - fontRect.GetHeight();
	}

	FontPrint(pFont,X,Y,pColour,pText);
}

Rectangle Graphics::FontGetRect(uint32_t pFont,const std::string_view& pText)const
{
	auto& font = mFreeTypeFonts.at(pFont);
	return font->GetRect(pText);
}

void Graphics::DrawRectangle(const Rectangle& pRect,const Style& pStyle)
{

	if( pStyle.mTexture )
	{
		// Later this will respect the rounded corners and board of the style.
		if( pStyle.mRadius <= 0.0f )
		{
			DrawTexture(pRect,pStyle.mTexture,pStyle.mBackground);
		}
		else
		{
			EnableShader(mShaders.TextureColour);
			mShaders.CurrentShader->SetGlobalColour(pStyle.mBackground);
			mShaders.CurrentShader->SetTexture(pStyle.mTexture);
			SetTextureTransformIdentity();

			GetRoundedRectanglePoints(pRect,mWorkBuffers.vertices,pStyle.mRadius);
			const Rectangle uv = {0,0,1,1};
			GetRoundedRectanglePoints(uv,mWorkBuffers.uvs,pStyle.mRadius);

			VertexPtr(2,GL_FLOAT,mWorkBuffers.vertices.Data());

			glVertexAttribPointer(
				(GLuint)StreamIndex::TEXCOORD,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,mWorkBuffers.uvs.Data());

			glDrawArrays(GL_TRIANGLE_FAN,0,mWorkBuffers.vertices.Used());
			CHECK_OGL_ERRORS();

		}
	}
	else if( pStyle.mBackground != COLOUR_NONE )
	{
		if( pStyle.mRadius )
		{
			GetRoundedRectanglePoints(pRect,mWorkBuffers.vertices,pStyle.mRadius);

			const int numPoints = mWorkBuffers.vertices.Used();
			const VertXY* points = mWorkBuffers.vertices.Data();

			EnableShader(mShaders.ColourOnly);
			mShaders.CurrentShader->SetGlobalColour(pStyle.mBackground);

			VertexPtr(2,GL_FLOAT,points);
			glDrawArrays(GL_TRIANGLE_FAN,0,numPoints);
			CHECK_OGL_ERRORS();
		}
		else
		{
			float quad[8];    
			pRect.GetQuad(quad);

			EnableShader(mShaders.ColourOnly);
			mShaders.CurrentShader->SetGlobalColour(pStyle.mBackground);
			VertexPtr(2,GL_FLOAT,quad);
			glDrawArrays(GL_TRIANGLE_FAN,0,4);
			CHECK_OGL_ERRORS();
		}
	}

	// Now see if we need to draw a boarder.
	if( pStyle.mBorder != COLOUR_NONE && pStyle.mThickness > 0 )
	{
		if( pStyle.mRadius )
		{
			GLenum primType = GL_LINE_LOOP;
			if( pStyle.mThickness == 1 )
			{
				GetRoundedRectanglePoints(pRect,mWorkBuffers.vertices,pStyle.mRadius);
			}
			else
			{
				GetRoundedRectangleBoarderPoints(pRect,mWorkBuffers.vertices,pStyle.mRadius,pStyle.mThickness);
				primType = GL_TRIANGLE_STRIP;
			}

			const int numPoints = mWorkBuffers.vertices.Used();
			const VertXY* points = mWorkBuffers.vertices.Data();

			EnableShader(mShaders.RectangleBorder);
			mShaders.CurrentShader->SetGlobalColour(pStyle.mBorder);
			VertexPtr(2,GL_FLOAT,points);

			const void* colours = mRoundedRect.BoarderWhite.data();
			switch (pStyle.mBoarderStyle)
			{
			case Style::BS_SOLID:
				break;
			
			case Style::BS_RAISED:
				colours = mRoundedRect.BoarderRaised.data();
				break;

			case Style::BS_DEPRESSED:
				colours = mRoundedRect.BoarderDepressed.data();
				break;
			}

			glVertexAttribPointer(
						(GLuint)StreamIndex::COLOUR,
						4,
						GL_UNSIGNED_BYTE,
						GL_TRUE,
						0,colours);
			CHECK_OGL_ERRORS();

			glDrawArrays(primType,0,numPoints);
			CHECK_OGL_ERRORS();

		}
		else
		{
			float quad[8];    
			pRect.GetQuad(quad);

			EnableShader(mShaders.ColourOnly);
			mShaders.CurrentShader->SetGlobalColour(pStyle.mBackground);
			VertexPtr(2,GL_FLOAT,quad);

			if( pStyle.mThickness == 1 )
			{
				mShaders.CurrentShader->SetGlobalColour(pStyle.mBorder);
				glDrawArrays(GL_LINE_LOOP,0,4);
				CHECK_OGL_ERRORS();
			}
			else
			{

			}
		}
	}

	if( false )
	{
		float quad[8];    
		pRect.GetQuad(quad);

		EnableShader(mShaders.ColourOnly);
		mShaders.CurrentShader->SetGlobalColour(MakeColour(255,0,255));
		VertexPtr(2,GL_FLOAT,quad);
		glDrawArrays(GL_LINE_LOOP,0,4);
		CHECK_OGL_ERRORS();
	}
}

void Graphics::DrawTick(const Rectangle& pRect,const Style& pStyle)
{
	Rectangle r = pRect.GetScaled(0.6f);
	const float step = r.GetMinSize() * 0.25;

	DrawLine(
		r.left,
		r.GetCenterY(),
		r.left + step,
		r.GetCenterY() + step,
		pStyle.mForeground,
		pStyle.mThickness
	);

	DrawLine(
		r.left + step,
		r.GetCenterY() + step,
		r.right,
		r.top,
		pStyle.mForeground,
		pStyle.mThickness
	);
}

void Graphics::DrawTexture(const Rectangle& pRect,uint32_t pTexture,Colour pColour)
{
	const float uv[8] = {0,0,1,0,1,1,0,1};
	float quad[8];
	
	pRect.GetQuad(quad);

	if( pColour == COLOUR_NONE )
	{
		pColour = COLOUR_WHITE;
	}

	EnableShader(mShaders.TextureColour);

	mShaders.CurrentShader->SetGlobalColour(pColour);
	mShaders.CurrentShader->SetTexture(pTexture);

	glVertexAttribPointer(
				(GLuint)StreamIndex::TEXCOORD,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,uv);
	CHECK_OGL_ERRORS();

	VertexPtr(2,GL_FLOAT,quad);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	CHECK_OGL_ERRORS();	
}

void Graphics::DrawLine(float pFromX,float pFromY,float pToX,float pToY,Colour pColour,float pWidth)
{
	if( pWidth < 2 )
	{
		const float quad[4] = {pFromX,pFromY,pToX,pToY};

		EnableShader(mShaders.ColourOnly);
		mShaders.CurrentShader->SetGlobalColour(pColour);

		VertexPtr(2,GL_FLOAT,quad);
		glDrawArrays(GL_LINES,0,2);
		CHECK_OGL_ERRORS();
	}
	else
	{
		pWidth /= 2;
		VertXY p[6];

		if( pFromY < pToY )
		{
			std::swap(pFromY,pToY);
			std::swap(pFromX,pToX);
		}

		if( pFromX < pToX )
		{
			p[0].x = pToX - pWidth;
			p[0].y = pToY - pWidth;

			p[1].x = pToX + pWidth;
			p[1].y = pToY - pWidth;

			p[2].x = pToX + pWidth;
			p[2].y = pToY + pWidth;

			p[3].x = pFromX + pWidth;
			p[3].y = pFromY + pWidth;

			p[4].x = pFromX - pWidth;
			p[4].y = pFromY + pWidth;

			p[5].x = pFromX - pWidth;
			p[5].y = pFromY - pWidth;
		}
		else
		{
			p[0].x = pFromX + pWidth;
			p[0].y = pFromY - pWidth;

			p[1].x = pFromX + pWidth;
			p[1].y = pFromY + pWidth;

			p[2].x = pFromX - pWidth;
			p[2].y = pFromY + pWidth;

			p[3].x = pToX - pWidth;
			p[3].y = pToY + pWidth;

			p[4].x = pToX - pWidth;
			p[4].y = pToY - pWidth;

			p[5].x = pToX + pWidth;
			p[5].y = pToY - pWidth;			
		}

		EnableShader(mShaders.ColourOnly);
		mShaders.CurrentShader->SetGlobalColour(pColour);

		VertexPtr(2,GL_FLOAT,p);
		glDrawArrays(GL_TRIANGLE_FAN,0,6);
		CHECK_OGL_ERRORS();
	}
}

void Graphics::DrawRoundedLine(float pFromX,float pFromY,float pToX,float pToY,Colour pColour,float pWidth)
{
//	VertXY* verts = rBuffer.Restart(mRoundedRect.NUM_VERTICES);
//
//	float A = 0.0f;// This starts the circle at the top, so the first corner is the right top one.
//	const float AD = mRoundedRect.ANGLE_INC;
//
//	const float size = pWidth;
//	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++, verts++, A += AD )
//	{
//		const float sA = sin(A);
//		const float cA = cos(A);
//
//		const float x = (1.0f - sA) * size;
//		const float y = (1.0f - cA) * size;
//
//		verts->x = pRect.right - x;
//		verts->y = pRect.top +   y;
//	}
//	A -= AD;
//
}

uint32_t Graphics::TextureLoadPNG(const std::string& pFilename,bool pFiltered,bool pGenerateMipmaps)
{
    if( mPNG->loader.LoadFromFile(pFilename) )
    {
		if( mPNG->loader.GetHasAlpha() )
        {
            mPNG->loader.GetRGBA(mPNG->pixelBuffer);
            return TextureCreate(mPNG->loader.GetWidth(),mPNG->loader.GetHeight(),mPNG->pixelBuffer.data(),TextureFormat::FORMAT_RGBA,pFiltered,pGenerateMipmaps);
        }
        else
        {
            mPNG->loader.GetRGB(mPNG->pixelBuffer);
            return TextureCreate(mPNG->loader.GetWidth(),mPNG->loader.GetHeight(),mPNG->pixelBuffer.data(),TextureFormat::FORMAT_RGB,pFiltered,pGenerateMipmaps);
        }
    }
	else
	{
		std::clog << "Failed to load PNG: " << pFilename << "\n";
	}

	return TextureGetDiagnostics();
}

uint32_t Graphics::TextureCreate(int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat,bool pFiltered,bool pGenerateMipmaps)
{
	const GLint format = TextureFormatToGLFormat(pFormat);
	if( format == GL_INVALID_ENUM )
	{
		THROW_MEANINGFUL_EXCEPTION("TextureCreate passed an unknown texture format, I can not continue.");
	}

	GLuint newTexture;
	glGenTextures(1,&newTexture);
	CHECK_OGL_ERRORS();
	if( newTexture == 0 )
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to create texture, glGenTextures returned zero");
	}

	if( mTextures.find(newTexture) != mTextures.end() )
	{
		THROW_MEANINGFUL_EXCEPTION("Bug found in GLES code, glGenTextures returned an index that we already know about.");
	}

	mTextures[newTexture] = std::make_unique<GLTexture>(pFormat,pWidth,pHeight);

	glBindTexture(GL_TEXTURE_2D,newTexture);
	CHECK_OGL_ERRORS();

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		pWidth,
		pHeight,
		0,
		format,
		GL_UNSIGNED_BYTE,
		pPixels);

	CHECK_OGL_ERRORS();

	// Unlike GLES 1.1 this is called after texture creation, in GLES 1.1 you say that you want glTexImage2D to make the mips.
	// Don't call if we don't yet have pixels. Will be called when you fill the texture.
	if( pPixels != nullptr )
	{
		if( pGenerateMipmaps )
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			CHECK_OGL_ERRORS();
			if( pFiltered )
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
		else
		{
			if( pFiltered )
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
	}

	// If it's alpha only we need to set the texture swizzle for RGB to one.
	// Leaving in for when I add GLES 3.0 support. But for now, grump, need two textures.
	// GL_TEXTURE_SWIZZLE_R not supported in GLES 2.0
	/*
	if( format == GL_ALPHA )
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_SWIZZLE_R,GL_ONE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_SWIZZLE_G,GL_ONE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_SWIZZLE_B,GL_ONE);
	}
	*/

	CHECK_OGL_ERRORS();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D,0);//Because we had to change it to setup the texture! Stupid GL!
	CHECK_OGL_ERRORS();

	VERBOSE_MESSAGE("Texture " << newTexture << " created, " << pWidth << "x" << pHeight << " Format = " << TextureFormatToString(pFormat) << " Mipmaps = " << (pGenerateMipmaps?"true":"false") << " Filtered = " << (pFiltered?"true":"false"));


	return newTexture;
}

void Graphics::TextureFill(uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat,bool pGenerateMips)
{
	glBindTexture(GL_TEXTURE_2D,pTexture);

	const GLint format = TextureFormatToGLFormat(pFormat);
	if( format == GL_INVALID_ENUM )
	{
		THROW_MEANINGFUL_EXCEPTION("TextureFill passed an unknown texture format, I can not continue.");
	}

	glTexSubImage2D(GL_TEXTURE_2D,
		0,
		pX,pY,
		pWidth,pHeight,
		format,GL_UNSIGNED_BYTE,
		pPixels);

	if( pGenerateMips )
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D,0);//Because we had to change it to setup the texture! Stupid GL!
}

void Graphics::TextureDelete(uint32_t pTexture)
{
	if( pTexture == mDiagnostics.texture )
	{
		THROW_MEANINGFUL_EXCEPTION("An attempt was made to delete the debug texture, do not do this!");
	}

	if( mTextures.find(pTexture) != mTextures.end() )
	{
		glDeleteTextures(1,(GLuint*)&pTexture);
		mTextures.erase(pTexture);
	}
}

int Graphics::TextureGetWidth(uint32_t pTexture)const
{
	return mTextures.at(pTexture)->mWidth;
}

int Graphics::TextureGetHeight(uint32_t pTexture)const
{
	return mTextures.at(pTexture)->mHeight;
}

void Graphics::InitialiseGL(int pWidth,int pHeight)
{
	mPhysical.Width = pWidth;
	mPhysical.Height = pHeight;

	SetDisplayRotation(ROTATE_FRAME_LANDSCAPE);

	VERBOSE_MESSAGE("Physical display resolution is " << mPhysical.Width << "x" << mPhysical.Height );

	SetRenderingDefaults();
	BuildShaders();
	BuildDebugTexture();
	InitFreeTypeFont();
	InitRoundedRect();

	VERBOSE_MESSAGE("GLES Ready");
}

void Graphics::BeginFrame()
{
	mDiagnostics.frameNumber++;

	const float Identity[4][4] ={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

	// Force identity transform matrix.
	mMatrices.transformIsIdentity = true;
	memcpy(mMatrices.transform,Identity,sizeof(float) * 4 * 4);

	mMatrices.textureTransformIsIdentity = true;
	memcpy(mMatrices.textureTransform,Identity,sizeof(float) * 4 * 4);

	// Reset some items so that we have a working render setup to begin the frame with.
	// This is done so that I don't have to have a load of if statements to deal with first frame. Also makes life simpler for the more minimal applications.
	EnableShader(mShaders.ColourOnly);
}

void Graphics::EndFrame()
{
	glFlush();// This makes sure the display is fully up to date before we allow them to interact with any kind of UI. This is the specified use of this function.
}

void Graphics::SetRenderingDefaults()
{
	glViewport(0, 0, (GLsizei)mPhysical.Width, (GLsizei)mPhysical.Height);
	glDepthRangef(0.0f,1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	// Always cull, because why not. :) Make code paths simple.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// I have alpha blend on all the time. Makes life easy. No point in complicating the code for speed, going for simple implementation not fastest!
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnableVertexAttribArray((int)StreamIndex::VERTEX);//Always on

    SetProjection2D();
	SetTransformIdentity();

	CHECK_OGL_ERRORS();
}

void Graphics::BuildDebugTexture()
{
	VERBOSE_MESSAGE("Creating mDiagnostics.texture");
	uint8_t pixels[16*16*4];
	uint8_t* dst = pixels;
	for( int y = 0 ; y < 16 ; y++ )
	{
		for( int x = 0 ; x < 16 ; x++ )
		{
			if( (x&1) == (y&1) )
			{
				dst[0] = 255;dst[1] = 0;dst[2] = 255;dst[3] = 255;
			}
			else
			{
				dst[0] = 0;dst[1] = 255;dst[2] = 0;dst[3] = 255;
			}
			dst+=4;
		}
	}
	// Put some dots in so I know which way is up and if it's flipped.
	pixels[(16*4) + (7*4) + 0] = 0xff;
	pixels[(16*4) + (7*4) + 1] = 0x0;
	pixels[(16*4) + (7*4) + 2] = 0x0;
	pixels[(16*4) + (8*4) + 0] = 0xff;
	pixels[(16*4) + (8*4) + 1] = 0x0;
	pixels[(16*4) + (8*4) + 2] = 0x0;

	pixels[(16*4*7) + (14*4) + 0] = 0x00;
	pixels[(16*4*7) + (14*4) + 1] = 0x0;
	pixels[(16*4*7) + (14*4) + 2] = 0xff;
	pixels[(16*4*8) + (14*4) + 0] = 0x00;
	pixels[(16*4*8) + (14*4) + 1] = 0x0;
	pixels[(16*4*8) + (14*4) + 2] = 0xff;

	mDiagnostics.texture = TextureCreate(16,16,pixels,TextureFormat::FORMAT_RGBA);
}

void Graphics::InitFreeTypeFont()
{
	if( FT_Init_FreeType(&mFreetype) == 0 )
	{
		VERBOSE_MESSAGE("Freetype font library created");
	}
	else
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to init free type font library");
	}
}

void Graphics::InitRoundedRect()
{
	auto PUSH_LIGHT = [this]()
	{
		mRoundedRect.BoarderWhite.push_back(eui::COLOUR_WHITE);
		mRoundedRect.BoarderWhite.push_back(eui::COLOUR_WHITE);

		mRoundedRect.BoarderRaised.push_back(eui::COLOUR_WHITE);
		mRoundedRect.BoarderRaised.push_back(eui::COLOUR_LIGHT_GREY);

		mRoundedRect.BoarderDepressed.push_back(eui::COLOUR_DARK_GREY);
		mRoundedRect.BoarderDepressed.push_back(eui::COLOUR_BLACK);
	};

	auto PUSH_DARK = [this]()
	{
		mRoundedRect.BoarderWhite.push_back(eui::COLOUR_WHITE);
		mRoundedRect.BoarderWhite.push_back(eui::COLOUR_WHITE);

		mRoundedRect.BoarderRaised.push_back(eui::COLOUR_DARK_GREY);
		mRoundedRect.BoarderRaised.push_back(eui::COLOUR_BLACK);

		mRoundedRect.BoarderDepressed.push_back(eui::COLOUR_WHITE);
		mRoundedRect.BoarderDepressed.push_back(eui::COLOUR_LIGHT_GREY);		
	};


// Top right
	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER/2 ; n++ )
	{
		PUSH_LIGHT();
	}
	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER/2 ; n++ )
	{
		PUSH_DARK();
	}
// Bottom right
	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ )
	{
		PUSH_DARK();
	}
// Bottom left
	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER/2 ; n++ )
	{
		PUSH_DARK();
	}

	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER/2 ; n++ )
	{
		PUSH_LIGHT();
	}
// Top left
	for( int n = 0 ; n < mRoundedRect.NUM_POINTS_PER_CORNER ; n++ )
	{
		PUSH_LIGHT();
	}

	PUSH_LIGHT();
	PUSH_LIGHT();
	PUSH_LIGHT();
}

void Graphics::SetProjection2D()
{
	// Setup 2D frustum
	memset(mMatrices.projection,0,sizeof(mMatrices.projection));
	mMatrices.projection[3][3] = 1;

	if( mDisplayRotation == ROTATE_FRAME_BUFFER_90 )
	{
		mMatrices.projection[0][1] = -2.0f / (float)mPhysical.Height;
		mMatrices.projection[1][0] = -2.0f / (float)mPhysical.Width;
				
		mMatrices.projection[3][0] = 1;
		mMatrices.projection[3][1] = 1;
	}
	else if( mDisplayRotation == ROTATE_FRAME_BUFFER_180 )
	{
		mMatrices.projection[0][0] = -2.0f / (float)mPhysical.Width;
		mMatrices.projection[1][1] = 2.0f / (float)mPhysical.Height;
				
		mMatrices.projection[3][0] = 1;
		mMatrices.projection[3][1] = -1;
	}
	else if( mDisplayRotation == ROTATE_FRAME_BUFFER_270 )
	{
		mMatrices.projection[0][1] = 2.0f / (float)mPhysical.Height;
		mMatrices.projection[1][0] = 2.0f / (float)mPhysical.Width;
				
		mMatrices.projection[3][0] = -1;
		mMatrices.projection[3][1] = -1;
	}
	else// what ever the HW is with no rotation.
	{
		mMatrices.projection[0][0] = 2.0f / (float)mPhysical.Width;
		mMatrices.projection[1][1] = -2.0f / (float)mPhysical.Height;
		mMatrices.projection[3][0] = -1;
		mMatrices.projection[3][1] = 1;
	}

	// No Depth buffer in 2D
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(false);    
}

void Graphics::SetTransform(const float pTransform[4][4])
{
	assert(mShaders.CurrentShader);
	memcpy(mMatrices.transform,pTransform,sizeof(float) * 4 * 4);
	mShaders.CurrentShader->SetTransform(mMatrices.transform);
	mMatrices.transformIsIdentity = false;
}

void Graphics::SetTransformIdentity()
{
	if( mMatrices.transformIsIdentity == false )
	{
		const float Identity[4][4] = 
		{
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		};
		memcpy(mMatrices.transform,Identity,sizeof(float) * 4 * 4);
		if( mShaders.CurrentShader )
		{
			mShaders.CurrentShader->SetTransform(mMatrices.transform);
		}
		mMatrices.transformIsIdentity = true;
	}
}

void Graphics::SetTextureTransform(const float pTransform[4][4])
{
	assert(mShaders.CurrentShader);
	memcpy(mMatrices.textureTransform,pTransform,sizeof(float) * 4 * 4);
	mShaders.CurrentShader->SetTextureTransform(mMatrices.textureTransform);
	mMatrices.textureTransformIsIdentity = false;
}

void Graphics::SetTextureTransformIdentity()
{
	assert(mShaders.CurrentShader);
	if( mMatrices.textureTransformIsIdentity == false )
	{
		const float Identity[4][4] = 
		{
			{1,0,0,0},
			{0,1,0,0},
			{0,0,1,0},
			{0,0,0,1}
		};
		memcpy(mMatrices.textureTransform,Identity,sizeof(float) * 4 * 4);
		mShaders.CurrentShader->SetTextureTransform(mMatrices.textureTransform);
		mMatrices.textureTransformIsIdentity = true;
	}
}

void Graphics::BuildShaders()
{
	const char *ColourOnly_PS = R"(
		varying vec4 v_col;
		void main(void)
		{
			gl_FragColor = v_col;
		}
	)";

	const char *TextureColour_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = v_col * texture2D(u_tex0,v_tex0);
		}
	)";

	const char* ColourOnly_VS = R"(
		uniform mat4 u_proj_cam;
		uniform mat4 u_trans;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		varying vec4 v_col;
		void main(void)
		{
			v_col = u_global_colour;
			gl_Position = u_proj_cam * (u_trans * a_xyz);
		}
	)";

	const char* TextureColour_VS = R"(
		uniform mat4 u_proj_cam;
		uniform mat4 u_trans;
		uniform mat4 u_textTrans;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		attribute vec4 a_uv0;
		varying vec4 v_col;
		varying vec2 v_tex0;
		void main(void)
		{
			v_col = u_global_colour;
			v_tex0 = (a_uv0 * u_textTrans).xy;
			gl_Position = u_proj_cam * (u_trans * a_xyz);
		}
	)";

	const char *TextureAlphaOnly_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = vec4(v_col.rgb,texture2D(u_tex0,v_tex0).a);
		}
	)";

	const char* RectangleBorder_VS = R"(
		uniform mat4 u_proj_cam;
		uniform mat4 u_trans;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		attribute vec4 a_col;
		varying vec4 v_col;
		void main(void)
		{
			v_col = a_col * u_global_colour;
			gl_Position = u_proj_cam * (u_trans * a_xyz);
		}
	)";

	mShaders.ColourOnly = new GLShader("ColourOnly",ColourOnly_VS,ColourOnly_PS);
	mShaders.TextureColour = new GLShader("TextureColour",TextureColour_VS,TextureColour_PS);
	mShaders.TextureAlphaOnly = new GLShader("TextureAlphaOnly",TextureColour_VS,TextureAlphaOnly_PS);
	mShaders.RectangleBorder = new GLShader("RectangleBorder",RectangleBorder_VS,ColourOnly_PS);

}

void Graphics::EnableShader(GLShaderPtr pShader)
{
	assert( pShader );
	if( mShaders.CurrentShader != pShader )
	{
		mShaders.CurrentShader = pShader;
		pShader->Enable(mMatrices.projection,mMatrices.transform,mMatrices.textureTransform);
	}
}

void Graphics::VertexPtr(int pNum_coord, uint32_t pType,const void* pPointer)
{
	if(pNum_coord < 2 || pNum_coord > 3)
	{
		THROW_MEANINGFUL_EXCEPTION("VertexPtr passed invalid value for pNum_coord, must be 2 or 3 got " + std::to_string(pNum_coord));
	}

	glVertexAttribPointer(
				(GLuint)StreamIndex::VERTEX,
				pNum_coord,
				pType,
				pType == GL_BYTE,
				0,pPointer);
	CHECK_OGL_ERRORS();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
}//namespace eui{
