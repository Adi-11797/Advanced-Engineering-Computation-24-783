#ifndef POLYGONALMESH_IS_INCLUDED
#define POLYGONALMESH_IS_INCLUDED

#include <list>
#include <unordered_map>
#include <ysclass.h>


class EdgeKey
{
public:
    unsigned int edVtKey[2];

    bool operator==(const EdgeKey &rhs) const
    {
        return (edVtKey[0]==rhs.edVtKey[0] && edVtKey[1]==rhs.edVtKey[1]) ||
               (edVtKey[0]==rhs.edVtKey[1] && edVtKey[1]==rhs.edVtKey[0]);
    }
    bool operator!=(const EdgeKey &rhs) const
    {
        return (edVtKey[0]!=rhs.edVtKey[0] || edVtKey[1]!=rhs.edVtKey[1]) &&
               (edVtKey[0]!=rhs.edVtKey[1] || edVtKey[1]!=rhs.edVtKey[0]);
    }
};

template <>
struct std::hash <EdgeKey>
{
    std::size_t operator()(const EdgeKey &s) const noexcept
    {
        auto larger=std::max(s.edVtKey[0], s.edVtKey[1]);
        auto smaller=std::max(s.edVtKey[0], s.edVtKey[1]); 
        return larger*11+smaller*7;
    };
};


class PolygonalMesh
{
protected:
	class HasSearchKey
	{
	public:
		unsigned int searchKey;
	};
    class Vertex : public HasSearchKey
    {
    public:
        YsVec3 pos;
    };
public:
    class VertexHandle
    {
    friend class PolygonalMesh;
    private:
        std::list <Vertex>::iterator vtxPtr;
    public:
        VertexHandle(){};  // C++11 VertexHandle()=default;
        bool operator==(const VertexHandle &vtHd) const
        {
			return this->vtxPtr==vtHd.vtxPtr;
		}
        bool operator!=(const VertexHandle &vtHd) const
        {
			return this->vtxPtr!=vtHd.vtxPtr;
		}
    };
private:
    class Polygon : public HasSearchKey
    {
    public:
        std::vector <VertexHandle> vtHd;
        YsVec3 nom;
        YsColor col;
    };
public:
    class PolygonHandle
    {
    friend class PolygonalMesh;
    private:
        std::list <Polygon>::iterator plgPtr;
    public:
        PolygonHandle(){};  // C++11 PolygonHandle()=default;
        inline bool operator==(const PolygonHandle &plHd) const
        {
			return plgPtr==plHd.plgPtr;
		}
        inline bool operator!=(const PolygonHandle &plHd) const
        {
			return plgPtr!=plHd.plgPtr;
		}
    };
private:
	const unsigned int nullSearchKey=~0;
	unsigned int searchKeySeed=1;
    mutable std::list <Vertex> vtxList;
    mutable std::list <Polygon> plgList;
	std::unordered_map <unsigned int,std::vector <PolygonHandle> > vtxToPlg;
	std::unordered_map <EdgeKey,std::vector <PolygonHandle> > edgToPlg;

public:
    VertexHandle AddVertex(const YsVec3 &pos)
    {
		Vertex newVtx;
		newVtx.pos=pos;
		newVtx.searchKey=searchKeySeed++;
		vtxList.push_back(newVtx);

		VertexHandle vtHd;
		vtHd.vtxPtr=vtxList.end();
		--vtHd.vtxPtr;
		return vtHd;
	}
    VertexHandle NullVertex(void) const
    {
		VertexHandle vtHd;
		vtHd.vtxPtr=vtxList.end();
		return vtHd;
	}
	/* Calling function must make sure vtHd!=NullVertex().
	*/
    YsVec3 GetVertexPosition(VertexHandle vtHd) const
    {
		return vtHd.vtxPtr->pos;
	}

	void SetVertexPosition(VertexHandle vtHd,const YsVec3 &pos)
	{
		vtHd.vtxPtr->pos=pos;
	}

	VertexHandle FindFirstVertex(void) const
	{
		VertexHandle vtHd;
		vtHd.vtxPtr=vtxList.begin();
		return vtHd;
	}
	/* Calling function must make sure vtHd!=NullVertex().
	*/
	VertexHandle FindNextVertex(VertexHandle vtHd) const
	{
		++vtHd.vtxPtr;
		return vtHd;
	}



public:
    PolygonHandle AddPolygon(int nPlVt,const VertexHandle plVtHd[])
    {
		Polygon plg;
		plg.vtHd.resize(nPlVt);
		for(int i=0; i<nPlVt; ++i)
		{
			plg.vtHd[i]=plVtHd[i];
		}
		plg.searchKey=searchKeySeed++;
		plgList.push_back(plg);

		PolygonHandle plHd;
		plHd.plgPtr=plgList.end();
		--plHd.plgPtr;

		RegisterPolygon(plHd,nPlVt,plVtHd);

		return plHd;
	}

	void SetPolygonVertex(PolygonHandle plHd,int nPlVt,const VertexHandle plVtHd[])
	{
		UnregisterPolygon(plHd);
		plHd.plgPtr->vtHd.resize(nPlVt);
		for(int i=0; i<nPlVt; ++i)
		{
			plHd.plgPtr->vtHd[i]=plVtHd[i];
		}
		RegisterPolygon(plHd,nPlVt,plVtHd);
	}

private:
	void RegisterPolygon(PolygonHandle plHd,int nPlVt,const VertexHandle plVtHd[])
	{
		for(int i=0; i<nPlVt; ++i)
		{
			{
				auto found=vtxToPlg.find(GetSearchKey(plVtHd[i]));
				if(vtxToPlg.end()!=found)
				{
					found->second.push_back(plHd);
				}
				else
				{
					std::vector <PolygonHandle> usingPlHd;
					usingPlHd.push_back(plHd);
					vtxToPlg[GetSearchKey(plVtHd[i])]=usingPlHd;
				}
			}
			{
				EdgeKey edge;
				edge.edVtKey[0]=GetSearchKey(plVtHd[i]);
				edge.edVtKey[1]=GetSearchKey(plVtHd[(i+1)%nPlVt]);
				auto found=edgToPlg.find(edge);
				if(edgToPlg.end()!=found)
				{
					found->second.push_back(plHd);
				}
				else
				{
					std::vector <PolygonHandle> usingPlHd;
					usingPlHd.push_back(plHd);
					edgToPlg[edge]=usingPlHd;
				}
			}
		}
	}
	void UnregisterPolygon(PolygonHandle plHd)
	{
		for(int i=0; i<plHd.plgPtr->vtHd.size(); ++i)
		{
			auto vtHd=plHd.plgPtr->vtHd[i];
			{
				auto found=vtxToPlg.find(GetSearchKey(vtHd));
				if(vtxToPlg.end()!=found)
				{
					for(int i=found->second.size()-1; 0<=i; --i)
					{
						if(found->second[i]==plHd)
						{
							found->second[i]=found->second.back();
							found->second.pop_back();
						}
					}
				}
			}
			{
				EdgeKey edge;
				edge.edVtKey[0]=GetSearchKey(vtHd);
				edge.edVtKey[1]=GetSearchKey(plHd.plgPtr->vtHd[(i+1)%plHd.plgPtr->vtHd.size()]);
				auto found=edgToPlg.find(edge);
				if(edgToPlg.end()!=found)
				{
					for(int i=found->second.size()-1; 0<=i; --i)
					{
						if(found->second[i]==plHd)
						{
							found->second[i]=found->second.back();
							found->second.pop_back();
						}
					}
				}
			}
		}
	}

public:
    PolygonHandle AddPolygon(const std::vector <VertexHandle> &plVtHd)
    {
		return AddPolygon((int)plVtHd.size(),plVtHd.data());
	}
    inline PolygonHandle NullPolygon(void) const
    {
		PolygonHandle plHd;
		plHd.plgPtr=plgList.end();
		return plHd;
	}
    const std::vector <VertexHandle> &GetPolygonVertex(PolygonHandle plHd) const
    {
		return plHd.plgPtr->vtHd;
	}

	PolygonHandle FindFirstPolygon(void) const
	{
		PolygonHandle plHd;
		plHd.plgPtr=plgList.begin();
		return plHd;
	}
	/* Calling function must make sure plHd!=NullPolygon().
	*/
	PolygonHandle FindNextPolygon(PolygonHandle plHd) const
	{
		++plHd.plgPtr;
		return plHd;
	}

	long long int GetNumVertex(void) const
	{
		return vtxList.size();
	}
    bool MoveToNextVertex(VertexHandle &vtHd) const
    {
		if(vtxList.end()==vtHd.vtxPtr)
		{
			vtHd.vtxPtr=vtxList.begin();
			return true;
		}
		++vtHd.vtxPtr;
		return vtxList.end()!=vtHd.vtxPtr;
	}
    long long int GetNumPolygon(void) const
    {
		return plgList.size();
	}
    bool MoveToNextPolygon(PolygonHandle &plHd) const
    {
		if(plgList.end()==plHd.plgPtr)
		{
			plHd.plgPtr=plgList.begin();
			return true;
		}
		++plHd.plgPtr;
		return plgList.end()!=plHd.plgPtr;
	}

	std::vector <PolygonHandle> FindPolygonFromVertex(VertexHandle vtHd) const
	{
		auto found=vtxToPlg.find(GetSearchKey(vtHd));
		if(vtxToPlg.end()!=found)
		{
			return found->second;
		}
		std::vector <PolygonHandle> empty;
		return empty;
	}

	std::vector <PolygonHandle> FindPolygonFromEdge(VertexHandle vtHd0,VertexHandle vtHd1) const
	{
		EdgeKey edge;
		edge.edVtKey[0]=GetSearchKey(vtHd0);
		edge.edVtKey[1]=GetSearchKey(vtHd1);
		auto found=edgToPlg.find(edge);
		if(edgToPlg.end()!=found)
		{
			return found->second;
		}
		std::vector <PolygonHandle> empty;
		return empty;
	}

	unsigned int GetSearchKey(VertexHandle vtHd) const
	{
		if(NullVertex()!=vtHd)
		{
			return vtHd.vtxPtr->searchKey;
		}
		return nullSearchKey;
	}
	unsigned int GetSearchKey(PolygonHandle plHd) const
	{
		if(NullPolygon()!=plHd)
		{
			return plHd.plgPtr->searchKey;
		}
		return nullSearchKey;
	}

    bool SetPolygonColor(PolygonHandle plHd,YsColor col)
    {
		plHd.plgPtr->col=col;
		return true;
	}
    YsColor GetColor(PolygonHandle plHd) const
    {
		return plHd.plgPtr->col;
	}
    bool SetPolygonNormal(PolygonHandle plHd,const YsVec3 &nom)
    {
		plHd.plgPtr->nom=nom;
		return true;
	}
    YsVec3 GetNormal(PolygonHandle plHd) const
    {
		return plHd.plgPtr->nom;
	}

	bool LoadSTL(const char fName[]);
	void GetBoundingBox(YsVec3 minmax[2]) const;
	void Stitch(double tolerance);

	/*! Returns neighboring polygon of plHd sharing edge i.  If the polygon consists of plVtHd[0] to
	    plVtHd[n-1], edge i connects plVtHd[i] and plVtHd[(i+1)%n].

	    If no polygon is connected to the polygon on edge i, or
	    more than two polygons are using edge i, this function returns NullPolygon().
	*/
	PolygonHandle GetNeighborPolygon(PolygonHandle plHd,int i) const;
};

#endif

