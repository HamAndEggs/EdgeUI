#ifndef GRIDLAYOUT_H__
#define GRIDLAYOUT_H__

#include "Element.h"

#include <vector>

namespace eui{
///////////////////////////////////////////////////////////////////////////////////////////////////////////    


/**
 * @brief
 */
class LayoutGrid : public Element
{
public:
    LayoutGrid(GraphicsPtr pGraphics,uint32_t pColumns, uint32_t pRows);
    virtual ~LayoutGrid();

    ElementPtr GetCell(uint32_t pColumn, uint32_t pRow)const
    {
        return mCells[pColumn][pRow];
    }

protected:
    virtual void RecomputeRectangles();

private:
    const uint32_t mColumns;
    const uint32_t mRows;

    // This is a convenience 2D array. The cells are added as a child to this element so all events work as normal.
    std::vector<std::vector<ElementPtr>> mCells;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif//