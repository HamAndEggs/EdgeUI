#ifndef GLTexture_H__
#define GLTexture_H__

#include "GLIncludes.h"

#include <string_view>

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

/**
 * @brief Mainly for debugging, returns a string representation of the enum.
 */
constexpr std::string_view TextureFormatToString(TextureFormat pFormat)
{
	switch( pFormat )
	{
	case TextureFormat::FORMAT_RGBA:
		return "FORMAT_RGBA";
		
	case TextureFormat::FORMAT_RGB:
		return "FORMAT_RGB";

	case TextureFormat::FORMAT_ALPHA:
		return "FORMAT_ALPHA";
	}
	return "Invalid TextureFormat";
}
 
constexpr GLint TextureFormatToGLFormat(TextureFormat pFormat)
{
	switch( pFormat )
	{
	case TextureFormat::FORMAT_RGB:
		return GL_RGB;

	case TextureFormat::FORMAT_RGBA:
		return GL_RGBA;

	case TextureFormat::FORMAT_ALPHA:
		return GL_ALPHA; // This is mainly used for the fonts.
	}
	return GL_INVALID_ENUM;
}

/**
 * @brief This is a pain in the arse, because we can't query the values used to create a gl texture we have to store them. horrid API GLES 2.0
 */
struct GLTexture
{
	GLTexture() = delete; // Forces user to use references.
	GLTexture(TextureFormat pFormat,int pWidth,int pHeight):mFormat(pFormat),mWidth(pWidth),mHeight(pHeight){}

	const TextureFormat mFormat;
	const int mWidth;
	const int mHeight;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //#ifndef GLTexture_H__
