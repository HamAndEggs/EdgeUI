#ifndef Style_h__
#define Style_h__

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Style
{
    Style() = default;
    Style(Colour pColour,float pRadius){mBackground = pColour;mRadius = pRadius;}

    Colour mForground = COLOUR_WHITE;       //!< If alpha is set to zero then forground will not be rendered.
    Colour mBackground = COLOUR_NONE;       //!< If alpha is set to zero then background will not be rendered.
    Colour mBackgroundGradient = COLOUR_NONE; //!< If mBackground and mBackgroundGradient are different and neither are COLOUR_NONE then a gradient is used.
    Colour mBorder = COLOUR_NONE;           //!< If alpha is set to zero then background will not be rendered.
    float mRadius = 0;                      //!< If background is rendered will give the rectangle rounded edges.
    float mBorderSize = 0;                  //!< if boarder is rendered, will be the width in pixels.
    float mGradientDirection = 0;           //!< The gradients direction in degrees, 0 is top to bottom, 90 is right to left, 180 is bottom to top. Goes clockwise.
    Alignment mAlignment = ALIGN_LEFT_TOP;  //!< How to position content in the content rect. For content scaled to the size of the content this will not have an effect.
    uint32_t mTexture = 0;                  //!< If set, will be scaled to fit content rect. If mBackground colour is set, will be modulated against that.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif //Style_h__