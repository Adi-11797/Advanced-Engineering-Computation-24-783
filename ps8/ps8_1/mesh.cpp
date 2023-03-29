#include <vector>
#include <math.h>
#include <stdio.h>
#include "mesh.h"
#include "lattice.h"


static void ExtractVector(float vec[3],const unsigned char dat[])
{
	vec[0]=*((const float *) dat   );
	vec[1]=*((const float *)(dat+4));  // &dat[4] <=>  dat+4
	vec[2]=*((const float *)(dat+8));
}

bool PolygonalMesh::LoadSTL(const char fName[])
{
	FILE *fp=fopen(fName,"rb");
	if(nullptr!=fp)
	{
		unsigned char buf[80];
		fread(buf,1,80,fp);

		fread(buf,1,4,fp);
		int nTri=*((int *)buf);

		printf("%d triangles.\n",nTri);

		for(int i=0; i<nTri; ++i)
		{
			auto bytesRead=fread(buf,1,50,fp);
			if(bytesRead<50)
			{
				break;
			}

			float n[3],v[3][3];
			ExtractVector(n,buf);
			ExtractVector(v[0],buf+12);
			ExtractVector(v[1],buf+24);
			ExtractVector(v[2],buf+36);

			VertexHandle triVtHd[3]={NullVertex(),NullVertex(),NullVertex()};
			for(int i=0; i<3; ++i)
			{
				auto pos=YsVec3(v[i][0],v[i][1],v[i][2]);
				triVtHd[i]=AddVertex(pos);
			}
			auto plHd=AddPolygon(3,triVtHd);
			SetPolygonNormal(plHd,YsVec3(n[0],n[1],n[2]));

			YsColor col;
			col.SetFloatRGBA(0,1,0,1);
			SetPolygonColor(plHd,col);
		}

		fclose(fp);
		return true;
	}
	return false;
}
void PolygonalMesh::GetBoundingBox(YsVec3 minmax[2]) const
{
	if(0<GetNumVertex())
	{
		YsVec3 pos=GetVertexPosition(FindFirstVertex());
		minmax[0]=pos;
		minmax[1]=pos;
		for(auto vtHd=FindFirstVertex(); NullVertex()!=vtHd; vtHd=FindNextVertex(vtHd))
		{
			YsVec3 pos=GetVertexPosition(vtHd);
			minmax[0][0]=std::min<double>(minmax[0][0],pos[0]);
			minmax[0][1]=std::min<double>(minmax[0][1],pos[1]);
			minmax[0][2]=std::min<double>(minmax[0][2],pos[2]);
			minmax[1][0]=std::max<double>(minmax[1][0],pos[0]);
			minmax[1][1]=std::max<double>(minmax[1][1],pos[1]);
			minmax[1][2]=std::max<double>(minmax[1][2],pos[2]);
		}
	}
	else
	{
		minmax[0][0]=0;
		minmax[0][1]=0;
		minmax[0][2]=0;
		minmax[1][0]=0;
		minmax[1][1]=0;
		minmax[1][2]=0;
	}
}

void PolygonalMesh::Stitch(double tolerance)
{
	if(0==GetNumVertex())
	{
		return;
	}

	Lattice3d <std::vector <VertexHandle> > ltc;
	YsVec3 minmax[2];
	GetBoundingBox(minmax);

	double D=(minmax[1]-minmax[0]).GetLength();
	minmax[0].SubX(D*0.01);
	minmax[0].SubY(D*0.01);
	minmax[0].SubZ(D*0.01);
	minmax[1].AddX(D*0.01);
	minmax[1].AddY(D*0.01);
	minmax[1].AddZ(D*0.01);

	YsVec3 diagon=minmax[1]-minmax[0];

	double vol=diagon.x()*diagon.y()*diagon.z();
	vol/=(double)GetNumVertex();

	double e=pow(vol,1.0/3.0);

	int nx=1+(int)(diagon.x()/e);
	int ny=1+(int)(diagon.y()/e);
	int nz=1+(int)(diagon.z()/e);

	ltc.Create(nx,ny,nz,minmax[0],minmax[1]);
	for(auto vtHd=FindFirstVertex(); NullVertex()!=vtHd; vtHd=FindNextVertex(vtHd))
	{
		auto idx=ltc.GetBlockIndex(GetVertexPosition(vtHd));
		ltc[idx].push_back(vtHd);
	}

	for(auto plHd=FindFirstPolygon(); NullPolygon()!=plHd; plHd=FindNextPolygon(plHd))
	{
		auto plVtHd=GetPolygonVertex(plHd);
		bool mod=false;

		for(auto &vtHd : plVtHd)
		{
			YsVec3 pos0=GetVertexPosition(vtHd);
			YsVec3 pos[2]={pos0,pos0};
			pos[0].SubX(tolerance);
			pos[0].SubY(tolerance);
			pos[0].SubZ(tolerance);
			pos[1].AddX(tolerance);
			pos[1].AddY(tolerance);
			pos[1].AddZ(tolerance);

			VertexHandle repl=vtHd;
			auto idx0=ltc.GetBlockIndex(pos[0]);
			auto idx1=ltc.GetBlockIndex(pos[1]);
			for(int x=idx0.x(); x<=idx1.x(); ++x)
			{
				for(int y=idx0.y(); y<=idx1.y(); ++y)
				{
					for(int z=idx0.z(); z<=idx1.z(); ++z)
					{
						YsVec3i idx(x,y,z);
						if(true==ltc.IsInRange(idx))
						{
							for(auto tstVtHd : ltc[idx])
							{
								double dist=(GetVertexPosition(tstVtHd)-pos0).GetLength();
								if(dist<tolerance && GetSearchKey(tstVtHd)<GetSearchKey(repl))
								{
									repl=tstVtHd;
								}
							}
						}
					}
				}
			}

			if(repl!=vtHd)
			{
				vtHd=repl;
				mod=true;
			}
		}

		if(true==mod)
		{
			SetPolygonVertex(plHd,plVtHd.size(),plVtHd.data());
		}
	}
}

PolygonalMesh::PolygonHandle PolygonalMesh::GetNeighborPolygon(PolygonHandle plHd,int i) const
{
	auto plVtHd=GetPolygonVertex(plHd);
	if(3<=plVtHd.size())
	{
		auto edPlHd=FindPolygonFromEdge(plVtHd[i],plVtHd[(i+1)%plVtHd.size()]);
		if(edPlHd.size()==2)
		{
			if(edPlHd[0]!=plHd)
			{
				return edPlHd[0];
			}
			else if(edPlHd[1]!=plHd)
			{
				return edPlHd[1];
			}
		}
	}
	return NullPolygon();
}
