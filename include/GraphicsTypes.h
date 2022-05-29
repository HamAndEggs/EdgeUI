#ifndef GRAPHICSTYPES_H__
#define GRAPHICSTYPES_H__

#include "ScratchBuffer.h"

#include <stdint.h>
#include <vector>
#include <assert.h>
#include <math.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    

typedef uint32_t Colour;    //!< 32bit ARGB value. 

constexpr Colour MakeColour(uint8_t pR,uint8_t pG,uint8_t pB,uint8_t pA = 255)
{
    return (pA<<24) | (pR<<16) | (pG<<8) | (pB<<0);
}

constexpr uint8_t GetAlpha(const Colour pColour)
{
    return (pColour&0xff000000)>>24;
}

constexpr uint8_t GetRed(const Colour pColour)
{
    return (pColour&0x00ff0000)>>16;
}

constexpr uint8_t GetGreen(const Colour pColour)
{
    return (pColour&0x0000ff00)>>8;
}

constexpr uint8_t GetBlue(const Colour pColour)
{
    return (pColour&0x000000ff)>>0;
}

constexpr Colour SetAlpha(Colour pColour,uint8_t pA = 255)
{
    return (pA<<24) | (pColour&0x00ffffff);
}

static const Colour COLOUR_NONE = 0;
static const Colour COLOUR_BLACK = MakeColour(0,0,0);
static const Colour COLOUR_WHITE = MakeColour(255,255,255);
static const Colour COLOUR_GREY = MakeColour(150,150,150);
static const Colour COLOUR_RED = MakeColour(255,0,0);
static const Colour COLOUR_GREEN = MakeColour(0,255,0);
static const Colour COLOUR_DARK_GREEN = MakeColour(0,127,0);
static const Colour COLOUR_BLUE = MakeColour(0,0,255);
static const Colour COLOUR_LIGHT_GREY = MakeColour(200,200,200);
static const Colour COLOUR_DARK_GREY = MakeColour(100,100,100);

constexpr float ColourToFloat(uint8_t pColour)
{
	return (float)pColour / 255.0f;
}

/**
 * @brief Allows draw functions to take a rectangle and draw content with a given alignment, such as text.
 * 
 */
enum AlignmentPrimitives
{
	ALIGN_MIN_EDGE		= (1<<0),
	ALIGN_CENTER		= (1<<1),
	ALIGN_MAX_EDGE		= (1<<2),
};

#define ALIGNMENT_MASK		0xffff
#define ALIGNMENT_X_SHIFT	16
#define ALIGNMENT_Y_SHIFT	0

#define SET_ALIGNMENT(ALIGNMENT_PRIM_X,ALIGNMENT_PRIM_Y)	((ALIGNMENT_PRIM_X << ALIGNMENT_X_SHIFT)|(ALIGNMENT_PRIM_Y << ALIGNMENT_Y_SHIFT))
#define GET_X_ALIGNMENT(ALIGNMENT_VALUE)					((ALIGNMENT_VALUE>>ALIGNMENT_X_SHIFT)&ALIGNMENT_MASK)
#define GET_Y_ALIGNMENT(ALIGNMENT_VALUE)					((ALIGNMENT_VALUE>>ALIGNMENT_Y_SHIFT)&ALIGNMENT_MASK)

typedef uint32_t Alignment;

static const Alignment ALIGN_LEFT_TOP			= SET_ALIGNMENT(ALIGN_MIN_EDGE,	ALIGN_MIN_EDGE);
static const Alignment ALIGN_CENTER_TOP			= SET_ALIGNMENT(ALIGN_CENTER,	ALIGN_MIN_EDGE);
static const Alignment ALIGN_RIGHT_TOP			= SET_ALIGNMENT(ALIGN_MAX_EDGE,	ALIGN_MIN_EDGE);

static const Alignment ALIGN_LEFT_CENTER		= SET_ALIGNMENT(ALIGN_MIN_EDGE,	ALIGN_CENTER);
static const Alignment ALIGN_CENTER_CENTER		= SET_ALIGNMENT(ALIGN_CENTER,	ALIGN_CENTER);
static const Alignment ALIGN_RIGHT_CENTER		= SET_ALIGNMENT(ALIGN_MAX_EDGE,	ALIGN_CENTER);

static const Alignment ALIGN_LEFT_BOTTOM		= SET_ALIGNMENT(ALIGN_MIN_EDGE,	ALIGN_MAX_EDGE);
static const Alignment ALIGN_CENTER_BOTTOM		= SET_ALIGNMENT(ALIGN_CENTER,	ALIGN_MAX_EDGE);
static const Alignment ALIGN_RIGHT_BOTTOM		= SET_ALIGNMENT(ALIGN_MAX_EDGE,	ALIGN_MAX_EDGE);

// Basic 3D vertex with x,y,z and 32bit colour value.
struct VertXYZC
{
	float x,y,z;
    Colour argb;
};
typedef std::vector<VertXYZC> VerticesXYZC;

// Basic 3D vertex with x,y,z and 16bit texture coords that are normalised. So 0x7fff is 1.0f. Used the texture coordinate scale option to get tiling. When I put that in...
struct VertXYZUV
{
	float x,y,z;
    float u,v;
};
typedef std::vector<VertXYZUV> VerticesXYZUV;

// List of points, expected to be in screen space, mainly used for drawing line lists. Compressed into 16 bits per element.
struct VertXY
{
	VertXY() = default;
	VertXY(float pX,float pY):x(pX),y(pY){};

	float x,y;

	typedef std::vector<VertXY> Vector;
//	typedef ScratchBuffer<VertXY,128,64,1024> Buffer;
	/**
	 * @brief Simple utility for building quads on the fly.
	 */
	struct Buffer : public ScratchBuffer<VertXY,256,64,1024>
	{
		/**
		 * @brief Writes six vertices to the buffer.
		 */
		inline void BuildQuad(float pX,float pY,float pWidth,float pHeight)
		{
			VertXY* verts = Next(6);
			verts[0].x = pX;			verts[0].y = pY;
			verts[1].x = pX + pWidth;	verts[1].y = pY;
			verts[2].x = pX + pWidth;	verts[2].y = pY + pHeight;

			verts[3].x = pX;			verts[3].y = pY;
			verts[4].x = pX + pWidth;	verts[4].y = pY + pHeight;
			verts[5].x = pX;			verts[5].y = pY + pHeight;
		}

		/**
		 * @brief Writes the UV's to six vertices in the correct order to match the quad built above.
		 */
		inline void AddUVRect(float U0,float V0,float U1,float V1)
		{
			VertXY* verts = Next(6);
			verts[0].x = U0;	verts[0].y = V0;
			verts[1].x = U1;	verts[1].y = V0;
			verts[2].x = U1;	verts[2].y = V1;

			verts[3].x = U0;	verts[3].y = V0;
			verts[4].x = U1;	verts[4].y = V1;
			verts[5].x = U0;	verts[5].y = V1;
		}

		/**
		 * @brief Adds a number of quads to the buffer, moving STEP for each one.
		 */
		inline void BuildQuads(float pX,float pY,float pWidth,float pHeight,int pCount,int pXStep,int pYStep)
		{
			for(int n = 0 ; n < pCount ; n++, pX += pXStep, pY += pYStep )
			{
				BuildQuad(pX,pY,pWidth,pHeight);
			}
		}

		const VertXY& operator[](size_t index)const
		{
			assert(index < Used());
			return Data()[index];
		}
	};
};

inline constexpr float GetPI()
{
	return std::acos(-1);
}

inline constexpr float GetRadian()
{
	return 2.0f * GetPI();
}

inline constexpr float DegreeToRadian(float pDegree)
{
	return pDegree * (GetPI()/180.0f);
}

inline float Lerp(float pFrom,float pTo,float pLerp)
{
	return (pFrom * (1.0f - pLerp)) + (pTo * pLerp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //GRAPHICSTYPES_H__