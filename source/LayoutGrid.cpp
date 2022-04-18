
#include "LayoutGrid.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
LayoutGrid* LayoutGrid::Create(uint32_t pColumns, uint32_t pRows)
{
    LayoutGrid* layout = new LayoutGrid(pColumns,pRows);
    layout->SetPos(0,0);
    layout->SetForground(COLOUR_WHITE);
    return layout;
}

LayoutGrid* LayoutGrid::Create(eui::Element* pParent,uint32_t pColumns, uint32_t pRows)
{
    LayoutGrid* layout = Create(pColumns,pRows);
    pParent->Attach(layout);

    return layout;
}

LayoutGrid::LayoutGrid(uint32_t pColumns, uint32_t pRows):
    Element(),
    mColumns(pColumns),
    mRows(pRows)
{
    assert( pColumns > 0 );
    assert( pRows > 0 );

    SET_DEFAULT_ID();
    const PointF& size = GetSize();
    const float cellWidth = 1.0f / mColumns;
    const float cellHeight = 1.0f / mRows;

    int n = 0;
    for(int r = 0 ; r < mRows ; r++ )
    {
        std::vector<Element*> row;
        for(int c = 0 ; c < mColumns ; c++ )
        {
            Element* e = Element::Create();
            e->SetPos(cellWidth * r,cellHeight * c);
            e->SetSize(cellWidth,cellHeight);
            Attach(e);
            if( (r+c)&1 == 1 )
                e->SetBackground(COLOUR_GREEN);
            else
                e->SetBackground(COLOUR_BLUE);

            e->SetID(std::to_string(n) );n++;
            row.emplace_back(e);
        }
        mCells.emplace_back(row);
    }
}

LayoutGrid::~LayoutGrid()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

