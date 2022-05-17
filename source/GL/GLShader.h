#ifndef GLShader_h__
#define GLShader_h__

#include "GLIncludes.h"
#include "Graphics.h"

#include <string>
#include <memory>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
enum struct StreamIndex
{
	VERTEX				= 0,		//!< Vertex positional data.
	TEXCOORD			= 1,		//!< Texture coordinate information.
	COLOUR				= 2,		//!< Colour type is in the format RGBA.
};


struct GLShader
{
	GLShader(const std::string& pName,const char* pVertex, const char* pFragment);
	~GLShader();

	int GetUniformLocation(const char* pName);
	void BindAttribLocation(int location,const char* pName);
	void Enable(const float projInvcam[4][4],const float pTransform[4][4]);
	void SetTransform(const float transform[4][4]);
	void SetGlobalColour(const Colour pColour);
	void SetGlobalColour(float red,float green,float blue,float alpha);
	void SetTexture(GLint texture);
	void SetTextureTransform(float pTransform[4]);

	bool GetUsesTexture()const{return mUniforms.tex0 > -1;}
	bool GetUsesTransform()const{return mUniforms.trans > -1;}

	const std::string mName;	//!< Mainly to help debugging.
	const bool mEnableStreamUV;
	const bool mEnableStreamColour;

	GLint mShader = 0;
	GLint mVertexShader = 0;
	GLint mFragmentShader = 0;
	struct
	{
		GLint trans;
		GLint proj_cam;
		GLint global_colour;
		GLint tex0;
		GLint textureTrans;
	}mUniforms;

	int LoadShader(int type, const char* shaderCode);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{


#endif //GLShader_h__