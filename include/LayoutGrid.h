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

    static LayoutGrid* Create(uint32_t pColumns, uint32_t pRows);
    static LayoutGrid* Create(eui::Element* pParent,uint32_t pColumns, uint32_t pRows);
    virtual ~LayoutGrid();

    Element* GetCell(uint32_t pColumn, uint32_t pRow)const
    {
        return mCells[pColumn][pRow];
    }

protected:
    LayoutGrid(uint32_t pColumns, uint32_t pRows);

private:
    const uint32_t mColumns;
    const uint32_t mRows;

    // This is a convenience 2D array. The cells are added as a child to this element so all events work as normal.
    std::vector<std::vector<Element*>> mCells;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif//