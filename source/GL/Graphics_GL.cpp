#include "Graphics_GL.h"
#include "GLDiagnostics.h"

#include <math.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
static Graphics* theGraphics = nullptr;

Graphics* Graphics::Open(DisplayRotation pDisplayRotation)
{
	if( theGraphics != nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("Graphics engine all ready allocated. Please only call Graphics::Open once");
	}
    theGraphics = new Graphics_GL(pDisplayRotation);
	return theGraphics;
}

void Graphics::Close()
{
	delete theGraphics;
	theGraphics = nullptr;
}

Graphics* Graphics::Get()
{
	if( theGraphics == nullptr )
	{
		THROW_MEANINGFUL_EXCEPTION("Graphics engine not allocated. Please call Graphics::Open first");
	}
	return theGraphics;
}

Graphics_GL::Graphics_GL(DisplayRotation pDisplayRotation)
{
    mPlatform = std::make_unique<PlatformInterface>();

	mPhysical.Width = mPlatform->GetWidth();
	mPhysical.Height = mPlatform->GetHeight();

	if( mCreateFlags&ROTATE_FRAME_PORTRATE )
	{
		mCreateFlags &= ~ROTATE_FRAME_PORTRATE;
		if( mPhysical.Width > mPhysical.Height )
		{
			mCreateFlags |= ROTATE_FRAME_BUFFER_90;
		}
	}

	if( mCreateFlags&ROTATE_FRAME_LANDSCAPE )
	{
		mCreateFlags &= ~ROTATE_FRAME_LANDSCAPE;
		if( mPhysical.Width < mPhysical.Height )
		{
			mCreateFlags |= ROTATE_FRAME_BUFFER_90;
		}
	}

	if( mCreateFlags&(ROTATE_FRAME_BUFFER_90|ROTATE_FRAME_BUFFER_270) )
	{
		mReported.Width = mPhysical.Height;
		mReported.Height = mPhysical.Width;
	}
	else
	{
		mReported.Width = mPhysical.Width;
		mReported.Height = mPhysical.Height;
	}

	VERBOSE_MESSAGE("Physical display resolution is " << mPhysical.Width << "x" << mPhysical.Height );

	mPlatform->InitialiseDisplay();

	SetRenderingDefaults();
	BuildShaders();
	BuildDebugTexture();
	InitFreeTypeFont();

	VERBOSE_MESSAGE("GLES Ready");
}

Graphics_GL::~Graphics_GL()
{
	VERBOSE_MESSAGE("GLES destructor called");

	VERBOSE_MESSAGE("On exit the following scratch memory buffers reached the sizes of...");
	VERBOSE_MESSAGE("    mWorkBuffers.verticesInt16XY " << mWorkBuffers.verticesInt16XY.MemoryUsed() << " bytes");
	VERBOSE_MESSAGE("    mWorkBuffers.verticesInt32XY " << mWorkBuffers.verticesInt32XY.MemoryUsed() << " bytes");
	VERBOSE_MESSAGE("    mWorkBuffers.verticesFloatXY " << mWorkBuffers.verticesFloatXY.MemoryUsed() << " bytes");
	VERBOSE_MESSAGE("    mWorkBuffers.uvShort " << mWorkBuffers.uvShort.MemoryUsed() << " bytes");

	glBindTexture(GL_TEXTURE_2D,0);
	CHECK_OGL_ERRORS();

	// Kill shaders.
	VERBOSE_MESSAGE("Deleting shaders");

	glUseProgram(0);
	CHECK_OGL_ERRORS();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

	mShaders.CurrentShader.reset();
	mShaders.ColourOnly2D.reset();
	mShaders.TextureColour2D.reset();
	mShaders.TextureAlphaOnly2D.reset();
	mShaders.SpriteShader2D.reset();
	mShaders.QuadBatchShader2D.reset();

	mShaders.ColourOnly3D.reset();
	mShaders.TextureOnly3D.reset();


	// delete all free type fonts.
#ifdef USE_FREETYPEFONTS
	mFreeTypeFonts.clear();
	if( mFreetype != nullptr )
	{
		if( FT_Done_FreeType(mFreetype) == FT_Err_Ok )
		{
			mFreetype = nullptr;
			VERBOSE_MESSAGE("Freetype font library deleted");
		}
	}
#endif

	// delete all textures.
	for( auto& t : mTextures )
	{
		glDeleteTextures(1,&t.first);
		CHECK_OGL_ERRORS();
	}

	VERBOSE_MESSAGE("All done");
}

Rectangle Graphics_GL::GetDisplayRect()const
{
    return Rectangle(0,0,GetDisplayWidth(),GetDisplayHeight());
}

RectangleF Graphics_GL::GetDisplayRectF()const
{
    return RectangleF(0.0f,0.0f,(float)GetDisplayWidth(),(float)GetDisplayHeight());
}

int32_t Graphics_GL::GetDisplayWidth()const
{
    return mPlatform->GetWidth();
}

int32_t Graphics_GL::GetDisplayHeight()const
{
    return mPlatform->GetHeight();
}

void Graphics_GL::BeginFrame()
{
	mDiagnostics.frameNumber++;

	// Reset some items so that we have a working render setup to begin the frame with.
	// This is done so that I don't have to have a load of if statements to deal with first frame. Also makes life simpler for the more minimal applications.
	EnableShader(mShaders.ColourOnly2D);

	static float identity[4][4] = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};
	SetTransform(identity);
}

void Graphics_GL::EndFrame()
{
	glFlush();// This makes sure the display is fully up to date before we allow them to interact with any kind of UI. This is the specified use of this function.
	mPlatform->SwapBuffers();
}

bool Graphics_GL::ProcessSystemEvents(EventTouchScreen mTouchEvent)
{
    mPlatform->ProcessEvents(mTouchEvent,[this]()
    {
        mExitRequest = true;
    });

    return mExitRequest == false;
}

void Graphics_GL::GetFontRect(uint32_t pFont,RectangleF& rRect)
{

}

void Graphics_GL::DrawFont(const Style& pStyle,int32_t pX,int32_t pY,const std::string& pText)
{

}

void Graphics_GL::DrawRectangle(const RectangleF& pRect,const Style& pStyle)
{
    if( pStyle.mRadius )
    {
		GetRoundedRectanglePoints(pRect,mWorkBuffers.verticesFloatXY,pStyle.mRadius);

		const int numPoints = mWorkBuffers.verticesFloatXY.Used();
		const VertFloatXY* points = mWorkBuffers.verticesFloatXY.Data();

        EnableShader(mShaders.ColourOnly2D);
        mShaders.CurrentShader->SetGlobalColour(pStyle.mBackground);

        VertexPtr(2,GL_FLOAT,points);
        glDrawArrays(GL_TRIANGLE_FAN,0,numPoints);
        CHECK_OGL_ERRORS();

        // Now see if we need to draw a boarder.
        if( pStyle.mBorder != COLOUR_NONE && pStyle.mBorderSize > 0 )
        {
            mShaders.CurrentShader->SetGlobalColour(pStyle.mBorder);
            if( pStyle.mBorderSize == 1 )
            {
                glDrawArrays(GL_LINE_LOOP,0,numPoints);
                CHECK_OGL_ERRORS();
            }
            else
            {
				const float shrinkX = ((float)pRect.width - pStyle.mBorderSize) / pRect.width;
				const float shrinkY = ((float)pRect.height - pStyle.mBorderSize) / pRect.height;

				auto MakeBorder = [shrinkX,shrinkY,pRect](VertFloatXY* rDest,const VertFloatXY& pSource)
				{
					rDest[1].x = pSource.x;
					rDest[1].y = pSource.y;
					rDest[0].x = ((pSource.x - pRect.GetCenterX()) * shrinkX) + pRect.GetCenterX();
					rDest[0].y = ((pSource.y - pRect.GetCenterY()) * shrinkY) + pRect.GetCenterY();
				};

				VertFloatXY* border = (VertFloatXY*)mWorkBuffers.scratchRam.Restart(sizeof(VertFloatXY) * (numPoints+1) * 2);
				for(int n = 0 ; n < numPoints ; n++ )
				{
					MakeBorder(border,points[n]);
					border += 2;
				}
				MakeBorder(border,points[0]);

				VertexPtr(2,GL_FLOAT,mWorkBuffers.scratchRam.Data());
				glDrawArrays(GL_TRIANGLE_STRIP,0,(numPoints+1)*2);

				CHECK_OGL_ERRORS();

            }
        }
    }
    else
    {
        int16_t quad[8];    
        pRect.GetQuad(quad);

        EnableShader(mShaders.ColourOnly2D);
        mShaders.CurrentShader->SetGlobalColour(pStyle.mBackground);
        VertexPtr(2,GL_SHORT,quad);
        glDrawArrays(GL_TRIANGLE_FAN,0,4);
        CHECK_OGL_ERRORS();

        // Now see if we need to draw a boarder.
        if( pStyle.mBorder != COLOUR_NONE && pStyle.mBorderSize > 0 )
        {
            if( pStyle.mBorderSize == 1 )
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
}

void Graphics_GL::DrawLine(int pFromX,int pFromY,int pToX,int pToY,Colour pColour,uint32_t pWidth)
{
	if( pWidth < 2 )
	{
		const int16_t quad[4] = {(int16_t)pFromX,(int16_t)pFromY,(int16_t)pToX,(int16_t)pToY};

		EnableShader(mShaders.ColourOnly2D);
		mShaders.CurrentShader->SetGlobalColour(pColour);

		VertexPtr(2,GL_SHORT,quad);
		glDrawArrays(GL_LINES,0,2);
		CHECK_OGL_ERRORS();
	}
	else
	{
		pWidth /= 2;
		VertInt16XY p[6];

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

		EnableShader(mShaders.ColourOnly2D);
		mShaders.CurrentShader->SetGlobalColour(pColour);

		VertexPtr(2,GL_SHORT,p);
		glDrawArrays(GL_TRIANGLE_FAN,0,6);
		CHECK_OGL_ERRORS();
	}
}

void Graphics_GL::SetRenderingDefaults()
{
	glViewport(0, 0, (GLsizei)mPhysical.Width, (GLsizei)mPhysical.Height);
	glDepthRangef(0.0f,1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    SetProjection2D();

	// Always cull, because why not. :) Make code paths simple.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// I have alpha blend on all the time. Makes life easy. No point in complicating the code for speed, going for simple implementation not fastest!
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnableVertexAttribArray((int)StreamIndex::VERTEX);//Always on

	CHECK_OGL_ERRORS();
}

void Graphics_GL::BuildDebugTexture()
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

	mDiagnostics.texture = CreateTexture(16,16,pixels,TextureFormat::FORMAT_RGBA);
}

void Graphics_GL::InitFreeTypeFont()
{
#ifdef USE_FREETYPEFONTS	
	if( FT_Init_FreeType(&mFreetype) == 0 )
	{
		VERBOSE_MESSAGE("Freetype font library created");
	}
	else
	{
		THROW_MEANINGFUL_EXCEPTION("Failed to init free type font library");
	}
#endif
}

void Graphics_GL::SetProjection2D()
{
	// Setup 2D frustum
	memset(mMatrices.projection,0,sizeof(mMatrices.projection));
	mMatrices.projection[3][3] = 1;

	if( mCreateFlags&ROTATE_FRAME_BUFFER_90 )
	{
		mMatrices.projection[0][1] = -2.0f / (float)mPhysical.Height;
		mMatrices.projection[1][0] = -2.0f / (float)mPhysical.Width;
				
		mMatrices.projection[3][0] = 1;
		mMatrices.projection[3][1] = 1;
	}
	else if( mCreateFlags&ROTATE_FRAME_BUFFER_180 )
	{
		mMatrices.projection[0][0] = -2.0f / (float)mPhysical.Width;
		mMatrices.projection[1][1] = 2.0f / (float)mPhysical.Height;
				
		mMatrices.projection[3][0] = 1;
		mMatrices.projection[3][1] = -1;
	}
	else if( mCreateFlags&ROTATE_FRAME_BUFFER_270 )
	{
		mMatrices.projection[0][1] = 2.0f / (float)mPhysical.Height;
		mMatrices.projection[1][0] = 2.0f / (float)mPhysical.Width;
				
		mMatrices.projection[3][0] = -1;
		mMatrices.projection[3][1] = -1;
	}
	else
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

void Graphics_GL::SetTransform(float pTransform[4][4])
{
	assert(mShaders.CurrentShader);
	memcpy(mMatrices.transform,pTransform,sizeof(float) * 4 * 4);
	mShaders.CurrentShader->SetTransform(pTransform);
}

void Graphics_GL::BuildShaders()
{
	const char* ColourOnly2D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		varying vec4 v_col;
		void main(void)
		{
			v_col = u_global_colour;
			gl_Position = u_proj_cam * a_xyz;
		}
	)";

	const char *ColourOnly2D_PS = R"(
		varying vec4 v_col;
		void main(void)
		{
			gl_FragColor = v_col;
		}
	)";

	mShaders.ColourOnly2D = std::make_unique<GLShader>("ColourOnly2D",ColourOnly2D_VS,ColourOnly2D_PS);

	const char* TextureColour2D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		attribute vec2 a_uv0;
		varying vec4 v_col;
		varying vec2 v_tex0;
		void main(void)
		{
			v_col = u_global_colour;
			v_tex0 = a_uv0;
			gl_Position = u_proj_cam * a_xyz;
		}
	)";

	const char *TextureColour2D_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = v_col * texture2D(u_tex0,v_tex0);
		}
	)";

	mShaders.TextureColour2D = std::make_unique<GLShader>("TextureColour2D",TextureColour2D_VS,TextureColour2D_PS);

	const char* TextureAlphaOnly2D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		attribute vec2 a_uv0;
		varying vec4 v_col;
		varying vec2 v_tex0;
		void main(void)
		{
			v_col = u_global_colour;
			v_tex0 = a_uv0;
			gl_Position = u_proj_cam * a_xyz;
		}
	)";

	const char *TextureAlphaOnly2D_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = vec4(v_col.rgb,texture2D(u_tex0,v_tex0).a);
		}
	)";

	mShaders.TextureAlphaOnly2D = std::make_unique<GLShader>("TextureAlphaOnly2D",TextureAlphaOnly2D_VS,TextureAlphaOnly2D_PS);
	
	const char* SpriteShader2D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform mat4 u_trans;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		attribute vec2 a_uv0;
		varying vec4 v_col;
		varying vec2 v_tex0;
		void main(void)
		{
			v_col = u_global_colour;
			v_tex0 = a_uv0;
			gl_Position = u_proj_cam * (u_trans * a_xyz);
		}
	)";

	const char *SpriteShader2D_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = v_col * texture2D(u_tex0,v_tex0);
		}
	)";

	mShaders.SpriteShader2D = std::make_unique<GLShader>("SpriteShader2D",SpriteShader2D_VS,SpriteShader2D_PS);

	const char* QuadBatchShader2D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform vec4 u_global_colour;
		attribute vec4 a_xyz;
		attribute vec2 a_uv0;
		attribute vec4 a_trans;
		varying vec4 v_col;
		varying vec2 v_tex0;
		void main(void)
		{
			float scale = a_trans.w;
			float sCos = cos(a_trans.z * 0.00019175455);
			float sSin = sin(a_trans.z * 0.00019175455);

			mat4 trans;
			trans[0][0] = sCos * scale;
			trans[0][1] = sSin * scale;
			trans[0][2] = 0.0; 
			trans[0][3] = 0.0;

			trans[1][0] = -sSin * scale;
			trans[1][1] = sCos * scale;
			trans[1][2] = 0.0; 
			trans[1][3] = 0.0;

			trans[2][0] = 0.0;
			trans[2][1] = 0.0;
			trans[2][2] = scale;
			trans[2][3] = 0.0;

			trans[3][0] = a_trans.x;
			trans[3][1] = a_trans.y;
			trans[3][2] = 0.0;
			trans[3][3] = 1.0;

			v_col = u_global_colour;
			v_tex0 = a_uv0;
			gl_Position = u_proj_cam * (trans * a_xyz);
		}
	)";

	const char *QuadBatchShader2D_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = v_col * texture2D(u_tex0,v_tex0);
		}
	)";

	mShaders.QuadBatchShader2D = std::make_unique<GLShader>("QuadBatchShader2D",QuadBatchShader2D_VS,QuadBatchShader2D_PS);


	const char* ColourOnly3D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform mat4 u_trans;
		uniform vec4 u_global_colour;		
		attribute vec4 a_xyz;
		attribute vec4 a_col;
		varying vec4 v_col;
		void main(void)
		{
			v_col = u_global_colour * a_col;
			gl_Position = u_proj_cam * (u_trans * a_xyz);
		}
	)";

	const char *ColourOnly3D_PS = R"(
		varying vec4 v_col;
		void main(void)
		{
			gl_FragColor = v_col;
		}
	)";

	mShaders.ColourOnly3D = std::make_unique<GLShader>("ColourOnly3D",ColourOnly3D_VS,ColourOnly3D_PS);	

	const char* TextureOnly3D_VS = R"(
		uniform mat4 u_proj_cam;
		uniform mat4 u_trans;
		uniform vec4 u_global_colour;		
		attribute vec4 a_xyz;
		attribute vec2 a_uv0;
		varying vec4 v_col;
		varying vec2 v_tex0;
		void main(void)
		{
			v_col = u_global_colour;
			v_tex0 = a_uv0;
			gl_Position = u_proj_cam * (u_trans * a_xyz);
		}
	)";

	const char *TextureOnly3D_PS = R"(
		varying vec4 v_col;
		varying vec2 v_tex0;
		uniform sampler2D u_tex0;
		void main(void)
		{
			gl_FragColor = v_col * texture2D(u_tex0,v_tex0);
		}
	)";

	mShaders.TextureOnly3D = std::make_unique<GLShader>("TextureOnly3D",TextureOnly3D_VS,TextureOnly3D_PS);	
}

void Graphics_GL::EnableShader(GLShaderPtr pShader)
{
	assert( pShader );
	if( mShaders.CurrentShader != pShader )
	{
		mShaders.CurrentShader = pShader;
		pShader->Enable(mMatrices.projection);
		pShader->SetTransform(mMatrices.transform);
	}
}

void Graphics_GL::VertexPtr(int pNum_coord, uint32_t pType,const void* pPointer)
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

uint32_t Graphics_GL::CreateTexture(int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat,bool pFiltered,bool pGenerateMipmaps)
{
	const GLint format = TextureFormatToGLFormat(pFormat);
	if( format == GL_INVALID_ENUM )
	{
		THROW_MEANINGFUL_EXCEPTION("CreateTexture passed an unknown texture format, I can not continue.");
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

void Graphics_GL::FillTexture(uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels,TextureFormat pFormat,bool pGenerateMips)
{
	glBindTexture(GL_TEXTURE_2D,pTexture);

	const GLint format = TextureFormatToGLFormat(pFormat);
	if( format == GL_INVALID_ENUM )
	{
		THROW_MEANINGFUL_EXCEPTION("FillTexture passed an unknown texture format, I can not continue.");
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

void Graphics_GL::DeleteTexture(uint32_t pTexture)
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

int Graphics_GL::GetTextureWidth(uint32_t pTexture)const
{
	return mTextures.at(pTexture)->mWidth;
}

int Graphics_GL::GetTextureHeight(uint32_t pTexture)const
{
	return mTextures.at(pTexture)->mHeight;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////    
};//namespace eui{
