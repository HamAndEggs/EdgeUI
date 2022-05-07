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

    static LayoutGrid* Create(uint32_t pWidth, uint32_t pHeight);
    static LayoutGrid* Create(eui::Element* pParent,uint32_t pWidth, uint32_t pHeight);
    virtual ~LayoutGrid();

    Element* GetCell(uint32_t pX, uint32_t pY)const
    {
        return mCells[pX][pY];
    }

    size_t GetWidth()const{return mCells.size();}
    size_t GetHeight()const{return mCells[0].size();}

    float GetCellFractionalWidth()const{return 1.0f / GetWidth();}
    float GetCellFractionalHeight()const{return 1.0f / GetHeight();}

    /**
     * @brief If the cell has an element, will attach as child, if not set then element will become the root.
     */
    void Attach(uint32_t pX, uint32_t pY, Element* pNewCell);

protected:
    LayoutGrid(uint32_t pWidth, uint32_t pHeight);

private:

    // This is a convenience 2D array. The cells are added as a child to this element so all events work as normal.
    std::vector<std::vector<Element*>> mCells;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
};//namespace eui{

#endif//
