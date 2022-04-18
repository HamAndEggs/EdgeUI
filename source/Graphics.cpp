#include "Graphics.h"
#include "Rectangle.h"

#include <math.h>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
Graphics::Graphics()
{

}

Graphics::~Graphics()
{

}

void Graphics::SetExitRequest()
{
    mExitRequest = true;
}

void Graphics::GetRoundedRectanglePoints(const RectangleF& pRect,VertFloatXY::Buffer& rBuffer,float pRadius)
{
	const int radius = (int)(std::min(std::min(pRect.height/2,pRect.width/2),pRadius));
    // This uses the Midpoint circle algorithm.
    // Did try with basic float trig and sin / cos but it caused issues with the curces at each corner not being the same.
    int x = radius;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius / 2.0f );

    // Little buffer for the segment that we calculate, it's 1/8th of the circle.
    int vx[radius];
    int vy[radius];

    int vNum = 0;
    while (x >= y)
    {
        vx[vNum] = x;
        vy[vNum] = y;
        vNum++;

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }

    int top = pRect.x + radius;
    int left = pRect.y + radius;
    int right = pRect.x + pRect.width - radius;
    int bottom = pRect.y + pRect.height - radius;

    // Now build circle.
    VertFloatXY* verts = rBuffer.Restart(vNum*8);
    for( int n = 0 ; n < vNum ; n++ )
    {
        assert( n + (vNum * 7) < (vNum*8) );
        verts[(vNum * 0) + n].x = right + vx[n];
        verts[(vNum * 0) + n].y = bottom + vy[n];

        verts[(vNum * 2) - n - 1].x = right + vy[n];
        verts[(vNum * 2) - n - 1].y = bottom + vx[n];

        verts[(vNum * 2) + n].x = left - vy[n];
        verts[(vNum * 2) + n].y = bottom + vx[n];

        verts[(vNum * 4) - n - 1].x = left - vx[n];
        verts[(vNum * 4) - n - 1].y = bottom + vy[n];

        verts[(vNum * 4) + n].x = left - vx[n];
        verts[(vNum * 4) + n].y = top - vy[n];

        verts[(vNum * 6) - n - 1].x = left - vy[n];
        verts[(vNum * 6) - n - 1].y = top - vx[n];

        verts[(vNum * 6) + n].x = right + vy[n];
        verts[(vNum * 6) + n].y = top - vx[n];

        verts[(vNum * 8) - n - 1].x = right + vx[n];
        verts[(vNum * 8) - n - 1].y = top - vy[n];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{
