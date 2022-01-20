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
static const Colour COLOUR_LIGHT_GREY = MakeColour(200,200,200);
static const Colour COLOUR_DARK_GREY = MakeColour(100,100,100);

constexpr float ColourToFloat(uint8_t pColour)
{
	return (float)pColour / 255.0f;
}

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
    int16_t u,v;

	void SetUV(float pU,float pV)
	{
		const float SCALE = (float)0x7fff;

		assert( pU >= -1.0f && pU <= 1.0f );
		assert( pV >= -1.0f && pV <= 1.0f );

		u = (int16_t)(pU * SCALE);
		v = (int16_t)(pV * SCALE);
	}

};
typedef std::vector<VertXYZUV> VerticesXYZUV;

// List of points, expected to be in screen space, mainly used for drawing line lists. Compressed into 16 bits per element.
template <typename ELEMENT_TYPE> struct VertexXY
{
	VertexXY() = default;
	VertexXY(ELEMENT_TYPE pX,ELEMENT_TYPE pY):x(pX),y(pY){};

	ELEMENT_TYPE x,y;

	typedef std::vector<VertexXY<ELEMENT_TYPE>> Vector;
	typedef ScratchBuffer<VertexXY<ELEMENT_TYPE>,128,64,1024> Buffer;	
};

typedef VertexXY<int8_t> VertInt8XY;
typedef VertexXY<int16_t> VertInt16XY;
typedef VertexXY<int32_t> VertInt32XY;
typedef VertexXY<float> VertFloatXY;

struct Quad2D
{
	VertInt16XY v[4];

	const int16_t* data()const{return &v[0].x;}
};

struct Quad2Df
{
	VertFloatXY v[4];

	const float* data()const{return &v[0].x;}
};


/**
 * @brief Simple utility for building quads on the fly.
 */
struct Vert2DShortScratchBuffer : public ScratchBuffer<VertInt16XY,256,64,1024>
{
	/**
	 * @brief Writes six vertices to the buffer.
	 */
	inline void BuildQuad(int pX,int pY,int pWidth,int pHeight)
	{
		VertInt16XY* verts = Next(6);
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
	inline void AddUVRect(int U0,int V0,int U1,int V1)
	{
		VertInt16XY* verts = Next(6);
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
	inline void BuildQuads(int pX,int pY,int pWidth,int pHeight,int pCount,int pXStep,int pYStep)
	{
		for(int n = 0 ; n < pCount ; n++, pX += pXStep, pY += pYStep )
		{
			BuildQuad(pX,pY,pWidth,pHeight);
		}
	}
};


constexpr float GetPI()
{
	return std::acos(-1);
}

constexpr float GetRadian()
{
	return 2.0f * GetPI();
}

constexpr float GetRadianToSignedShort()
{
	return 32767.0f / GetRadian();
}

constexpr float DegreeToRadian(float pDegree)
{
	return pDegree * (GetPI()/180.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //GRAPHICSTYPES_H__