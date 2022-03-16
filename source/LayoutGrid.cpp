
#include "LayoutGrid.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
LayoutGrid::LayoutGrid(GraphicsPtr pGraphics,uint32_t pColumns, uint32_t pRows):
    Element(pGraphics),
    mColumns(pColumns),
    mRows(pRows)
{
    SetArea(0,0,ELEMENT_SIZE_USE_PARENT,ELEMENT_SIZE_USE_PARENT);
    assert( pColumns > 0 );
    assert( pRows > 0 );

    int n = 0;
    for(int r = 0 ; r < mRows ; r++ )
    {
        std::vector<ElementPtr> row;
        for(int c = 0 ; c < mColumns ; c++ )
        {
            ElementPtr e = AddElement(0,0,0,0);
            if( (r+c)&1 == 1 )
                e->SetBackground(COLOUR_GREEN);
            else
                e->SetBackground(COLOUR_BLUE);

            e->SetLabel(std::to_string(n));n++;
            row.emplace_back(e);
        }
        mCells.emplace_back(row);
    }
}

LayoutGrid::~LayoutGrid()
{

}

void LayoutGrid::RecomputeRectangles(const Rectangle& pParentContectRect)
{
    Element::RecomputeRectangles(pParentContectRect);

    assert( mColumns > 0 );
    assert( mRows > 0 );

    const Rectangle total = GetContentRectangle();
    const float cellWidth = (float)total.width / mColumns;
    const float cellHeight = (float)total.height / mRows;

    float y = 0;

    for(int r = 0 ; r < mRows ; r++, y += cellHeight )
    {
        float x = 0;
        for(int c = 0 ; c < mColumns ; c++, x += cellWidth )
        {
            mCells[r][c]->SetArea(x,y,cellWidth,cellHeight);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

