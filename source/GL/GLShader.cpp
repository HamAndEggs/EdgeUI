
#include "GLShader.h"
#include "GLDiagnostics.h"

#include <string.h>
#include <iostream>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
GLShader::GLShader(const std::string& pName,const char* pVertex, const char* pFragment) :
	mName(pName),
	mEnableStreamUV(strstr(pVertex," a_uv0;")),
	mEnableStreamColour(strstr(pVertex," a_col;"))
{
	VERBOSE_SHADER_MESSAGE("Creating " << mName << " mEnableStreamUV " << mEnableStreamUV << " mEnableStreamColour" << mEnableStreamColour);

	mVertexShader = LoadShader(GL_VERTEX_SHADER,pVertex);

	mFragmentShader = LoadShader(GL_FRAGMENT_SHADER,pFragment);

	VERBOSE_SHADER_MESSAGE("vertex("<<mVertexShader<<") fragment("<<mFragmentShader<<")");

	mShader = glCreateProgram(); // create empty OpenGL Program
	CHECK_OGL_ERRORS();

	glAttachShader(mShader, mVertexShader); // add the vertex shader to program
	CHECK_OGL_ERRORS();

	glAttachShader(mShader, mFragmentShader); // add the fragment shader to program
	CHECK_OGL_ERRORS();

	//Set the input stream numbers.
	//Has to be done before linking.
	BindAttribLocation((int)StreamIndex::VERTEX, "a_xyz");
	BindAttribLocation((int)StreamIndex::TEXCOORD, "a_uv0");
	BindAttribLocation((int)StreamIndex::COLOUR, "a_col");

	glLinkProgram(mShader); // creates OpenGL program executables
	CHECK_OGL_ERRORS();

	GLint compiled;
	glGetProgramiv(mShader,GL_LINK_STATUS,&compiled);
	CHECK_OGL_ERRORS();
	if ( compiled == GL_FALSE )
	{	
		GLint infoLen = 0;
		glGetProgramiv ( mShader, GL_INFO_LOG_LENGTH, &infoLen );

		std::string error = "Failed to compile shader, infoLen " + std::to_string(infoLen) + "\n";
		if ( infoLen > 1 )
		{
			char* error_message = new char[infoLen];

			glGetProgramInfoLog(mShader,infoLen,&infoLen,error_message);

			error += error_message;

			delete []error_message;
		}
		glDeleteShader ( mShader );
		mShader = 0;
		THROW_MEANINGFUL_EXCEPTION(error);
	}

	VERBOSE_SHADER_MESSAGE("Shader " << mName << " Compiled ok");

	//Get the bits for the variables in the shader.
	mUniforms.proj_cam = GetUniformLocation("u_proj_cam");
	mUniforms.trans = GetUniformLocation("u_trans");
	mUniforms.global_colour = GetUniformLocation("u_global_colour");
	mUniforms.tex0 = GetUniformLocation("u_tex0");
	mUniforms.textureTrans = GetUniformLocation("u_textTrans");


	glUseProgram(0);
#ifdef VERBOSE_SHADER_BUILD
	gCurrentShaderName = "";
#endif
}

GLShader::~GLShader()
{
	VERBOSE_SHADER_MESSAGE("Deleting shader " << mName << " " << mShader);
	
	glDeleteShader(mVertexShader);
	CHECK_OGL_ERRORS();

	glDeleteShader(mFragmentShader);
	CHECK_OGL_ERRORS();

	glDeleteProgram(mShader);
	CHECK_OGL_ERRORS();
	mShader = 0;
}

int GLShader::GetUniformLocation(const char* pName)
{
	int location = glGetUniformLocation(mShader,pName);
	CHECK_OGL_ERRORS();

	if( location < 0 )
	{
		VERBOSE_SHADER_MESSAGE( mName << " Failed to find UniformLocation " << pName);
	}

	VERBOSE_SHADER_MESSAGE( mName << " GetUniformLocation(" << pName << ") == " << location);

	return location;

}

void GLShader::BindAttribLocation(int location,const char* pName)
{
	glBindAttribLocation(mShader, location,pName);
	CHECK_OGL_ERRORS();
	VERBOSE_SHADER_MESSAGE( mName << " AttribLocation("<< pName << "," << location << ")");
}

void GLShader::Enable(const float projInvcam[4][4],const float pTransform[4][4],const float pTextureTransform[4][4])
{
#ifdef VERBOSE_SHADER_BUILD
	gCurrentShaderName = mName;
#endif

	assert(mShader);
    glUseProgram(mShader);
    CHECK_OGL_ERRORS();

    glUniformMatrix4fv(mUniforms.proj_cam, 1, false,(const float*)projInvcam);
    CHECK_OGL_ERRORS();

	SetTransform(pTransform);
	SetTextureTransform(pTextureTransform);

	if( mEnableStreamUV )
	{
		glEnableVertexAttribArray((int)StreamIndex::TEXCOORD);
	}
	else
	{
		glDisableVertexAttribArray((int)StreamIndex::TEXCOORD);
	}

	if( mEnableStreamColour )
	{
		glEnableVertexAttribArray((int)StreamIndex::COLOUR);
	}
	else
	{
		glDisableVertexAttribArray((int)StreamIndex::COLOUR);
	}

    CHECK_OGL_ERRORS();
}

void GLShader::SetTransform(const float pTransform[4][4])
{
	assert(mUniforms.trans >= 0 );
	glUniformMatrix4fv(mUniforms.trans, 1, false,(const GLfloat*)pTransform);
	CHECK_OGL_ERRORS();
}

void GLShader::SetGlobalColour(const Colour pColour)
{
	SetGlobalColour(
		ColourToFloat(GetRed(pColour)),
		ColourToFloat(GetGreen(pColour)),
		ColourToFloat(GetBlue(pColour)),
		ColourToFloat(GetAlpha(pColour))
	);
	CHECK_OGL_ERRORS();
}

void GLShader::SetGlobalColour(float pRed,float pGreen,float pBlue,float pAlpha)
{
	glUniform4f(mUniforms.global_colour,pRed,pGreen,pBlue,pAlpha);
}

void GLShader::SetTexture(GLint pTexture)
{
	assert(pTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,pTexture);
	glUniform1i(mUniforms.tex0,0);
	CHECK_OGL_ERRORS();
}

void GLShader::SetTextureTransform(const float pTransform[4][4])
{
	if(mUniforms.textureTrans >= 0 )
	{
		glUniformMatrix4fv(mUniforms.textureTrans, 1, false,(const GLfloat*)pTransform);
		CHECK_OGL_ERRORS();
	}
}

int GLShader::LoadShader(int type, const char* shaderCode)
{
	// create a vertex shader type (GLES20.GL_VERTEX_SHADER)
	// or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
	int shaderFrag = glCreateShader(type);

	// If we're GLES system we need to add "precision highp float"
//#if defined PLATFORM_DRM_EGL || defined PLATFORM_WAYLAND_GL
	const std::string glesShaderCode= std::string("precision highp float; ") + shaderCode;
	shaderCode = glesShaderCode.c_str();
//#endif

	// add the source code to the shader and compile it
	glShaderSource(shaderFrag,1,&shaderCode,NULL);
	glCompileShader(shaderFrag);
	CHECK_OGL_ERRORS();
	// Check the compile status
	GLint compiled;
	glGetShaderiv(shaderFrag,GL_COMPILE_STATUS,&compiled);
	if ( compiled == GL_FALSE )
	{
		GLint infoLen = 0;
		glGetShaderiv ( shaderFrag, GL_INFO_LOG_LENGTH, &infoLen );
		const std::string shaderType = (type == GL_VERTEX_SHADER) ? "GL_VERTEX_SHADER" : "GL_FRAGMENT_SHADER";
		std::string error = "Failed to compile shader, " + shaderType + ", infoLen " + std::to_string(infoLen) + "\n";

		if ( infoLen > 1 )
		{
			char* error_message = new char[infoLen];

			glGetShaderInfoLog(shaderFrag,infoLen,&infoLen,error_message);

			error += error_message;
		}
		glDeleteShader ( shaderFrag );
		std::cerr << shaderCode << "\n";

		THROW_MEANINGFUL_EXCEPTION(error);
	}
	CHECK_OGL_ERRORS();

	return shaderFrag;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{
