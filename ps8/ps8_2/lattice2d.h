#ifndef LATTICE_IS_INCLUDED
#define LATTICE_IS_INCLUDED
/* { */

#include <vector>
#include "ysclass.h"

template <class T>
class Lattice2d
{
protected:
    int nx,ny;
    std::vector <T> elem;
    YsVec2 min,max;
public:
    /*! Create a lattice with nx*ny*nz blocks.  To store nodal information, the length of the
        storage actually allocated will be (nx+1)*(ny+1).  */
    void Create(int nx,int ny,const YsVec2 &min,const YsVec2 &max)
    {
		this->nx=nx;
		this->ny=ny;
		elem.resize((nx+1)*(ny+1));
		this->min=min;
		this->max=max;
	}
    /*! Returns number of blocks in X,Y, and Z direction. */
    YSSIZE_T Nx(void) const
    {
		return nx;
	}
    YSSIZE_T Ny(void) const
    {
		return ny;
	}
    /*! Returns the dimension of one block. */
    YsVec2 GetBlockDimension(void) const
    {
		if(0<nx && 0<ny)
		{
			YsVec2 diagonal=max-min;
			diagonal.DivX((double)nx);
			diagonal.DivY((double)ny);
			return diagonal;
		}
		else
		{
			return YsVec2::Origin();
		}
	}
    /*! Returns the minimum (x,y) of the block at (bx,by). */
    YsVec2 GetBlockPosition(int ix,int iy) const
    {
		auto blockDim=GetBlockDimension();
		blockDim.MulX((double)ix);
		blockDim.MulY((double)iy);
		return min+blockDim;
	}
    /*! Returns the index of the block that encloses the given position. */
    YsVec2i GetBlockIndex(const YsVec2 &pos) const
    {
		auto blockDim=GetBlockDimension();
		auto rel=pos-min;

		YsVec2i idx;
		idx.SetX((int)(rel.x()/blockDim.x()));
		idx.SetY((int)(rel.y()/blockDim.y()));

		return idx;
	}
    /*! Returns if the block index is within the valid range.
        The lattice elements can be nodal value or cell value.  To support the nodal values,
        this class allocates (nx+1)*(ny+1)*(nz+1) elems.  Therefore, the index is valid
        and this function returns true, if:
           0<=idx.x() && idx.x()<=nx && 0<=idx.y() && idx.y()<=ny && 0<=idx.z() && idx.z()<=nz. */
    bool IsInRange(const YsVec2i idx) const
    {
		return (0<=idx.x() && idx.x()<=nx && 0<=idx.y() && idx.y()<=ny);
	}
    /*! Returns a reference to the lattice element at the index. */
    T &operator[](const YsVec2i idx)
    {
		return elem[idx.y()*(nx+1)+idx.x()];
	}
    const T &operator[](const YsVec2i idx) const
    {
		return elem[idx.y()*(nx+1)+idx.x()];
	}
};



/* } */
#endif
