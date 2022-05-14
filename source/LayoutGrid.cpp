
#include "LayoutGrid.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
LayoutGrid* LayoutGrid::Create(uint32_t pWidth, uint32_t pHeight)
{
    LayoutGrid* layout = new LayoutGrid(pWidth,pHeight);
    return layout;
}

LayoutGrid* LayoutGrid::Create(eui::Element* pParent,uint32_t pWidth, uint32_t pHeight)
{
    LayoutGrid* layout = Create(pWidth,pHeight);
    pParent->Attach(layout);

    return layout;
}

LayoutGrid::LayoutGrid(uint32_t pWidth, uint32_t pHeight):
    Element()
{
    assert( pWidth > 0 );
    assert( pHeight > 0 );

    // Pre allocate the 2D array
    mCells.resize(pWidth);
    for( auto& c : mCells )
    {
        c.resize(pHeight);
    }

    SET_DEFAULT_ID();
}

LayoutGrid::~LayoutGrid()
{

}

void LayoutGrid::Attach(Element* pNewCell,uint32_t pX, uint32_t pY,uint32_t pSpanX,uint32_t pSpanY)
{
    if( mCells[pX][pY] )
    {
        mCells[pX][pY]->Attach(pNewCell);
        return;
    }
    Element::Attach(pNewCell);

    pNewCell->SetLeftTop(GetCellFractionalWidth() * pX,GetCellFractionalHeight() * pY);
    pNewCell->SetRightBottom(GetCellFractionalWidth() * (pX+pSpanX),GetCellFractionalHeight() * (pY+pSpanY));
    mCells[pX][pY] = pNewCell;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

