
#include "LayoutGrid.h"

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    
LayoutGrid* LayoutGrid::Create(uint32_t pColumns, uint32_t pRows)
{
    LayoutGrid* layout = new LayoutGrid(pColumns,pRows);
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
    const float cellWidth = 1.0f / mColumns;
    const float cellHeight = 1.0f / mRows;

    int n = 0;
    for(int c = 0 ; c < mColumns ; c++ )
    {
        std::vector<Element*> column;
        for(int r = 0 ; r < mRows ; r++ )
        {
            Element* e = Element::Create();
            e->SetLeftTop(cellWidth * c,cellHeight * r);
            e->SetRightBottom(cellWidth * (c+1),cellHeight * (r+1));
            Attach(e);
/*            if( (r+c)&1 == 1 )
            {
                e->SetBackground(COLOUR_GREEN);
            }
            else
            {
                e->SetBackground(COLOUR_BLUE);
            }*/
            e->SetID(std::to_string(n) );n++;
            column.emplace_back(e);
        }
        mCells.emplace_back(column);
    }
}

LayoutGrid::~LayoutGrid()
{

}

void LayoutGrid::ReplaceCell(uint32_t pColumn, uint32_t pRow, Element* pNewCell)
{
    if( mCells[pColumn][pRow] )
    {
        Remove(mCells[pColumn][pRow]);
        delete mCells[pColumn][pRow];
    }
    Attach(pNewCell);

    const float cellWidth = 1.0f / mColumns;
    const float cellHeight = 1.0f / mRows;

    pNewCell->SetLeftTop(cellWidth * pRow,cellHeight * pColumn);
    pNewCell->SetRightBottom(cellWidth * (pRow+1),cellHeight * (pColumn+1));
    mCells[pColumn][pRow] = pNewCell;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

