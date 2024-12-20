#include "Graphics.h"
#include "Diagnostics.h"
#include "FreeTypeFont.h"

/**
 * @brief Because this is a very simple font system I have a limited number of characters I can render. This allows me to use the ones I expect to be most useful.
 */
static std::array<int,256>GlyphIndex;
static bool BuildGlyphIndex = true;

inline int GetGlyphIndex(FT_UInt pCharacter)
{
	if( pCharacter < GlyphIndex.size() )
	{
		return GlyphIndex[pCharacter];
	}
	return -1;
}

inline FT_UInt GetNextGlyph(const char *& pText)
{
	if( pText == 0 )
		return 0;

	const FT_UInt c1 = *pText;
	pText++;

	if( (c1&0x80) == 0 )
	{
		return c1;
	}

	const FT_UInt c2 = *pText;
	pText++;

	return ((c1&0x3f) << 6) | (c2&0x3f);
}

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////
FreeTypeFont::FreeTypeFont(const std::string mID,FT_Face pFontFace,int pPixelHeight) :
	mID(pFontFace->family_name),
	mFontName(pFontFace->family_name),
	mFace(pFontFace)
{
	if( BuildGlyphIndex )
	{
		BuildGlyphIndex = false;
		// First set all to -1 (not used)
		for( auto& i : GlyphIndex )
		{
			i = -1;
		}

		// Now set up the indices for the ones we use. has be be 96 characters as that is how many glyphs we have.
		const char AllowedCharacters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUZWXYZabcdefghijklmnopqrstuvwxyz@!\"#$%&'()*+,-./:;<>=?[]\\^{}|~`¬£";

		size_t n = 0;

		const char* ptr = AllowedCharacters;

		FT_UInt glyph = 0;
		while( (glyph = GetNextGlyph(ptr)) != 0 )
		{
			assert( n < 96 );
			assert( glyph < GlyphIndex.size() );
			GlyphIndex[glyph] = n;
			n++;
		}

//		std::clog << "sizeof(AllowedCharacters) == " << sizeof(AllowedCharacters) << "\n";
//		assert( sizeof(AllowedCharacters) == 96 );
/*		for( size_t n = 0 ; n < sizeof(AllowedCharacters) ; n++ )
		{
			const char letter = AllowedCharacters[n];
			const int index = letter+128;
			std::clog << letter << " : " << index << " -> " << n << "\n";
			GlyphIndex[index] = n;
		}
		*/
	}


	if( FT_Set_Pixel_Sizes(mFace,0,pPixelHeight) == 0 )
	{
		VERBOSE_MESSAGE("Set pixel size " << pPixelHeight << " for true type font " << mFontName);
	}
	else
	{
		VERBOSE_MESSAGE("Failed to set pixel size " << pPixelHeight << " for true type font " << mFontName);
	}
}

FreeTypeFont::~FreeTypeFont()
{
	VERBOSE_MESSAGE("Deleting font:"<<mID<<" face:"<<mFace);
	FT_Done_Face(mFace);	
}

bool FreeTypeFont::GetGlyph(FT_UInt pChar,FreeTypeFont::Glyph& rGlyph,std::vector<uint8_t>& rPixels)
{
	assert(mFace);

	// Copied from original example source by Kevin Boone. http://kevinboone.me/fbtextdemo.html?i=1

	// Note that TT fonts have no built-in padding. 
	// That is, first,
	//  the top row of the bitmap is the top row of pixels to 
	//  draw. These rows usually won't be at the face bounding box. We need to
	//  work out the overall height of the character cell, and
	//  offset the drawing vertically by that amount. 
	//
	// Similar, there is no left padding. The first pixel in each row will not
	//  be drawn at the left margin of the bounding box, but in the centre of
	//  the screen width that will be occupied by the glyph.
	//
	//  We need to calculate the x and y offsets of the glyph, but we can't do
	//  this until we've loaded the glyph, because metrics
	//  won't be available.

	// Note that, by default, TT metrics are in 64'ths of a pixel, hence
	//  all the divide-by-64 operations below.

	// Get a FreeType glyph index for the character. If there is no
	//  glyph in the face for the character, this function returns
	//  zero.  
	FT_UInt gi = FT_Get_Char_Index (mFace, pChar);
	if( gi == 0 )
	{// Character not found, so default to space.
		VERBOSE_MESSAGE("Font: "<< mFontName << " Failed find glyph for character index " << (int)pChar);
		return false;
	}

	// Loading the glyph makes metrics data available
	if( FT_Load_Glyph (mFace, gi, FT_LOAD_DEFAULT ) != 0 )
	{
		VERBOSE_MESSAGE("Font: "<< mFontName << " Failed to load glyph for character index " << (int)pChar);
		return false;
	}

	// Rendering a loaded glyph creates the bitmap
	if( FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_NORMAL) != 0 )
	{
		VERBOSE_MESSAGE("Font: "<< mFontName << " Failed to render glyph for character index " << (int)pChar);
		return false;
	}

	assert(mFace->glyph);

	// Now we have the metrics, let's work out the x and y offset
	//  of the glyph from the specified x and y. Because there is
	//  no padding, we can't just draw the bitmap so that it's
	//  TL corner is at (x,y) -- we must insert the "missing" 
	//  padding by aligning the bitmap in the space available.

	// bbox.yMax is the height of a bounding box that will enclose
	//  any glyph in the face, starting from the glyph baseline.
	// Code changed, was casing it to render in the Y center of the font not on the base line. Will add it as an option in the future. Richard.
	int bbox_ymax = 0;//mFace->bbox.yMax / 64;
	mBaselineHeight = std::max(mBaselineHeight,(int)mFace->bbox.yMax / 64);

	// glyph_width is the pixel width of this specific glyph
	int glyph_width = mFace->glyph->metrics.width / 64;

	// So now we have (x_off,y_off), the location at which to
	//   start drawing the glyph bitmap.

	// Build the new glyph.
	rGlyph.width = mFace->glyph->bitmap.width;
	rGlyph.height = mFace->glyph->bitmap.rows;
	rGlyph.pitch = mFace->glyph->bitmap.pitch;

	// Advance is the amount of x spacing, in pixels, allocated
	//   to this glyph
	rGlyph.advance = mFace->glyph->metrics.horiAdvance / 64;


	// horiBearingX is the height of the top of the glyph from
	//   the baseline. So we work out the y offset -- the distance
	//   we must push down the glyph from the top of the bounding
	//   box -- from the height and the Y bearing.
	rGlyph.y_off = bbox_ymax - mFace->glyph->metrics.horiBearingY / 64;

	// Work out where to draw the left-most row of pixels --
	//   the x offset -- by halving the space between the 
	//   glyph width and the advance
	rGlyph.x_off = (rGlyph.advance - glyph_width) / 2;

	// It's an alpha only texture
	const size_t expectedSize = mFace->glyph->bitmap.rows * mFace->glyph->bitmap.pitch;
	// Some have no pixels, and so we just stop here.
	if(expectedSize == 0)
	{
		VERBOSE_MESSAGE("Font character " << pChar << " has no pixels " << mFace->glyph->bitmap.rows << " " << mFace->glyph->bitmap.pitch );
		return true;
	}

	assert(mFace->glyph->bitmap.buffer);

	if( mFace->glyph->bitmap.pitch == (int)mFace->glyph->bitmap.width )
	{// Quick path. Normally taken.
		rPixels.resize(expectedSize);
		memcpy(rPixels.data(),mFace->glyph->bitmap.buffer,expectedSize);
	}
	else
	{
		rPixels.reserve(expectedSize);
		const uint8_t* src = mFace->glyph->bitmap.buffer;
		for (int i = 0; i < (int)mFace->glyph->bitmap.rows; i++ , src += mFace->glyph->bitmap.pitch )
		{
			for (int j = 0; j < (int)mFace->glyph->bitmap.width; j++ )
			{
				rPixels.push_back(src[j]);
			}
		}
	}

	if( expectedSize != rPixels.size() )
	{
		THROW_MEANINGFUL_EXCEPTION("Font: " + mFontName + " Error, we read more pixels for free type font than expected for the glyph " + std::to_string(pChar) );
	}

	return true;
}

void FreeTypeFont::BuildTexture(
			int pMaximumAllowedGlyph,
			std::function<uint32_t(int pWidth,int pHeight)> pCreateTexture,
			std::function<void(uint32_t pTexture,int pX,int pY,int pWidth,int pHeight,const uint8_t* pPixels)> pFillTexture)
{

	int maxX = 0,maxY = 0;
	mBaselineHeight = 0;

	FreeTypeFont::Glyph spaceGlyph;
	std::vector<uint8_t> spacePixels;
	GetGlyph(' ',spaceGlyph,spacePixels);
	mSpaceAdvance = spaceGlyph.advance;

	std::array<std::vector<uint8_t>,96>glyphsPixels;
	for( FT_UInt c = 0 ; c < 256 ; c++ )// Cheap and quick font ASCII renderer. I'm not geeting into unicode. It's a nightmare to make fast in GL on a resource constrained system!
	{
		const int index = GetGlyphIndex(c);
		if( index >= 0 )
		{
			assert( (size_t)index < mGlyphs.size() );
			assert( (size_t)index < glyphsPixels.size() );

			auto& g = mGlyphs.at(index);
			auto& p = glyphsPixels.at(index);
			if( GetGlyph(c,g,p) )
			{
				if( p.size() > 0 )
				{
					maxX = std::max(maxX,g.width);
					maxY = std::max(maxY,g.height);
				}
				else
				{
					VERBOSE_MESSAGE("Character " << c << " is empty, will just move the cursor " << g.advance << " pixels");
				}
			}
		}
	}
	VERBOSE_MESSAGE("Font max glyph size requirement for cache is " << maxX << " " << maxY << " mBaselineHeight = " << mBaselineHeight);
	if( maxX > pMaximumAllowedGlyph || maxY > pMaximumAllowedGlyph )
	{
		THROW_MEANINGFUL_EXCEPTION("Font: " + mFontName + " requires a very large texture as it's maximum size glyph is very big, maxX == " + std::to_string(maxX) + " maxY == " + std::to_string(maxY) + ". This creation has been halted. Please reduce size of font!");
	}

	auto nextPow2 = [](int v)
	{
		int pow2 = 1;
		while( pow2 < v )
		{
			pow2 <<= 1;
		}
		return pow2;
	};

	// Work out a texture size that will fit. Need 96 slots. 32 -> 127
	const float width = nextPow2(maxX * 12);
	const float height = nextPow2(maxY * 8);
	VERBOSE_MESSAGE("Texture size needed is << " << width << "x" << height);

	mTexture = pCreateTexture(width,height);
	assert(mTexture);

	// Now get filling. Could have a lot of wasted space, but I am not getting into complicated packing algos at load time. Take it offline. :)
	const float cellWidth = width / 12;
	const float cellHeight = height / 8;

	int y = 0;
	int x = 0;
	for( FT_UInt c = 0 ; c < 256 ; c++ )
	{
		const int index = GetGlyphIndex(c);
		if( index >= 0 )
		{
			auto& g = mGlyphs.at(index);
			auto& p = glyphsPixels.at(index);
			const float cx = (x * cellWidth) + (cellWidth/2) - (g.width / 2);
			const float cy = (y * cellHeight) + (cellHeight/2) - (g.height / 2);
			if( p.size() > 0 )
			{
				pFillTexture(
					mTexture,
					cx,
					cy,
					g.width,
					g.height,
					p.data()
					);

				g.uv[0].x = cx / width;
				g.uv[0].y = cy / height;
				g.uv[1].x = (cx + g.width)  / width;
				g.uv[1].y = (cy + g.height) / height;
			}
			else
			{
				g.uv[0].x = 0;
				g.uv[0].y = 0;
				g.uv[1].x = 0;
				g.uv[1].y = 0;
			}

			// Advance to the next free cell.
			x++;
			if( x == 12 )
			{
				x = 0;
				y++;
			}
		}
	}	
}

void FreeTypeFont::BuildQuads(const char* pText,float pX,float pY,VertXY::Buffer& pVertices,VertXY::Buffer& pUVs)const
{
	FT_UInt glyph = 0;
	while( (glyph = GetNextGlyph(pText)) != 0 )
	{
		const size_t index = GetGlyphIndex(glyph);
		if( index < mGlyphs.size() )
		{
			auto&g = mGlyphs.at(index);

			pVertices.BuildQuad(pX + g.x_off,pY + g.y_off,g.width,g.height);

			pUVs.AddUVRect(
					g.uv[0].x,
					g.uv[0].y,
					g.uv[1].x,
					g.uv[1].y);

			pX += g.advance;
		}
	}
}

Rectangle FreeTypeFont::GetRect(const std::string_view& pText)const
{
	Rectangle r = {0,0,0,0};

	const char* text = pText.data();
	FT_UInt glyph = 0;
	float x = 0;
	while( (glyph = GetNextGlyph(text)) != 0 )
	{
		const size_t index = GetGlyphIndex(glyph);
		if( index < mGlyphs.size() )
		{
			auto &g = mGlyphs.at(index);

			const float px = x + g.x_off;
			const float py = g.y_off;

			r.AddPoint(px,py);
			r.AddPoint(px + g.width,py + g.height);

			x += g.advance;
		}
		else
		{
			x += mSpaceAdvance;
		}			
	}
	return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{
