#ifndef FreeTypeFont_H__
#define FreeTypeFont_H__

#include <freetype2/ft2build.h> //sudo apt install libfreetype6-dev
#include FT_FREETYPE_H

#include <vector>
#include <string>
#include <functional>
#include <stdint.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Optional freetype font library support. 
 * Rendering is done in the graphics code, this class is just a container with platform independent font specific code..
 */
struct FreeTypeFont
{
	/**
	 * @brief An entry into the glyph cache
	 */
	struct Glyph
	{
		int width;
		int height;
		int pitch;
		int advance;
		int x_off,y_off;	//!< offset from current x and y that the quad is rendered.
		struct
		{// Where, in 16bit UV's, the glyph is.
			int x,y;
		}uv[2];
	};

	FreeTypeFont(FT_Face pFontFace,int pPixelHeight);
	~FreeTypeFont();

	/**
	 * @brief Get the Glyph object of an ASCII character. All that is needed to render as well as build the texture.
	 */
	bool GetGlyph(FT_UInt pChar,FreeTypeFont::Glyph& rGlyph,std::vector<uint8_t>& rPixels);

	/**
	 * @brief Builds our texture object.
	 */
	void BuildTexture(
			int pMaximumAllowedGlyph,
			std::function<uint32_t(int pWidth,int pHeight)> pCreateTexture,
			std::function<void(uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels)> pFillTexture);

	void BuildQuads(const char* pText,int pX,int pY,VertXY::Buffer& pVertices,VertXY::Buffer& pUVs)const;

	const std::string mFontName; //<! Helps with debugging.
	FT_Face mFace;								//<! The font we are rending from.
	uint32_t mTexture;							//<! This is the texture that the glyphs are in so we can render using GL and quads.
	std::array<FreeTypeFont::Glyph,96>mGlyphs;	//<! Meta data needed to render the characters.
	int mBaselineHeight;						//<! This is the number of pixels above baseline the higest character is. Used for centering a font in the y.
	int mSpaceAdvance;							//<! How much to advance by for a non rerendered character.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //#ifndef FreeTypeFont_H__