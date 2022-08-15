#ifndef Style_h__
#define Style_h__

#include "GraphicsTypes.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Style
{
    Style() = default;
    Style(Colour pColour,float pRadius){mBackground = pColour;mRadius = pRadius;}

    enum BoarderStyle
    {
        BS_SOLID,
        BS_RAISED,
        BS_DEPRESSED
    };

    Colour mForeground = COLOUR_WHITE;          //!< If alpha is set to zero then foreground will not be rendered.
    Colour mBackground = COLOUR_NONE;           //!< If alpha is set to zero then background will not be rendered.
    Colour mBorder = COLOUR_NONE;               //!< If alpha is set to zero then background will not be rendered.
    float mRadius = 0;                          //!< If background is rendered will give the rectangle rounded edges.
    float mThickness = 0;                      //!< if boarder is rendered, will be the width in pixels. For lines, drawn with this mThickness.
    BoarderStyle mBoarderStyle = BS_SOLID;      //!< Used for buttons and content groups, normally you'll use BS_SOLID.
    Alignment mAlignment = ALIGN_CENTER_CENTER; //!< How to position content in the content rect. For content scaled to the size of the content this will not have an effect.
    uint32_t mTexture = 0;                      //!< If set, will be scaled to fit content rect. If mBackground colour is set, will be modulated against that.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace eui{

#endif //Style_h__