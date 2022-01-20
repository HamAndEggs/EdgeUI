#ifndef Style_h__
#define Style_h__

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Style
{
    Style() = default;
    Style(Colour pColour,int pRadius){mBackground = pColour;mRadius = pRadius;}
    uint32_t mFont = 0;                     //!< The font used for rendering.
    Colour mForground = COLOUR_NONE;        //!< If alpha is set to zero then forground will not be rendered.
    Colour mBackground = COLOUR_NONE;       //!< If alpha is set to zero then background will not be rendered.
    Colour mBackgroundGradient = COLOUR_NONE; //!< If mBackground and mBackgroundGradient are different and neither are COLOUR_NONE then a gradient is used.
    Colour mBorder = COLOUR_NONE;           //!< If alpha is set to zero then background will not be rendered.
    uint32_t mRadius = 0;                   //!< If background is rendered will give the rectangle rounded edges.
    uint32_t mBorderSize = 0;               //!< if boarder is rendered, will be the width in pixels.
    float mGradientDirection = 0;           //!< The gradients direction in degrees, 0 is top to bottom, 90 is right to left, 180 is bottom to top. Goes clockwise.

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //Style_h__